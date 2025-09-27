#include <iterator>
#include <Arduino.h>
#include "Terminal.h"

Terminal::Terminal(){
  Serial.print("Terminal::Terminal()\n");
}

Terminal::~Terminal(){
  Serial.print("Terminal::~Terminal()\n");
}

void Terminal::process_command(std::string command){
  Serial.print("ASHALET\n");
  Serial.print(command.c_str());
  Serial.print("\n");
}

std::string Terminal::read_command(){
  if (Serial.available()) {
    std::string command = "";
    char new_char = Serial.read();
    while (new_char != '\n' && command.length()<19){
      command += new_char;
      if (Serial.available()) {
        new_char = Serial.read();
      } else {
        delay(10);  // Небольшая задержка для прихода следующих символов
      }
    }
    return command;
  }
  return "";
}
