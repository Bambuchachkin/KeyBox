#include "Json_Handler.h"

#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>

std::vector<uint8_t> hexStringToVector(const std::string& hexStr) {
    std::vector<uint8_t> result;
    // Проверяем, начинается ли строка с "0x" или "0X"
    std::string cleanStr = hexStr;
    if (hexStr.length() >= 2 && hexStr.substr(0, 2) == "0x") {
        cleanStr = hexStr.substr(2); // Убираем "0x"
    } else if (hexStr.length() >= 2 && hexStr.substr(0, 2) == "0X") {
        cleanStr = hexStr.substr(2); // Убираем "0X"
    }
    // Длина строки должна быть четной
    if (cleanStr.length() % 2 != 0) {
        cleanStr = "0" + cleanStr; // Добавляем ведущий ноль если нечетная длина
    }
    // Преобразуем каждые два символа в байт
    for (size_t i = 0; i < cleanStr.length(); i += 2) {
        std::string byteStr = cleanStr.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
        result.push_back(byte);
    }
    return result;
}

Json_Handler::Json_Handler(Data_Base* Base) {
    data_base = Base;
    comPort = &Serial;
    Serial.begin(9600);
    delay(2000);
    Serial.println("=== ESP32 JSON Handler ===");
    Serial.println("Готов к приему JSON...");
}

void Json_Handler::apdate_Data_Base(JSONVar jsonDoc){
  data_base->clear_Base();
  String uidStr = (String)jsonDoc["uid"];
  std::string uidStdStr = uidStr.c_str();
  std::vector<uint8_t> uidBytes = hexStringToVector(uidStdStr);
  String nameStr = (String)jsonDoc["name"];
  std::string nameStdStr = nameStr.c_str();
  data_base->add_person_json(uidBytes, nameStdStr);
}

int Json_Handler::waitAndProcessJSON() {
  if (!comPort->available()){return 1;}
  String receivedData = comPort->readStringUntil('\n');
  receivedData.trim();
            
  Serial.print("Получены данные: ");
  Serial.println(receivedData);
  if (receivedData == "END_OF_SESSION"){
    return 2;
  }
  // Serial.print("Длина данных: ");
  // Serial.println(receivedData.length());
            
  if (receivedData.length() == 0) {
    Serial.println("Пустая строка");
    return 1;
  }
            
  // Парсим JSON
  JSONVar jsonDoc = JSON.parse(receivedData);
  String jsonType = JSON.typeof(jsonDoc);
  // Serial.print("Тип JSON: ");
  // Serial.println(jsonType);
            
  if (jsonType == "undefined") {
    Serial.println("Ошибка: Неверный JSON формат");
    return 1;
  }
            
  Serial.println("JSON таблица получена");

  apdate_Data_Base(jsonDoc);
            
  // Добавляем пометку
  jsonDoc["processed_by_esp32"] = true;
  jsonDoc["processed_at"] = (long)millis();
            
  // Отправляем обратно
  String output = JSON.stringify(jsonDoc);
  Serial.println(output);

  return 0;
}