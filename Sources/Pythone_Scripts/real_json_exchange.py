#
#
# import pandas as pd
# import json
# import serial
# import time
#
#
# class ExcelToESP32:
#     def __init__(self, port='COM7', baudrate=9600):
#         self.port = port
#         self.baudrate = baudrate
#         self.ser = None
#
#     def connect(self):
#         """Подключение к ESP32 с правильными настройками"""
#         try:
#             # Важно: отключаем DTR и RTS
#             self.ser = serial.Serial(
#                 port=self.port,
#                 baudrate=self.baudrate,
#                 bytesize=serial.EIGHTBITS,
#                 parity=serial.PARITY_NONE,
#                 stopbits=serial.STOPBITS_ONE,
#                 timeout=2,
#                 dsrdtr=False,  # ОТКЛЮЧАЕМ DTR
#                 rtscts=False,  # ОТКЛЮЧАЕМ RTS/CTS
#                 xonxoff=False  # ОТКЛЮЧАЕМ XON/XOFF
#             )
#
#             print(f"Подключено к {self.port}")
#
#             # Даем ESP32 время на восстановление
#             print("Ждем стабилизации ESP32...")
#             time.sleep(4)
#
#             # Очищаем буферы
#             self.ser.reset_input_buffer()
#             self.ser.reset_output_buffer()
#
#             # Проверяем связь
#             self.ser.write(b'\n')
#             time.sleep(1)
#
#             # Читаем приветственное сообщение
#             print("Проверка связи с ESP32...")
#             time.sleep(2)
#             while self.ser.in_waiting > 0:
#                 line = self.ser.readline().decode('utf-8', errors='ignore').strip()
#                 if line:
#                     print(f"ESP32: {line}")
#
#             return True
#
#         except serial.SerialException as e:
#             print(f"Ошибка подключения: {e}")
#             return False
#
#     def excel_to_json(self, file_path, sheet_name=0):
#         """
#         Чтение Excel файла и преобразование в JSON
#         """
#         try:
#             # Читаем Excel файл
#             df = pd.read_excel(file_path, sheet_name=sheet_name)
#             print(f"Файл прочитан: {file_path}")
#             print(f"Размер таблицы: {df.shape[0]} строк, {df.shape[1]} столбцов")
#             print(f"Столбцы: {list(df.columns)}")
#
#             # Заменяем NaN значения на пустые строки
#             df = df.fillna('')
#
#             # Преобразуем в компактный JSON
#             data = {
#                 "source_file": file_path,
#                 "table_name": sheet_name if isinstance(sheet_name, str) else f"sheet_{sheet_name}",
#                 "columns": list(df.columns),
#                 "rows_count": len(df),
#                 "data": df.to_dict('records')
#             }
#
#             return data
#
#         except Exception as e:
#             print(f"Ошибка чтения Excel файла: {e}")
#             return None
#
#     def send_json_data(self, json_data):
#         """Отправка JSON данных на ESP32"""
#         if not self.ser:
#             print("Нет подключения к ESP32")
#             return False
#
#         try:
#             # Сначала отправляем команду для активации JSON режима
#             print("Активация JSON режима...")
#             self.ser.write("START_JSON_TEST\n".encode('utf-8'))
#             time.sleep(2)  # Даем время на переключение режима
#
#             # Преобразуем в JSON
#             json_str = json.dumps(json_data, ensure_ascii=False, separators=(',', ':'))
#             print(f"Отправка JSON данных ({len(json_str)} символов)...")
#
#             # Отправляем данные
#             self.ser.write((json_str + '\n').encode('utf-8'))
#             self.ser.flush()  # Ждем отправки всех данных
#
#             print("Данные отправлены, ждем ответа...")
#
#             # Читаем ответ с увеличенным таймаутом
#             self.read_response(timeout=15)
#             return True
#
#         except Exception as e:
#             print(f"Ошибка отправки: {e}")
#             return False
#
#     def read_response(self, timeout=15):
#         """Чтение ответа от ESP32"""
#         print(f"Ожидаем ответа от ESP32 ({timeout} сек)...")
#         start_time = time.time()
#
#         response_lines = []
#
#         while time.time() - start_time < timeout:
#             if self.ser.in_waiting > 0:
#                 try:
#                     line = self.ser.readline().decode('utf-8', errors='ignore').strip()
#                     if line:
#                         print(f"ESP32: {line}")
#                         response_lines.append(line)
#
#                         # Если получили признак завершения обработки
#                         if any(keyword in line for keyword in ['JSON_MODE_ENDED', 'processed', 'COMPLETE']):
#                             print("Обработка JSON завершена")
#                             break
#                 except Exception as e:
#                     print(f"Ошибка чтения: {e}")
#
#             time.sleep(0.1)
#
#         if not response_lines:
#             print("ESP32 не ответила")
#
#         return response_lines
#
#     def close(self):
#         """Закрытие соединения"""
#         if self.ser:
#             self.ser.close()
#             print("Соединение закрыто")
#
#
# def test_connection():
#     """Тест соединения без отправки данных"""
#     try:
#         ser = serial.Serial('COM7', 9600, timeout=2, dsrdtr=False, rtscts=False)
#         print("Порт открыт, наблюдаем за ESP32...")
#
#         # Наблюдаем 10 секунд
#         start_time = time.time()
#         while time.time() - start_time < 10:
#             if ser.in_waiting > 0:
#                 line = ser.readline().decode('utf-8', errors='ignore').strip()
#                 print(f"ESP32: {line}")
#             time.sleep(0.1)
#
#         ser.close()
#         print("Тест завершен")
#
#     except Exception as e:
#         print(f"Ошибка теста: {e}")
#
#
# def main():
#     print("=== ТЕСТ СОЕДИНЕНИЯ ===")
#     test_connection()
#
#     time.sleep(2)
#     print("\n=== ОСНОВНАЯ ОТПРАВКА ===")
#
#     esp = ExcelToESP32('COM7', 9600)
#
#     if not esp.connect():
#         return
#
#     try:
#         excel_file = "data.xlsx"
#         json_data = esp.excel_to_json(excel_file)  # Теперь этот метод существует!
#
#         if json_data:
#             print(f"\nПреобразовано в JSON:")
#             print(f"Таблица: {json_data['table_name']}")
#             print(f"Колонок: {len(json_data['columns'])}")
#             print(f"Строк: {json_data['rows_count']}")
#
#             print(f"\nОтправка на ESP32...")
#             esp.send_json_data(json_data)
#
#     except Exception as e:
#         print(f"Ошибка: {e}")
#
#     finally:
#         esp.close()
#
#
# if __name__ == "__main__":
#     main()

