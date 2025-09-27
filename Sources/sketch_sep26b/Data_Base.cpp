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

bool Data_Base::add_person(uint8_t person_UID){
  Person* person = new Person(person_UID);
  person_data[person_UID] = person;
  Serial.print("New person has been added\n");
  return true;
}

bool Data_Base::delete_person(uint8_t person_UID){
  auto it = person_data.find(person_UID);
  if (it != person_data.end()){
    delete it->second;
    person_data.erase(it);
    return true;
  }
  return false;
}

void Data_Base::print_persons_data(){
  for (auto it = person_data.begin(); it!=person_data.end(); it++){
    it->second->print_info();
    // Serial.print("\n");
  }
}

