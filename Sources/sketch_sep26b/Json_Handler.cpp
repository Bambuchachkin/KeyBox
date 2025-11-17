#include "Json_Handler.h"

#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>

std::vector<int> read_keys(std::string keys_str){
  std::vector<int> keys;
  std::string number_str;
  int number = -1;
  char new_char = 'a';
  for (int pos = 0; pos<keys_str.size(); pos++){
    new_char = keys_str[pos];
    if (new_char != ' '){
      number_str+=new_char;
    } else {
      number = std::stoi(number_str);
      keys.push_back(number);
      number_str.clear();
    }
  }
  number = std::stoi(number_str);
  keys.push_back(number);
  number_str.clear();
  return keys;
}

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

  Person* person = data_base->get_person(uidBytes);
  String AvailableKeysStr = (String)jsonDoc["available"];
  std::string AvailableKeysStdStr = AvailableKeysStr.c_str();
  std::vector<int> keys = read_keys(AvailableKeysStdStr);
  for (int i =0; i< keys.size(); i++){
    person->add_key_access(keys[i]);
    // data_base->save_Base();
  }

  String TakenKeysStr = (String)jsonDoc["on_hands"];
  std::string TakenKeysStdStr = TakenKeysStr.c_str();
  keys = read_keys(TakenKeysStdStr);
  for (int i =0; i< keys.size(); i++){
    person->take_key(keys[i]);
    // data_base->save_Base();
  }
  data_base->save_Base();
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
  if (receivedData.length() == 0) {
    Serial.println("Пустая строка");
    return 1;
  }
            
  // Парсим JSON
  JSONVar jsonDoc = JSON.parse(receivedData);
  String jsonType = JSON.typeof(jsonDoc);
            
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