import pandas as pd
import json
import serial
import time


class ExcelToESP32:
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

    def excel_to_json(self, file_path, sheet_name=0):
        """
        Чтение Excel файла и преобразование в JSON
        """
        try:
            # Читаем Excel файл
            df = pd.read_excel(file_path, sheet_name=sheet_name)
            print(f"Файл прочитан: {file_path}")
            print(f"Размер таблицы: {df.shape[0]} строк, {df.shape[1]} столбцов")
            print(f"Столбцы: {list(df.columns)}")

            # Заменяем NaN значения на пустые строки
            df = df.fillna('')

            # Преобразуем в компактный JSON
            data = {
                "source_file": file_path,
                "table_name": sheet_name if isinstance(sheet_name, str) else f"sheet_{sheet_name}",
                "columns": list(df.columns),
                "rows_count": len(df),
                "data": df.to_dict('records')
            }

            return data

        except Exception as e:
            print(f"Ошибка чтения Excel файла: {e}")
            return None

    def send_json_data(self, json_data):
        """Отправка JSON данных на ESP32"""
        if not self.ser:
            print("Нет подключения к ESP32")
            return False

        try:
            # Сначала отправляем команду для активации JSON режима
            print("Активация JSON режима...")
            self.ser.write("START_JSON_TEST\n".encode('utf-8'))
            time.sleep(2)  # Даем время на переключение режима

            # Преобразуем в JSON
            json_str = json.dumps(json_data, ensure_ascii=False, separators=(',', ':'))
            print(f"Отправка JSON данных ({len(json_str)} символов)...")

            # Отправляем данные
            self.ser.write((json_str + '\n').encode('utf-8'))
            self.ser.flush()  # Ждем отправки всех данных

            print("Данные отправлены, ждем ответа...")

            # Читаем ответ с увеличенным таймаутом
            self.read_response(timeout=15)
            return True

        except Exception as e:
            print(f"Ошибка отправки: {e}")
            return False

    def read_response(self, timeout=15):
        """Чтение ответа от ESP32"""
        print(f"Ожидаем ответа от ESP32 ({timeout} сек)...")
        start_time = time.time()

        response_lines = []

        while time.time() - start_time < timeout:
            if self.ser.in_waiting > 0:
                try:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"ESP32: {line}")
                        response_lines.append(line)

                        # Если получили признак завершения обработки
                        if any(keyword in line for keyword in
                               ['JSON_MODE_ENDED', 'processed', 'COMPLETE', 'ОБРАБОТАН']):
                            print("Обработка JSON завершена")
                            break
                except Exception as e:
                    print(f"Ошибка чтения: {e}")

            time.sleep(0.1)

        if not response_lines:
            print("ESP32 не ответила")

        return response_lines

    def close(self):
        """Закрытие соединения"""
        if self.ser:
            self.ser.close()
            print("Соединение закрыто")


