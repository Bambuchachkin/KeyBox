#include <Arduino.h>
#include "Person.h"

Person::Person(uint8_t person_UID){
  UID = person_UID;
  Serial.print("Person::Person()\n");
}

Person::~Person(){
  Serial.print("Person::~Person()\n");
}

bool Person::add_key_access(int key_number){
  keys[key_number] = 1;
  Serial.print("Access was given\n");
  return true;
}

bool Person::remove_key_access(int key_number){
  auto it = keys.find(key_number);
  if (it != keys.end()) {
    it->second = 0;
    Serial.print("Access removed\n");
    return true;
  } else {
    Serial.print("Person didn`t have access to this key\n");
  }
  return false;
}

bool Person::check_key_access(int key_number){
  auto it = keys.find(key_number);
  if (it != keys.end()) {
    if(it->second == 1){
      Serial.print("Key is available\n");
      return true;
    }
  } else {
    Serial.print("Key isn`t available\n");
  }
  return false;
}

int Person::get_key_status(int key_number){
  auto it = keys.find(key_number);
  if (it != keys.end()) {
    if(it->second == 0){
      Serial.print("Key isn`t available\n");
      return 0;
    } else if((it->second == 1)) {
      Serial.print("Key is available\n");
      return 1;
    } else {
      Serial.print("Key is in hands\n");
    }
  }
  return 2;
}

bool Person::take_key(int key_number){
  if (check_key_access(key_number)){
    auto it = keys.find(key_number);
    it->second = 2;
    Serial.print("Key is taken\n");
    return true;
  } else {
    Serial.print("Key is taken\n");
  }
  return false;
}


bool Person::return_key(int key_number){
  if (check_key_access(key_number)){
    auto it = keys.find(key_number);
    it->second = 1;
    Serial.print("Key was returned\n");
    return true;
  } else {
    Serial.print("Key wasn`t returned\\n");
  }
  return false;
}

void Person::print_info(){
  Serial.print("Person UID: ");
  Serial.print(UID);
  Serial.print(" | Available keys: ");
  for (auto it = keys.begin(); it != keys.end(); it++){
    if (it->second == 1 || it->second == 2){
      Serial.print(it->first);
      Serial.print(" ");
    }
  }
  Serial.print(" | Keys in hands: ");
  for (auto it = keys.begin(); it!=keys.end(); it++){
    if (it->second == 2){
      Serial.print(it->first);
      Serial.print(" ");
    }
  }
  Serial.print("\n");
}

