#include <iterator>
#include <Arduino.h>
#include "Data_Base.h"

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

void Data_Base::print_persons_data(){
  if (person_data.size() == 0){
    Serial.print("Data_Base is empty\n");
    return;
  }
  int i = 1;
  for (auto it = person_data.begin(); it!=person_data.end(); it++){
    Serial.print(i);
    Serial.print(") ");
    it->second->print_info();
    // Serial.print("\n");
    i++;
  }
}

