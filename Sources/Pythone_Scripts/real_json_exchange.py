import pandas as pd
import json
import serial
import time


class ExcelToESP32:
    def __init__(self, port='COM7', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None

    def connect(self):
        """Подключение к ESP32"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=2)
            print(f"Подключено к {self.port}")
            time.sleep(3)
            self.ser.reset_input_buffer()
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
        """Отправка JSON данных на ESP32 в ОДНОЙ строке"""
        if not self.ser:
            print("Нет подключения к ESP32")
            return False

        try:
            # Преобразуем в JSON БЕЗ переносов строк и отступов
            json_str = json.dumps(json_data, ensure_ascii=False, separators=(',', ':'))
            print(f"Отправка JSON данных...")
            print(f"Размер данных: {len(json_str)} символов")
            print(f"JSON для отправки: {json_str}")

            # Отправляем данные в ОДНОЙ строке
            self.ser.write((json_str + '\n').encode('utf-8'))

            # Читаем ответ
            self.read_response()
            return True

        except Exception as e:
            print(f"Ошибка отправки: {e}")
            return False

    def read_response(self, timeout=10):
        """Чтение ответа от ESP32"""
        print("Ожидаем ответа от ESP32...")
        start_time = time.time()

        while time.time() - start_time < timeout:
            if self.ser.in_waiting > 0:
                line = self.ser.readline().decode('utf-8').strip()
                if line:
                    print(f"ESP32: {line}")

            time.sleep(0.1)

    def close(self):
        """Закрытие соединения"""
        if self.ser:
            self.ser.close()
            print("Соединение закрыто")


def main():
    # Создаем объект для работы с ESP32
    esp = ExcelToESP32('COM7', 115200)

    # Подключаемся к ESP32
    if not esp.connect():
        return

    try:
        # Путь к вашему Excel файлу
        excel_file = "data.xlsx"  # Замените на путь к вашему файлу

        # Преобразуем Excel в JSON
        json_data = esp.excel_to_json(excel_file)

        if json_data:
            # Выводим информацию о данных
            print(f"\nПреобразовано в JSON:")
            print(f"Таблица: {json_data['table_name']}")
            print(f"Колонок: {len(json_data['columns'])}")
            print(f"Строк: {json_data['rows_count']}")

            # Отправляем на ESP32
            print(f"\nОтправка на ESP32...")
            esp.send_json_data(json_data)

    except Exception as e:
        print(f"Ошибка: {e}")

    finally:
        esp.close()


# Упрощенная версия для тестирования
def simple_send():
    """Простая отправка тестового JSON"""
    try:
        ser = serial.Serial('COM7', 115200, timeout=2)
        time.sleep(3)
        ser.reset_input_buffer()

        # Тестовый JSON в одной строке
        test_data = {
            "table": "test",
            "columns": ["col1", "col2"],
            "data": [{"col1": "val1", "col2": "val2"}]
        }

        json_str = json.dumps(test_data, ensure_ascii=False, separators=(',', ':'))
        print(f"Отправка: {json_str}")

        ser.write((json_str + '\n').encode('utf-8'))

        # Читаем ответ
        print("Ожидаем ответа...")
        time.sleep(2)

        while ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            if line:
                print(f"ESP32: {line}")

        ser.close()

    except Exception as e:
        print(f"Ошибка: {e}")


if __name__ == "__main__":
    # Сначала протестируйте простую отправку
    print("=== ТЕСТ ПРОСТОЙ ОТПРАВКИ ===")
    simple_send()

    time.sleep(2)

    # Затем основную функцию
    print("\n=== ОСНОВНАЯ ОТПРАВКА ===")
    main()