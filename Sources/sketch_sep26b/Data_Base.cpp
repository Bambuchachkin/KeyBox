#include <iterator>
#include <Arduino.h>
#include "Data_Base.h"
#include <Arduino_JSON.h>

std::string get_string(){
  std::string name = "";
  while (Serial.available() == 0) {
    delay(10);
  }
  while (true) {
    if (Serial.available() > 0) {
      char new_char = Serial.read();
      if (new_char == '\n' || new_char == '\r') {
        if (new_char == '\n') {
          break;
        }
      } else {
        name += new_char;
      }
      if (name.length() >= 128) {
        break;
      }
    }
    delay(1); // Небольшая задержка для стабильности
  }
  return name;
}

Data_Base::Data_Base(){
  Serial.print("Data_Base::Data_Base()\n");
}

Data_Base::~Data_Base(){
  for (auto it = person_data.begin(); it!=person_data.end(); it++){
    delete (it->second);
  }
  Serial.print("Data_Base::~Data_Base()\n");
}

bool operator==(const std::vector<uint8_t>& lhs, const std::vector<uint8_t>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    for (int i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i]) return false;
    }
    return true;
}

auto Data_Base::find_person(std::vector<uint8_t> person_UID){
  for (auto it = person_data.begin(); it!=person_data.end(); it++){
    if (it->first == person_UID){
      return it;
    }
  }
  return person_data.end();
}

auto Data_Base::find_person(std::string person_name){
  for (auto it = person_data.begin(); it!=person_data.end(); it++){
    if (it->second->get_name() == person_name){
      return it;
    }
  }
  return person_data.end();
}

auto Data_Base::find_person(int p_number){
  for (auto it = person_data.begin(); it!=person_data.end(); it++){
    if (it->second->get_person_number() == p_number){
      return it;
    }
  }
  return person_data.end();
}

Person* Data_Base::get_person(std::vector<uint8_t> person_UID){
  auto it = find_person(person_UID);
  return it->second;
}

auto Data_Base::get_map_end(){
  return person_data.end();
}

bool Data_Base::add_person(std::vector<uint8_t> person_UID){
  Serial.print("Trying to add person in Data_Base: ");
  for (int i =0; i< person_UID.size(); i++){
    Serial.print(person_UID[i]);
    Serial.print(' ');
  }
  Serial.print('\n');
  Person* person = new Person(person_UID);

  Serial.print("Enter the user`s full name\n");
  std::string p_name = get_string();
  person->rename(p_name);
  person_data[person_UID] = person;
  Serial.print("New person has been added\n");
  save_to_spiffs();
  return true;
}

bool Data_Base::add_person_json(std::vector<uint8_t> person_UID, std::string person_name){
  // load_from_spiffs();
  // Serial.print("Trying to add person in Data_Base: ");
  // for (int i =0; i< person_UID.size(); i++){
    // Serial.print(person_UID[i]);
    // Serial.print(' ');
  // }
  // Serial.print('\n');
  Person* person = new Person(person_UID);
  std::string p_name = person_name;
  person->rename(p_name);
  person_data[person_UID] = person;
  // Serial.print("New person has been added\n");
  // save_to_spiffs();
  return true;
}

bool Data_Base::delete_person(std::vector<uint8_t> person_UID){
  auto it = find_person(person_UID);
  if (it != person_data.end()){
    delete it->second;
    person_data.erase(it);
    save_to_spiffs();
    return true;
  }
  return false;
}

bool Data_Base::delete_person(int p_number){
  auto it = find_person(p_number);
  if (it != person_data.end()){
    delete it->second;
    person_data.erase(it);
    save_to_spiffs();
    return true;
  }
  return false;
}

bool Data_Base::take_key(std::vector<uint8_t> person_UID, int key_number){
  if (person_data.find(person_UID) == person_data.end()){ return false; }
  return person_data[person_UID]->take_key(key_number);
}
bool Data_Base::return_key(std::vector<uint8_t> person_UID, int key_number){
  if (person_data.find(person_UID) == person_data.end()){ return false; }
  return person_data[person_UID]->return_key(key_number);
}
bool Data_Base::give_access(std::vector<uint8_t> person_UID, int key_number){
  if (person_data.find(person_UID) == person_data.end()){ return false; }
  return person_data[person_UID]->add_key_access(key_number);
}
bool Data_Base::remove_access(std::vector<uint8_t> person_UID, int key_number){
  if (person_data.find(person_UID) == person_data.end()){ return false; }
  return person_data[person_UID]->remove_key_access(key_number);
}
int Data_Base::get_status(std::vector<uint8_t> person_UID, int key_number){
  if (person_data.find(person_UID) == person_data.end()){ return false; }
  return person_data[person_UID]->get_key_status(key_number);
}
bool Data_Base::give_access_by_number(int p_number, int key_number){
  if (find_person(p_number) == person_data.end()){ return false; }
  return find_person(p_number)->second->add_key_access(key_number);
}
bool Data_Base::remove_access_by_number(int p_number, int key_number){
  if (find_person(p_number) == person_data.end()){ return false; }
  return find_person(p_number)->second->remove_key_access(key_number);
}

