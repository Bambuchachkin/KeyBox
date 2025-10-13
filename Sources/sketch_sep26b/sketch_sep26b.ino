#include <SPI.h>
#include <MFRC522.h>

#include "Data_Base.h"
#include "Terminal.h"

#define SS_PIN 5
#define RST_PIN 21
#define ANALOG_PIN 35

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::StatusCode status;

Terminal terminal;
std::vector<uint8_t> received_UID;

// Функция для измерения уровня сигнала на аналоговом порту
void readAnalogSignal() {
  int sensorValue = analogRead(ANALOG_PIN);        // Чтение аналогового значения (0-1023)
  float voltage = sensorValue * (3.3 / 4095.0);    // Конвертация в напряжение (для ESP32)
  
  Serial.print("Analog Pin A0 - Value: ");
  Serial.print(sensorValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage);
  Serial.println("V");
}
// Функция для измерения уровня сигнала на цифровом порту
void readDigitalSignal(int pin) {
  int digitalValue = digitalRead(pin);             // Чтение цифрового значения (0 или 1)
  
  Serial.print("Digital Pin ");
  Serial.print(pin);
  Serial.print(" - Value: ");
  Serial.println(digitalValue);
}

void setup() {
  Serial.begin(9600); // Инициализация Serial-порта
  SPI.begin();          // Инициализация шины SPI
  mfrc522.PCD_Init();   // Инициализация RFID-модуля

  pinMode(ANALOG_PIN, INPUT);  // Настройка аналогового пина как вход

  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);  // Установка усиления антенны
  mfrc522.PCD_AntennaOff();           // Перезагружаем антенну
  mfrc522.PCD_AntennaOn();            // Включаем антенну

  delay(5000);
  Serial.print("\n");
  Serial.println("Inicialization saccess");
}

void loop() { 

  std::vector<std::string> cmd = terminal.read_command();
  if (cmd.size() > 0) {
      terminal.process_command(cmd);
  }
  delay(100);

  static uint32_t rebootTimer = millis(); // Важный костыль против зависания модуля!
  if (millis() - rebootTimer >= 1000) {   // Таймер с периодом 1000 мс
    rebootTimer = millis();               // Обновляем таймер
    digitalWrite(RST_PIN, HIGH);          // Сбрасываем модуль
    delayMicroseconds(2);                 // Ждем 2 мкс
    digitalWrite(RST_PIN, LOW);           // Отпускаем сброс
    mfrc522.PCD_Init();                      // Инициализируем заного
  }

  // Выводим уровень сигнала каждую секунду
    // readAnalogSignal();
    // delay(50);
    
    // Если нужно читать цифровой пин, раскомментируйте:
    // readDigitalSignal(2);

  if (!mfrc522.PICC_IsNewCardPresent()) return; // Проверяем, поднесена ли новая метка. Если нет, выходим из loop и проверяем снова
  if (!mfrc522.PICC_ReadCardSerial()) return; // Пытаемся прочитать данные метки. Если не удалось, выходим из loop

  received_UID.clear();
  for (uint8_t  i = 0; i < mfrc522.uid.size; i++) {
    received_UID.push_back(mfrc522.uid.uidByte[i]);
  }

  terminal.buffer_UID(received_UID);
  
  // Останавливаем работу с меткой
  // mfrc522.PICC_HaltA();
  delay(1000);
}