def test_no_reboot():
    """Тест для проверки отсутствия перезагрузки ESP32"""
    try:
        ser = serial.Serial('COM7', 9600, timeout=2, dsrdtr=False, rtscts=False)

        print("Проверка начального состояния...")
        time.sleep(0.5)

        initial_data = []
        start_time = time.time()
        while time.time() - start_time < 3:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    initial_data.append(line)
                    print(f"Начальные данные: {line}")

        # Проверяем наличие признаков перезагрузки
        reboot_indicators = ['ets Jun', 'rst', 'boot', 'load', 'entry']
        has_reboot = any(any(indicator in line for indicator in reboot_indicators)
                         for line in initial_data)

        if has_reboot:
            print("ОБНАРУЖЕНА ПЕРЕЗАГРУЗКА ESP32")
        else:
            print("ESP32 не перезагружалась")

        ser.close()
        return not has_reboot

    except Exception as e:
        print(f"Ошибка теста: {e}")
        return False


def main():
    print("=== ТЕСТ ОТСУТСТВИЯ ПЕРЕЗАГРУЗКИ ===")
    stable_connection = test_no_reboot()

    if not stable_connection:
        print("ПРЕДУПРЕЖДЕНИЕ: ESP32 перезагружается при подключении")
        print("Рекомендуется аппаратное решение проблемы")
        return

    time.sleep(2)
    print("\n=== ОСНОВНАЯ ОТПРАВКА ===")

    esp = ExcelToESP32('COM7', 9600)

    if not esp.connect():
        return

    try:
        excel_file = "data.xlsx"
        json_data = esp.excel_to_json(excel_file)

        if json_data:
            print(f"\nПреобразовано в JSON:")
            print(f"Таблица: {json_data['table_name']}")
            print(f"Колонок: {len(json_data['columns'])}")
            print(f"Строк: {json_data['rows_count']}")

            print(f"\nОтправка на ESP32...")
            esp.send_json_data(json_data)

    except Exception as e:
        print(f"Ошибка: {e}")

    finally:
        esp.close()


if __name__ == "__main__":
    main()