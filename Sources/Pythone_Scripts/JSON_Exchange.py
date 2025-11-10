import serial
import json
import time

# Простой скрипт для отправки одного JSON
port = 'COM7'  # Замените на ваш порт
baudrate = 115200

# Данные для отправки
data = {
    "message": "Hello ESP32",
    "number": 42,
    "values": [1, 2, 3, 4, 5],
    "timestamp": time.time()
}

try:
    # Подключаемся к порту
    ser = serial.Serial(port, baudrate, timeout=2)
    print(f"Подключено к {port}")

    # Ждем готовности ESP32
    time.sleep(2)

    # Отправляем JSON
    json_str = json.dumps(data)
    print(f"Отправка: {json_str}")
    ser.write((json_str + '\n').encode())

    # Читаем ответ
    print("Ответ от ESP32:")
    start_time = time.time()
    while time.time() - start_time < 5:  # Ждем 5 секунд
        if ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            if line:
                print(f"-> {line}")
        time.sleep(0.1)

    ser.close()

except Exception as e:
    print(f"Ошибка: {e}")