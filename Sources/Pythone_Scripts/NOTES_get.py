import json
import serial
import pandas as pd
from datetime import datetime
import time
import re


class ESP32ToExcel:
    def __init__(self, port='COM7', baudrate=9600):
        self.port = port
        self.baudrate = baudrate
        self.ser = None

    def connect(self):
        """Подключение к ESP32 с полным контролем DTR"""
        try:
            # Явное управление DTR перед открытием порта
            self.ser = serial.Serial()
            self.ser.port = self.port
            self.ser.baudrate = self.baudrate
            self.ser.bytesize = serial.EIGHTBITS
            self.ser.parity = serial.PARITY_NONE
            self.ser.stopbits = serial.STOPBITS_ONE
            self.ser.timeout = 2
            self.ser.dsrdtr = False
            self.ser.rtscts = False
            self.ser.xonxoff = False

            # Принудительно устанавливаем DTR в False перед открытием
            self.ser.setDTR(False)
            self.ser.setRTS(False)

            self.ser.open()

            # Даем время на стабилизацию
            time.sleep(2)

            # Снова сбрасываем DTR после открытия
            self.ser.setDTR(False)
            self.ser.setRTS(False)

            print(f"Подключено к {self.port}")

            # Очищаем буферы
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()

            # Ждем стабилизации ESP32
            print("Ждем стабилизации ESP32...")
            time.sleep(3)

            # Проверяем связь
            self.ser.write(b'\n')
            time.sleep(1)

            # Читаем приветственное сообщение
            print("Проверка связи с ESP32...")
            time.sleep(2)

            initial_messages = []
            start_time = time.time()
            while time.time() - start_time < 5:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"ESP32: {line}")
                        initial_messages.append(line)

            # Проверяем наличие признаков перезагрузки
            reboot_indicators = ['ets Jun', 'rst', 'boot', 'load', 'entry']
            has_reboot = any(any(indicator in line for indicator in reboot_indicators)
                             for line in initial_messages)

            if has_reboot:
                print("ВНИМАНИЕ: Обнаружены признаки перезагрузки ESP32")
            else:
                print("ESP32 подключена стабильно")

            return True

        except serial.SerialException as e:
            print(f"Ошибка подключения: {e}")
            return False

    def receive_json_data(self, timeout=30):
        """Прием JSON данных от ESP32"""
        if not self.ser:
            print("Нет подключения к ESP32")
            return None

        print("Ожидание JSON данных от ESP32...")

        json_data = ""
        start_time = time.time()
        json_started = False

        while time.time() - start_time < timeout:
            if self.ser.in_waiting > 0:
                try:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()

                    if line:
                        print(f"Получено: {line}")

                        # Ищем начало JSON данных
                        if line.startswith('{') or json_started:
                            json_started = True
                            json_data += line

                            # Проверяем, является ли собранная строка валидным JSON
                            if self._is_valid_json(json_data):
                                print("Валидный JSON получен!")
                                return json_data

                        # Проверяем конец передачи для notes
                        if "END_OF_NOTES_SENDING" in line:
                            print("Получен маркер конца передачи notes")
                            if self._is_valid_json(json_data):
                                return json_data

                except Exception as e:
                    print(f"Ошибка чтения: {e}")

            time.sleep(0.1)

        print("Таймаут ожидания данных")
        return None

    def _is_valid_json(self, json_str):
        """Проверка валидности JSON строки"""
        try:
            # Пытаемся найти JSON объект в строке
            json_match = re.search(r'\{.*\}', json_str, re.DOTALL)
            if json_match:
                json.loads(json_match.group())
                return True
            return False
        except:
            return False

    def parse_notes_to_dataframe(self, json_data):
        """Парсинг JSON данных с записями и создание DataFrame"""
        try:
            # Извлекаем JSON объект из строки
            json_match = re.search(r'\{.*\}', json_data, re.DOTALL)
            if not json_match:
                print("JSON объект не найден в полученных данных")
                return None

            clean_json = json_match.group()
            data = json.loads(clean_json)

            table_data = []

            if "notes" in data:
                for note in data["notes"]:
                    # Парсим данные записи
                    time_str = note.get("time", "")
                    uid = note.get("uid", "")
                    action = note.get("action", "")
                    key_number = note.get("key_number", "")

                    # Преобразуем время из timestamp в читаемый формат
                    readable_time = self._convert_timestamp(time_str)

                    # Создаем строку таблицы
                    row = {
                        "timestamp": time_str,
                        "datetime": readable_time,
                        "uid": uid,
                        "action": action,
                        "key_number": key_number
                    }

                    table_data.append(row)

            # Создаем DataFrame и сортируем по времени (от старых к новым)
            df = pd.DataFrame(table_data)
            if not df.empty:
                df = df.sort_values('timestamp').reset_index(drop=True)

            return df

        except Exception as e:
            print(f"Ошибка парсинга JSON notes: {e}")
            return None

    def _convert_timestamp(self, timestamp_str):
        """Конвертация timestamp в читаемое время"""
        try:
            # Пытаемся преобразовать строку в число
            timestamp = int(timestamp_str)
            # Конвертируем в читаемый формат
            dt = datetime.fromtimestamp(timestamp)
            return dt.strftime("%Y-%m-%d %H:%M:%S")
        except (ValueError, TypeError, OSError):
            # Если не удалось преобразовать, возвращаем оригинальную строку
            return timestamp_str

    def save_notes_to_excel(self, dataframe, filename=None):
        """Сохранение данных записей в Excel файл"""
        if dataframe is None or dataframe.empty:
            print("Нет данных записей для сохранения")
            return False

        if not filename:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"./received_data/esp32_notes_{timestamp}.xlsx"

        try:
            # Сохраняем в Excel с правильным форматированием
            with pd.ExcelWriter(filename, engine='openpyxl') as writer:
                dataframe.to_excel(writer, sheet_name='Notes', index=False)

                # Получаем workbook и worksheet для форматирования
                workbook = writer.book
                worksheet = writer.sheets['Notes']

                # Устанавливаем ширину колонок для лучшего отображения
                column_widths = {
                    'A': 15,   # timestamp
                    'B': 20,   # datetime
                    'C': 20,   # uid
                    'D': 15,   # action
                    'E': 12    # key_number
                }

                for col, width in column_widths.items():
                    worksheet.column_dimensions[col].width = width

            print(f"Данные записей успешно сохранены в файл: {filename}")
            print(f"Сохранено {len(dataframe)} записей")
            print("\nСтруктура таблицы записей:")
            print(dataframe.to_string(index=False))
            return True

        except Exception as e:
            print(f"Ошибка сохранения записей в Excel: {e}")
            return False

    def request_notes_from_esp32(self):
        """Запрос данных записей с ESP32"""
        if not self.ser:
            print("Нет подключения к ESP32")
            return False

        try:
            # Отправляем команду для получения записей
            print("Отправка запроса записей на ESP32...")
            self.ser.write(b"GET_NOTES\n")
            self.ser.flush()
            return True
        except Exception as e:
            print(f"Ошибка отправки запроса записей: {e}")
            return False

    def close(self):
        """Закрытие соединения"""
        if self.ser:
            self.ser.close()
            print("Соединение закрыто")


def main():
    # Создаем экземпляр класса
    esp_receiver = ESP32ToExcel('COM7', 9600)

    # Подключаемся к ESP32
    if not esp_receiver.connect():
        return

    try:
        # Запрашиваем записи с ESP32
        if esp_receiver.request_notes_from_esp32():
            # Получаем JSON данные
            json_data = esp_receiver.receive_json_data(timeout=30)

            if json_data:
                print("JSON данные записей получены успешно!")
                print("Парсинг данных записей...")

                # Парсим JSON в DataFrame
                dataframe = esp_receiver.parse_notes_to_dataframe(json_data)

                if dataframe is not None:
                    # Сохраняем в Excel
                    if esp_receiver.save_notes_to_excel(dataframe):
                        print("Операция завершена успешно!")
                    else:
                        print("Ошибка сохранения данных записей")
                else:
                    print("Ошибка парсинга данных записей или нет данных")
            else:
                print("Не удалось получить данные записей от ESP32")
        else:
            print("Не удалось отправить запрос записей на ESP32")

    except Exception as e:
        print(f"Ошибка в основном процессе: {e}")

    finally:
        esp_receiver.close()


if __name__ == "__main__":
    main()