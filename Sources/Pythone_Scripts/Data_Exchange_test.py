import serial
import time


def send_csv_to_esp32(port, filename):
    try:
        print("Жду завершения загрузки ESP32...")
        time.sleep(5)  # Даем ESP32 полностью загрузиться ДО открытия порта

        # Быстрое открытие порта с минимальными настройками
        ser = serial.Serial(port, 9600, timeout=1)

        # Немедленно отключаем DTR
        ser.setDTR(False)
        ser.setRTS(False)

        time.sleep(2)  # Короткая пауза после открытия

        # Очистка буфера
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        print("Отправляю команду START_CSV_UPLOAD...")
        ser.write(b"START_CSV_UPLOAD\n")

        # Ждем READY_FOR_CSV
        print("Ожидаю READY_FOR_CSV...")
        start_time = time.time()

        while time.time() - start_time < 10:  # Увеличил таймаут
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"ESP32: {line}")
                    if "READY_FOR_CSV" in line:
                        break
            time.sleep(0.1)
        else:
            print("Таймаут ожидания READY_FOR_CSV")
            ser.close()
            return

        print("Начинаю отправку файла...")

        with open(filename, 'r', encoding='utf-8') as file:
            for line_num, line in enumerate(file, 1):
                line = line.strip()
                if line:
                    ser.write((line + '\n').encode('utf-8'))
                    print(f"Отправлена строка {line_num}")
                    time.sleep(0.2)

        time.sleep(1)

        print("Отправляю END_CSV_FILE...")
        ser.write(b"END_CSV_FILE\n")
        time.sleep(1)

        print("Ожидаю подтверждения...")
        start_time = time.time()
        while time.time() - start_time < 10:
            if ser.in_waiting > 0:
                response = ser.readline().decode('utf-8', errors='ignore').strip()
                if response:
                    print(f"ESP32: {response}")
                    if "CSV_FILE_DELETED" in response:
                        break
            time.sleep(0.1)

        ser.close()
        print("Готово!")

    except Exception as e:
        print(f"Ошибка: {e}")
        try:
            ser.close()
        except:
            pass


if __name__ == "__main__":
    send_csv_to_esp32('COM7', 'data.csv')