void Data_Base::print_persons_data(){
  if (person_data.size() == 0){
    Serial.print("Data_Base is empty\n");
    return;
  }
  // int i = 1;
  for (auto it = person_data.begin(); it!=person_data.end(); it++){
    Serial.print(it->second->get_person_number());
    Serial.print(") ");
    it->second->print_info();
    // Serial.print("\n");
    // i++;
  }
}

// void Data_Base::print_persons_data(std::string p_name){
//   if (person_data.size() == 0){
//     Serial.print("Data_Base is empty\n");
//     return;
//   }
//   for (auto it = person_data.begin(); it!=person_data.end(); it++){
//     if (it->second->get_name() == p_name){ // точное совпадение
//       Serial.print(it->second->get_person_number());
//       Serial.print(") ");
//       it->second->print_info();
//     }
//     std::string = 
//   }
// }

void Data_Base::print_persons_data(std::string p_name){
  if (person_data.size() == 0){
    Serial.print("Data_Base is empty\n");
    return;
  }
  bool found = false;
  for (auto it = person_data.begin(); it != person_data.end(); it++){
    std::string person_name = it->second->get_name();
    // 1. Точное совпадение
    if (person_name == p_name) {
      Serial.print(it->second->get_person_number());
      Serial.print(") ");
      it->second->print_info();
      found = true;
      continue;
    }
    // 2. Простой поиск подстроки (фамилия/имя/отчество содержатся в ФИО)
    if (person_name.find(p_name) != std::string::npos) {
      Serial.print(it->second->get_person_number());
      Serial.print(") ");
      it->second->print_info();
      found = true;
    }
  }
  if (!found) {
    Serial.print("No persons found for: ");
    Serial.println(p_name.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// РЕАЛИЗАЦИЯ МЕТОДОВ SPIFFS ДЛЯ Arduino_JSON.h

bool Data_Base::save_to_spiffs() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Ошибка инициализации SPIFFS");
        return false;
    }
    
    File file = SPIFFS.open("/person_data.json", "w");
    if (!file) {
        Serial.println("Ошибка открытия файла для записи");
        SPIFFS.end();
        return false;
    }
    
    // Создаем JSON объект для Arduino_JSON
    JSONVar doc;
    JSONVar persons = JSONVar::parse("[]"); // Создаем пустой массив
    
    int index = 0;
    for (auto& pair : person_data) {
        JSONVar person;
        
        // Сохраняем UID как массив
        JSONVar uidArray;
        for (int i = 0; i < pair.first.size(); i++) {
            uidArray[i] = (int)pair.first[i];
        }
        person["uid"] = uidArray;
        
        // Сохраняем данные
        person["name"] = pair.second->get_name().c_str();
        person["number"] = pair.second->get_person_number();
        
        // Сохраняем ключи доступа
        JSONVar keysArray;
        int keyIndex = 0;
        // Получаем ключи из Person (нужно добавить метод доступа к keys в классе Person)
        // Предполагая, что у Person есть метод get_keys() который возвращает std::map<int, int>
        std::map<int, int> personKeys = pair.second->get_keys(); // Нужно добавить этот метод в Person
        
        for (auto& keyPair : personKeys) {
            JSONVar keyObj;
            keyObj["key_number"] = keyPair.first;
            keyObj["key_status"] = keyPair.second;
            keysArray[keyIndex] = keyObj;
            keyIndex++;
        }
        person["keys"] = keysArray;
        
        persons[index] = person;
        index++;
    }
    
    doc["persons"] = persons;
    
    // Сохраняем в файл
    String jsonString = JSON.stringify(doc);
    file.print(jsonString);
    file.close();
    SPIFFS.end();
    
    Serial.print("Данные сохранены в SPIFFS. Записей: ");
    Serial.println(person_data.size());
    return true;
}

bool Data_Base::load_from_spiffs() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Ошибка инициализации SPIFFS");
        return false;
    }
    if (!SPIFFS.exists("/person_data.json")) {
        Serial.println("Файл данных не найден");
        SPIFFS.end();
        return false;
    }
    File file = SPIFFS.open("/person_data.json", "r");
    if (!file) {
        Serial.println("Ошибка открытия файла для чтения");
        SPIFFS.end();
        return false;
    }
    String jsonString = file.readString();
    file.close();
    SPIFFS.end();
    
    // Парсим JSON
    JSONVar doc = JSON.parse(jsonString);
    if (JSON.typeof(doc) == "undefined") {
        Serial.println("Ошибка: Неверный JSON формат");
        return false;
    }
    
    // Очищаем текущие данные
    for (auto& pair : person_data) {
        delete pair.second;
    }
    person_data.clear();
    
    JSONVar persons = doc["persons"];
    // Получаем длину массива
    int personsLength = persons.length();
    for (int i = 0; i < personsLength; i++) {
        JSONVar person = persons[i];
        
        // Восстанавливаем UID
        JSONVar uidArray = person["uid"];
        std::vector<uint8_t> uid;
        int uidLength = uidArray.length();
        for (int j = 0; j < uidLength; j++) {
            uid.push_back((uint8_t)(int)uidArray[j]);
        }
        
        // Восстанавливаем данные
        const char* name = person["name"];
        int p_number = (int)person["number"];
        
        // Восстанавливаем персонажа
        Person* p = new Person(uid);
        p->rename(std::string(name));
        
        // Восстанавливаем ключи доступа
        if (person.hasOwnProperty("keys")) {
            JSONVar keysArray = person["keys"];
            int keysLength = keysArray.length();
            for (int k = 0; k < keysLength; k++) {
                JSONVar keyObj = keysArray[k];
                int key_number = (int)keyObj["key_number"];
                int key_status = (int)keyObj["key_status"];
                p->add_key_access(key_number); // Добавляем ключ
                // Устанавливаем статус ключа (нужно добавить метод в Person)
                p->set_key_status(key_number, key_status); // Нужно добавить этот метод в Person
            }
        }
        
        person_data[uid] = p;
    }
    
    Serial.print("Загружено записей из SPIFFS: ");
    Serial.println(personsLength);
    return true;
}

