import json
import serial
import pandas as pd
from datetime import datetime, timedelta
import time
import re
import os


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
        """Прием JSON данных от ESP32 с гарантированным выходом по маркеру"""
        if not self.ser:
            print("Нет подключения к ESP32")
            return None

        print("Ожидание JSON данных от ESP32...")

        buffer = ""
        start_time = time.time()
        json_started = False
        transmission_complete = False

        while time.time() - start_time < timeout and not transmission_complete:
            if self.ser.in_waiting > 0:
                try:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()

                    if line:
                        print(f"Получено: {line}")
                        buffer += line + "\n"

                        # Отслеживаем начало JSON данных
                        if not json_started and ('{' in line or '[' in line):
                            json_started = True
                            print("Начало JSON данных обнаружено")

                        # Проверяем маркер конца передачи - ВЫХОДИМ НЕМЕДЛЕННО
                        if "END_OF_NOTES_SENDING" in line:
                            print("Получен маркер конца передачи notes - завершаем прием")
                            transmission_complete = True
                            break  # Немедленный выход из цикла

                except Exception as e:
                    print(f"Ошибка чтения: {e}")
            else:
                # Короткая пауза если нет данных
                time.sleep(0.05)

        # Обрабатываем результат после выхода из цикла
        if transmission_complete:
            print("Передача завершена по маркеру")
            # Извлекаем JSON из буфера
            json_match = re.search(r'(\{.*\}|\[.*\])', buffer, re.DOTALL)
            if json_match:
                return json_match.group()
            else:
                print("JSON не найден в полученных данных")
                return None
        else:
            print("Таймаут ожидания данных")
            return None

    def _convert_uid_to_hex(self, uid_str):
        """Конвертация UID из строки в hex формат 0x576E7A5B"""
        try:
            # Если UID уже в читаемом формате, возвращаем как есть
            if all(c in '0123456789abcdefABCDEF' for c in uid_str.replace(' ', '').replace('0x', '')):
                # Убираем пробелы и добавляем префикс 0x если его нет
                clean_uid = uid_str.replace(' ', '').upper()
                if not clean_uid.startswith('0x'):
                    clean_uid = '0x' + clean_uid
                return clean_uid

            # Если это бинарные данные в строке, конвертируем в hex формат 0x576E7A5B
            hex_bytes = []
            for char in uid_str:
                hex_bytes.append(f"{ord(char):02X}")

            # Объединяем все байты в одну hex строку с префиксом 0x
            hex_uid = '0x' + ''.join(hex_bytes)
            return hex_uid

        except Exception as e:
            print(f"Ошибка конвертации UID '{uid_str}': {e}")
            return uid_str

    def _convert_millis_to_datetime(self, millis_str, connection_time):
        """Конвертация миллисекунд millis() в реальное время"""
        try:
            millis = int(millis_str)

            # Вычисляем реальное время: время подключения + прошедшие миллисекунды
            real_time = connection_time + timedelta(milliseconds=millis)

            return real_time.strftime("%Y-%m-%d %H:%M:%S")
        except (ValueError, TypeError) as e:
            print(f"Ошибка конвертации времени '{millis_str}': {e}")
            return f"Ошибка времени: {millis_str}"

    def parse_notes_to_dataframe(self, json_data):
        """Парсинг JSON данных с записями и создание DataFrame"""
        try:
            # Запоминаем время подключения для вычисления реального времени
            connection_time = datetime.now()

            if not json_data or json_data.strip() == "":
                print("Получены пустые данные")
                return pd.DataFrame()  # Возвращаем пустой DataFrame

            data = json.loads(json_data)
            table_data = []

            if "notes" in data and data["notes"]:
                for note in data["notes"]:
                    # Парсим данные записи
                    millis_str = note.get("time", "")
                    uid = note.get("uid", "")
                    action = note.get("action", "")
                    key_number = note.get("key_number", "")

                    # Преобразуем UID в hex формат 0x576E7A5B
                    readable_uid = self._convert_uid_to_hex(uid)

                    # Преобразуем миллисекунды в реальное время
                    readable_time = self._convert_millis_to_datetime(millis_str, connection_time)

                    # Создаем строку таблицы (ТОЛЬКО реальное время, без timestamp)
                    row = {
                        "datetime": readable_time,
                        "uid": readable_uid,
                        "action": action,
                        "key_number": key_number
                    }
                    table_data.append(row)

                    print(f"Запись: время={readable_time}, UID={readable_uid}, действие={action}, ключ={key_number}")

                print(f"Найдено {len(table_data)} записей")
            else:
                print("В JSON нет записей или пустой массив notes")

            # Создаем DataFrame и сортируем по времени
            df = pd.DataFrame(table_data)
            if not df.empty:
                # Сортируем по времени
                df = df.sort_values('datetime').reset_index(drop=True)

            return df

        except json.JSONDecodeError as e:
            print(f"Ошибка декодирования JSON: {e}")
            print(f"Полученные данные: {json_data}")
            return None
        except Exception as e:
            print(f"Ошибка парсинга JSON notes: {e}")
            return None

    def save_notes_to_excel(self, dataframe, filename=None):
        """Сохранение данных записей в Excel файл"""
        if dataframe is None or dataframe.empty:
            print("Нет данных записей для сохранения")
            return False

        # Создаем директорию если не существует
        os.makedirs("./received_data", exist_ok=True)

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
                    'A': 20,  # datetime
                    'B': 15,  # uid (теперь короче, т.к. формат 0x576E7A5B)
                    'C': 15,  # action
                    'D': 12  # key_number
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
            time.sleep(1)  # Даем время на обработку команды
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

    except KeyboardInterrupt:
        print("\nПрервано пользователем")
    except Exception as e:
        print(f"Ошибка в основном процессе: {e}")

    finally:
        esp_receiver.close()


if __name__ == "__main__":
    main()