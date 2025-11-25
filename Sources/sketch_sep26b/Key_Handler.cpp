#include <string>
#include <iterator>
#include <Arduino.h>
#include <Arduino_JSON.h>

#include "Key_Handler.h"

Key_Handler::Key_Handler(Data_Base* Data_base){
  data_base = Data_base;
  for (int i = 1; i<11; i++){
    key_map[i] = 0; // считаем что ключей нет
    sum_signal_map[i] = 0;
  }

  pin_map[1] = 34;
  pin_map[2] = 35;
  pin_map[3] = 32;
  pin_map[4] = 33;
  pin_map[5] = 25;
  pin_map[6] = 26;
  pin_map[7] = 27;
  pin_map[8] = 14;
  pin_map[9] = 12;
  pin_map[10] = 13;
  buffered_UID = {0};
}

void Key_Handler::create_note(std::string uid, std::string action, std::string k_number){
  // Serial.print("create_note_START\n");
  process_time = (unsigned long)millis();
  key_notes[process_time][0] = uid;
  key_notes[process_time][1] = action;
  key_notes[process_time][2] = k_number;
  // Serial.print("create_note_END\n");
}


bool Key_Handler::save_to_spiffs() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Ошибка инициализации SPIFFS");
        return false;
    }
    File file = SPIFFS.open("/keys_notes.json", "w");
    if (!file) {
        Serial.println("Ошибка открытия файла для записи");
        SPIFFS.end();
        return false;
    }
    // Создаем JSON объект для Arduino_JSON
    JSONVar doc;
    JSONVar notes = JSONVar::parse("[]"); // Создаем пустой массив
    
    for (auto it = key_notes.begin(); it!=key_notes.end(); it++){
      JSONVar note;
      note["time"] = std::to_string(it->first).c_str();
      note["uid"] = it->second[0].c_str();
      note["action"] = it->second[1].c_str();
      note["key_number"] = it->second[2].c_str();
      notes[notes.length()] = note;
    }
    doc ["notes"] = notes;
    
    // Сохраняем в файл
    String jsonString = JSON.stringify(doc);
    file.print(jsonString);
    file.close();
    SPIFFS.end();
    
    Serial.print("Данные сохранены в SPIFFS. Записей: ");
    // Serial.println(person_data.size());
    return true;
}

bool Key_Handler::load_from_spiffs() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Ошибка инициализации SPIFFS");
        return false;
    }
    if (!SPIFFS.exists("/keys_notes.json")) {
        // Serial.println("Файл данных не найден");
        SPIFFS.end();
        return false;
    }
    File file = SPIFFS.open("/keys_notes.json", "r");
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
    key_notes.clear();
    
    JSONVar notes = doc["notes"];
    int notes_length = notes.length();
    unsigned long time;
    for (int i = 0; i < notes_length; i++) {
        JSONVar note = notes[i];
        String time_str = (const char*)note["time"];
        time = std::stoul(time_str.c_str());
        key_notes[time][0] = (const char*)note["uid"];
        key_notes[time][1] = (const char*)note["action"];
        key_notes[time][2] = (const char*)note["key_number"];
        // key_notes[time][0] = note["uid"];
        // key_notes[time][1] = note["action"];
    }
    Serial.print("Загружено записей из SPIFFS: ");
    // Serial.println(personsLength);
    return true;
}

void Key_Handler::save_notes(){
  save_to_spiffs();
}

bool Key_Handler::send_notes_to_PC(){
  save_notes();
    JSONVar doc;
    JSONVar notes = JSONVar::parse("[]"); // Создаем пустой массив
    
    for (auto it = key_notes.begin(); it!=key_notes.end(); it++){
      JSONVar note;
      note["time"] = std::to_string(it->first).c_str();
      note["uid"] = it->second[0].c_str();
      note["action"] = it->second[1].c_str();
      note["key_number"] = it->second[2].c_str();
      notes[notes.length()] = note;
    }
    doc ["notes"] = notes;
    
    // Сохраняем в файл
    String jsonString = JSON.stringify(doc);

  String output = JSON.stringify(doc);
  Serial.println(output);
    
  Serial.print("END_OF_NOTES_SENDING");
  return true;
}

bool Key_Handler::take_key(std::vector<uint8_t> UID, int key_number){
  if (key_map[key_number] == 0){
    return false;
  }
  if (!data_base->take_key(UID, key_number)){
    return false;
  }
  key_map[key_number] = 0;
  std::string uidStr(UID.begin(), UID.end());
  if (data_base->get_status(UID, key_number)==-2){
    create_note(uidStr, "STEAL", std::to_string(key_number));
  } else{
    create_note(uidStr, "take", std::to_string(key_number));
  }
  return true;
}

bool Key_Handler::return_key(std::vector<uint8_t> UID, int key_number){
  if (key_map[key_number] == 1){
    return false;
  }
  if (!data_base->return_key(UID, key_number)){
    return false;
  }
  key_map[key_number] = 1;
  std::string uidStr(UID.begin(), UID.end());
  create_note(uidStr, "return", std::to_string(key_number));
  return true;
}

float Key_Handler::read_key_signal(int key_number) {
  int sensorValue = analogRead(pin_map[key_number]);        // Чтение аналогового значения (0-1023)
  return( sensorValue * (3.3 / 4095.0) );
}

void Key_Handler::check_keys(int check_time, int step){
  for (int i = 0; i<check_time; i+=step){
    for (int i = 1 ; i<11; i++){
      sum_signal_map[i]+=read_key_signal(i);
    }
    delay(step);
  }
  for (int i = 1; i<6; i++){
    if(sum_signal_map[i] >= 3.3*(check_time/step)*0.8){
      if (key_map[i] == 0){
        Serial.print(i);
        Serial.print(") key_returning...\n");
        return_key(buffered_UID, i);
      }
      key_map[i] = 1;
    } else {
      if (key_map[i] == 1){
        Serial.print(i);
        Serial.print(") key_taking...\n");
        take_key(buffered_UID, i);
      }
      key_map[i] = 0;
    }
  }
  for (int i = 6; i<11; i++){
    if(sum_signal_map[i] <= 3.3*(check_time/step)*0.2){
      if (key_map[i] == 0){
        Serial.print(i);
        Serial.print(") key_returning...\n");
        return_key(buffered_UID, i);
      }
      key_map[i] = 1;
    } else {
      if (key_map[i] == 1){
        Serial.print(i);
        Serial.print(") key_taking...\n");
        take_key(buffered_UID, i);
      }
      key_map[i] = 0;
    }
  }
  for (int i = 1; i<11; i++){
    sum_signal_map[i] = 0;
  }
}

void Key_Handler::print_keys_status(){
  for (int i = 1; i<11; i++){
    Serial.print(i);
    Serial.print(") ");
    Serial.print(key_map[i]);
    Serial.print(" | ");
  }
  Serial.print("\n");
}

void Key_Handler::set_current_UID(std::vector<uint8_t> UID){
  buffered_UID = UID;
}
