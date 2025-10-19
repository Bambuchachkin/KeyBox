#include <iterator>
#include <Arduino.h>
#include "Data_Base.h"

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
  return true;
}

bool Data_Base::delete_person(std::vector<uint8_t> person_UID){
  auto it = find_person(person_UID);
  if (it != person_data.end()){
    delete it->second;
    person_data.erase(it);
    return true;
  }
  return false;
}

bool Data_Base::delete_person(int p_number){
  auto it = find_person(p_number);
  if (it != person_data.end()){
    delete it->second;
    person_data.erase(it);
    return true;
  }
  return false;
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

