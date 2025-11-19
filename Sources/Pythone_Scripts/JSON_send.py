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

    def excel_to_json_rows(self, file_path, sheet_name=0):
        """
        Чтение Excel файла и преобразование в список JSON строк
        """
        try:
            # Читаем Excel файл
            df = pd.read_excel(file_path, sheet_name=sheet_name)
            print(f"Файл прочитан: {file_path}")
            print(f"Размер таблицы: {df.shape[0]} строк, {df.shape[1]} столбцов")
            print(f"Столбцы: {list(df.columns)}")

            # Заменяем NaN значения на пустые строки
            df = df.fillna('')

            df = df.astype(str)

            # Создаем список всех строк данных (только сырые данные)
            rows_data = []
            for index, row in df.iterrows():
                row_data = row.to_dict()
                rows_data.append(row_data)

            return rows_data

        except Exception as e:
            print(f"Ошибка чтения Excel файла: {e}")
            return None

    def send_json_data_row_by_row(self, json_rows):
        """Отправка JSON данных на ESP32 построчно"""
        if not self.ser:
            print("Нет подключения к ESP32")
            return False

        try:
            # Активация JSON режима - отправляем START_JSON_TEST
            print("Активация JSON режима...")
            self.ser.write("START_JSON_UPLOAD\n".encode('utf-8'))
            time.sleep(2)

            total_rows = len(json_rows)
            print(f"Будет отправлено {total_rows} строк данных")

            # Отправляем каждый JSON объект по отдельности
            for i, row_data in enumerate(json_rows):
                print(f"Отправка строки {i + 1}/{total_rows}...")

                # Преобразуем в JSON строку
                json_str = json.dumps(row_data, ensure_ascii=False, separators=(',', ':'))

                # Отправляем строку
                self.ser.write((json_str + '\n').encode('utf-8'))
                self.ser.flush()

                # Небольшая задержка между строками для стабильности
                time.sleep(0.1)

            # ОТПРАВЛЯЕМ СТРОКУ END_OF_SESSION В КОНЦЕ
            print("Отправка команды END_OF_SESSION для завершения...")
            self.ser.write("END_OF_SESSION\n".encode('utf-8'))
            self.ser.flush()

            print("Все строки и команда завершения отправлены успешно!")
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
                               ['COMPLETE', 'FINISHED', 'ЗАВЕРШЕНО', 'ВСЕ_СТРОКИ',
                                'ПУСТАЯ_СТРОКА_ПРИНЯТА', 'SESSION_ENDED', 'FINISHING_OF_JSON_PROCESSING']):
                            print("Обработка всех строк завершена")
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


def main():
    esp = ExcelToESP32('COM7', 9600)

    if not esp.connect():
        return

    try:
        excel_file = "./sending_data/data.xlsx"
        json_rows = esp.excel_to_json_rows(excel_file)

        if json_rows:
            print(f"\nПодготовлено {len(json_rows)} строк данных для отправки")
            print(f"Начинаем построчную отправку на ESP32...")

            if esp.send_json_data_row_by_row(json_rows):
                print("Чтение финального ответа...")
                esp.read_response(timeout=10)
            else:
                print("Ошибка при отправке данных")

    except Exception as e:
        print(f"Ошибка: {e}")

    finally:
        esp.close()


if __name__ == "__main__":
    main()