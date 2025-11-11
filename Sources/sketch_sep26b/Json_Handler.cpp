#include "Json_Handler.h"

Json_Handler::Json_Handler() {
    comPort = &Serial;
    Serial.begin(9600);
    delay(2000);
    Serial.println("=== ESP32 JSON Handler ===");
    Serial.println("Готов к приему JSON...");
}

int Json_Handler::waitAndProcessJSON() {
  if (!comPort->available()){return 1;}
  String receivedData = comPort->readStringUntil('\n');
  receivedData.trim();
            
  Serial.print("Получены данные: ");
  Serial.println(receivedData);
  Serial.print("Длина данных: ");
  Serial.println(receivedData.length());
            
  if (receivedData.length() == 0) {
    Serial.println("Пустая строка");
    return 1;
  }
            
  // Парсим JSON
  JSONVar jsonDoc = JSON.parse(receivedData);
  String jsonType = JSON.typeof(jsonDoc);
  Serial.print("Тип JSON: ");
  Serial.println(jsonType);
            
  if (jsonType == "undefined") {
    Serial.println("Ошибка: Неверный JSON формат");
    return 1;
  }
            
  Serial.println("JSON таблица получена");
            
  // Добавляем пометку
  jsonDoc["processed_by_esp32"] = true;
  jsonDoc["processed_at"] = (long)millis();
            
  // Отправляем обратно
  String output = JSON.stringify(jsonDoc);
  Serial.println(output);

  return 0;
}