void Data_Base::clear_Base(){
  auto it = person_data.begin();
  it->second->set_P_NUMBER_TO_0();
  person_data.clear();
}

void Data_Base::save_Base(){
  save_to_spiffs();
}

// bool Data_Base::send_json_to_PC(){
//   save_Base();
//   // Создаем JSON объект для Arduino_JSON
//   JSONVar doc;
//   JSONVar persons = JSONVar::parse("[]"); // Создаем пустой массив
    
//   int index = 0;
//   for (auto& pair : person_data) {
//     JSONVar person;
//     JSONVar uidArray;
//     for (int i = 0; i < pair.first.size(); i++) {
//         uidArray[i] = (int)pair.first[i];
//     }
//     person["uid"] = uidArray;
//     // Сохраняем данные
//     person["name"] = pair.second->get_name().c_str();
//     person["number"] = pair.second->get_person_number();    
//     // Сохраняем ключи доступа
//     JSONVar keysArray;
//     int keyIndex = 0;
//     std::map<int, int> personKeys = pair.second->get_keys(); // Нужно добавить этот метод в Person
        
//     for (auto& keyPair : personKeys) {
//       JSONVar keyObj;
//       keyObj["key_number"] = keyPair.first;
//       keyObj["key_status"] = keyPair.second;
//       keysArray[keyIndex] = keyObj;
//       keyIndex++;
//     }
//     person["keys"] = keysArray;    
//     persons[index] = person;
//     index++;
//   }
//   doc["persons"] = persons; 

//   String output = JSON.stringify(doc);
//   Serial.println(output);
    
//   Serial.print("END_OF_JSON_SENDING");
//   return true;
// }
bool Data_Base::send_json_to_PC(){
  save_Base();
  JSONVar doc;
  JSONVar persons = JSONVar::parse("[]");
    
  int index = 0;
  for (auto& pair : person_data) {
    JSONVar person;
    JSONVar uidArray;
    
    // UID в hex формате
    for (int i = 0; i < pair.first.size(); i++) {
        uidArray[i] = (int)pair.first[i];
    }
    person["uid"] = uidArray;
    
    // Основные данные
    person["name"] = pair.second->get_name().c_str();
    person["number"] = pair.second->get_person_number();    
    
    // Ключи доступа - определяем статусы
    JSONVar availableKeys = JSONVar::parse("[]");
    JSONVar onHandsKeys = JSONVar::parse("[]");
    int availableIndex = 0;
    int onHandsIndex = 0;
    
    std::map<int, int> personKeys = pair.second->get_keys();
        
    for (auto& keyPair : personKeys) {
      if ((keyPair.second == 1) || (keyPair.second == 2)) { // Доступные ключи
        availableKeys[availableIndex] = keyPair.first;
        availableIndex++;
      }
      if ((keyPair.second == 2) || (keyPair.second == -2)) { // Ключи на руках
        onHandsKeys[onHandsIndex] = keyPair.first;
        onHandsIndex++;
      }
    }
    
    person["available_keys"] = availableKeys;
    person["on_hands_keys"] = onHandsKeys;
    
    persons[index] = person;
    index++;
  }
  doc["persons"] = persons; 

  String output = JSON.stringify(doc);
  Serial.println(output);
    
  Serial.print("END_OF_JSON_SENDING");
  return true;
}