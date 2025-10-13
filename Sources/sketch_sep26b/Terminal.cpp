#include <iterator>
#include <Arduino.h>
#include "Terminal.h"

Terminal::Terminal(){
  Serial.print("Terminal::Terminal()\n");
}

Terminal::~Terminal(){
  Serial.print("Terminal::~Terminal()\n");
}

void Terminal::process_add(std::string new_user){
  if (!new_user.length()) return;
  if (new_user == "THIS"){
    if (buffered_UID.size()==0){
      Serial.print("There is not person\n");
      return;
    }
    data_base.add_person(buffered_UID);
  } else {
    // Serial.print(new_user.c_str());
    Serial.print("Try to use 'help'\n");
  }
}

void Terminal::process_info(std::string who){
  if (!who.length()) return;
  if (who == "ALL"){
    data_base.print_persons_data();
  } else {
    Serial.print("Try to use 'help'\n");
  }
}

// Функция для измерения уровня сигнала на аналоговом порту
void readAnalogSignal(int pin_number) {
  int sensorValue = analogRead(pin_number);        // Чтение аналогового значения (0-1023)
  float voltage = sensorValue * (3.3 / 4095.0);    // Конвертация в напряжение (для ESP32)
  Serial.print("Pin: ");
  Serial.print(pin_number);
  // Serial.print(" | Value: ");
  // Serial.print(sensorValue);
  Serial.print(" | V: ");
  Serial.print(voltage);
  Serial.print("     ");
}

void Terminal::process_check(std::string who){
  if (!who.length()) return;
  if (who == "buffer"){
    if (buffered_UID.size() ==0){
      Serial.print("Buffer is empty\n");
      return;
    }
    Serial.print("The last buffered UID: ");
    for (int i =0; i< buffered_UID.size(); i++){
      Serial.print(buffered_UID[i]);
      Serial.print(' ');
    }
    Serial.println();
    return;
  } if (who == "voltage"){
    char new_char = ' ';
    while (new_char != '\n'){
      new_char = Serial.read();
      readAnalogSignal(35); // перечисляем все порты с которых будем считывать сигнал
      readAnalogSignal(34);
      Serial.print("press Enter to finish V-checking\n");
      delay(100);
    }
      return;
  } else {
    Serial.print("Try to use 'help'\n");
  }
}

void Terminal::process_help(){
  Serial.print("Commands list:\n");
  Serial.print("'add [THIS]' too add new person\n");
  Serial.print("'delete [Ivanov Ivan Ivanovich]' too view persons data\n");
  Serial.print("'info [ALL]' too view persons data\n");
  Serial.print("'check [buffer; voltage]' too view sm\n");
}

void Terminal::process_delete(std::string who){
  Serial.print("Trying to delete person: ");
  Serial.print(who.c_str());
  Serial.print('\n');

  // auto it = data_base.find_person(who);
  // if (it != data_base.get_map_end()){
  //   data_base.delete_person(it->first);
  // }
  // data_base.delete_person(std::vector<uint8_t> person_UID)
}

void Terminal::process_command(std::vector<std::string> commands){
  Serial.print('\n');
  if (commands.size() == 1){
    if (commands[0] == "help"){
      process_help();
      return;
    }
    Serial.print("Try to use 'help'\n");
    return;
  }

  std::string command = commands[0];
  if (command == "add"){
    process_add(commands[1]);
    return;
  }
  if (command == "info"){
    process_info(commands[1]);
    return;
  }
  if (command == "check"){
    process_check(commands[1]);
    return;
  }
  if (command == "delete"){
    process_delete(commands[1]);
    return;
  } else {
    Serial.print("Unknown command\n");
  }
}

std::vector<std::string> Terminal::read_command(){
  std::vector<std::string> commands;
  if (Serial.available()) {
    std::string command = "";
    char new_char = Serial.read();
    while (new_char != '\n' && command.length()<64){
      if (new_char == ' '){
        if (!command.empty()){
          commands.push_back(command);
        }
        command = "";
      } else {
        command += new_char;
      }
      if (Serial.available()) {
        new_char = Serial.read();
      } else {
        delay(10);  // Небольшая задержка для прихода следующих символов
      }
    }
    if (!command.empty()) {
      commands.push_back(command);
    }
  }
  return commands;
}

void Terminal::buffer_UID(std::vector<uint8_t>& new_UID){
  buffered_UID.clear();
  for (int i =0; i< new_UID.size(); i++){
    buffered_UID.push_back(new_UID[i]);
  }
  Serial.print("Buffered UID: ");
  for (int i =0; i< buffered_UID.size(); i++){
    Serial.print(buffered_UID[i]);
    Serial.print(' ');
  }
  Serial.println();
}

