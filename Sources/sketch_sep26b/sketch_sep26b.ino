#include <SPI.h>
#include <MFRC522.h>

#include "Data_Base.h"

#define SS_PIN 5
#define RST_PIN 21

// Создаем объект MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::StatusCode status;

Data_Base data_base;

void setup() {
  Serial.begin(9600); // Инициализация Serial-порта
  SPI.begin();          // Инициализация шины SPI
  mfrc522.PCD_Init();   // Инициализация RFID-модуля

  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);  // Установка усиления антенны
  mfrc522.PCD_AntennaOff();           // Перезагружаем антенну
  mfrc522.PCD_AntennaOn();            // Включаем антенну

  delay(5000);
  Serial.println("Inicialization saccess");

  data_base.add_person(0x12);  // Пример добавления пользователя
  data_base.add_person(0x34);
  data_base.print_persons_data();
}

void loop() { 

  static uint32_t rebootTimer = millis(); // Важный костыль против зависания модуля!
  if (millis() - rebootTimer >= 1000) {   // Таймер с периодом 1000 мс
    rebootTimer = millis();               // Обновляем таймер
    digitalWrite(RST_PIN, HIGH);          // Сбрасываем модуль
    delayMicroseconds(2);                 // Ждем 2 мкс
    digitalWrite(RST_PIN, LOW);           // Отпускаем сброс
    mfrc522.PCD_Init();                      // Инициализируем заного
  }

  if (!mfrc522.PICC_IsNewCardPresent()) return; // Проверяем, поднесена ли новая метка. Если нет, выходим из loop и проверяем снова
  if (!mfrc522.PICC_ReadCardSerial()) return; // Пытаемся прочитать данные метки. Если не удалось, выходим из loop

  // Выводим UID метки в шестнадцатеричном формате
  Serial.print("UID метки: ");
  for (uint8_t  i = 0; i < mfrc522.uid.size; i++) {
    Serial.print("0x");
    // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(", ");
  }
  Serial.println();

  // Останавливаем работу с меткой
  // mfrc522.PICC_HaltA();
  delay(1000);
}