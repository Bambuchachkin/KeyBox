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

                        # Проверяем конец передачи
                        if "END_OF_JSON_SENDING" in line:
                            print("Получен маркер конца передачи")
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

    def parse_json_to_dataframe(self, json_data):
        """Парсинг JSON данных и создание DataFrame в нужном формате"""
        try:
            # Извлекаем JSON объект из строки
            json_match = re.search(r'\{.*\}', json_data, re.DOTALL)
            if not json_match:
                print("JSON объект не найден в полученных данных")
                return None

            clean_json = json_match.group()
            data = json.loads(clean_json)

            table_data = []

            if "persons" in data:
                for person in data["persons"]:
                    # Парсим UID
                    uid = self._parse_uid(person.get("uid", []))

                    # Получаем имя и номер
                    name = person.get("name", "")
                    number = person.get("number", "")

                    # Обрабатываем available_keys
                    available_keys = person.get("available_keys", [])
                    on_hands_keys = person.get("on_hands_keys", [])

                    # Форматируем available и on_hands как в примере
                    available_str = self._format_keys(available_keys)
                    on_hands_str = self._format_keys(on_hands_keys)

                    # Создаем строку таблицы
                    row = {
                        "number": number,
                        "uid": uid,  # Исправлено с "uid" на "did" как в вашем примере
                        "name": name,
                        "available": available_str,
                        "on_hands": on_hands_str
                    }

                    table_data.append(row)

            # Создаем DataFrame и сортируем по номеру
            df = pd.DataFrame(table_data)
            if not df.empty:
                df = df.sort_values('number').reset_index(drop=True)

            return df

        except Exception as e:
            print(f"Ошибка парсинга JSON: {e}")
            return None

    def _format_keys(self, keys_array):
        """Форматирование массива ключей в строку"""
        if not keys_array:
            return "-1"

        # Фильтруем -1 значения
        filtered_keys = [str(key) for key in keys_array if key != -1]

        if not filtered_keys:
            return "-1"

        return " ".join(filtered_keys)

    def _parse_uid(self, uid_array):
        """Парсинг UID из массива чисел в hex строку"""
        try:
            if isinstance(uid_array, list):
                # Преобразуем массив чисел в hex строку с префиксом 0x
                uid_bytes = bytes(uid_array)
                return "0x" + uid_bytes.hex().upper()
            return str(uid_array)
        except:
            return str(uid_array)

    def save_to_excel(self, dataframe, filename=None):
        """Сохранение данных в Excel файл"""
        if dataframe is None or dataframe.empty:
            print("Нет данных для сохранения")
            return False

        if not filename:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"./received_data/esp32_persons_{timestamp}.xlsx"

        try:
            # Сохраняем в Excel с правильным форматированием
            with pd.ExcelWriter(filename, engine='openpyxl') as writer:
                dataframe.to_excel(writer, sheet_name='Persons', index=False)

                # Получаем workbook и worksheet для форматирования
                workbook = writer.book
                worksheet = writer.sheets['Persons']

                # Устанавливаем ширину колонок для лучшего отображения
                column_widths = {
                    'A': 8,    # number
                    'B': 15,   # did
                    'C': 25,   # name
                    'D': 15,   # available
                    'E': 10    # on_hands
                }

                for col, width in column_widths.items():
                    worksheet.column_dimensions[col].width = width

            print(f"Данные успешно сохранены в файл: {filename}")
            print(f"Сохранено {len(dataframe)} записей")
            print("\nСтруктура таблицы:")
            print(dataframe.to_string(index=False))
            return True

        except Exception as e:
            print(f"Ошибка сохранения в Excel: {e}")
            return False

    def request_data_from_esp32(self):
        """Запрос данных с ESP32"""
        if not self.ser:
            print("Нет подключения к ESP32")
            return False

        try:
            # Отправляем команду для получения данных
            print("Отправка запроса данных на ESP32...")
            self.ser.write(b"GET_JSON_DATA\n")
            self.ser.flush()
            return True
        except Exception as e:
            print(f"Ошибка отправки запроса: {e}")
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
        # Запрашиваем данные с ESP32
        if esp_receiver.request_data_from_esp32():
            # Получаем JSON данные
            json_data = esp_receiver.receive_json_data(timeout=30)

            if json_data:
                print("JSON данные получены успешно!")
                print("Парсинг данных...")

                # Парсим JSON в DataFrame
                dataframe = esp_receiver.parse_json_to_dataframe(json_data)

                if dataframe is not None:
                    # Сохраняем в Excel
                    if esp_receiver.save_to_excel(dataframe):
                        print("Операция завершена успешно!")
                    else:
                        print("Ошибка сохранения данных")
                else:
                    print("Ошибка парсинга данных или нет данных")
            else:
                print("Не удалось получить данные от ESP32")
        else:
            print("Не удалось отправить запрос на ESP32")

    except Exception as e:
        print(f"Ошибка в основном процессе: {e}")

    finally:
        esp_receiver.close()


if __name__ == "__main__":
    main()