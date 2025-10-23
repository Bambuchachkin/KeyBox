import serial
import time


def send_csv_to_esp32(port, filename):
    try:
        ser = serial.Serial(port, 9600, timeout=1)
        time.sleep(2)

        # Сначала отправляем команду активации режима приема
        ser.write(b"START_CSV_UPLOAD\n")
        time.sleep(0.5)

        # Ждем подтверждения
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                print(f"ESP32: {line}")
                if "READY_FOR_CSV" in line:
                    break

        print("Начинаю отправку файла...")
        # Отправляем файл
        with open(filename, 'r', encoding='utf-8') as file:
            csv_content = file.read()
            # Отправляем частями
            chunk_size = 64
            for i in range(0, len(csv_content), chunk_size):
                chunk = csv_content[i:i + chunk_size]
                ser.write(chunk.encode('utf-8'))
                time.sleep(0.05)  # Небольшая задержка между chunk'ами

        # Сигнал конца файла
        ser.write(b"END_CSV_FILE\n")
        print("Файл отправлен, ожидаю обработки...")

        # Ждем подтверждения обработки
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                print(f"ESP32: {line}")
                if "CSV_FILE_DELETED" in line:
                    break

        ser.close()
        print("Готово!")

    except Exception as e:
        print(f"Ошибка: {e}")


# Использование
send_csv_to_esp32('COM7', 'data.csv')