#ifndef KEY_HANDLER_H
#define KEY_HANDLER_H

#include <map>
#include <string>
// #include <vector>
#include <Arduino_JSON.h>
#include <SPIFFS.h>
#include "Data_Base.h"

class Key_Handler {
  Data_Base* data_base;
  std::map<int, int> key_map; // 1 - на месте, 0 - отсутствует
  std::map<unsigned long, std::string[3]> key_notes; // информация по ключам, int - время операции (взят, возвращен, выдан доступ), string - uid, тип операции, std::string - номер ключа
  unsigned long process_time;
public:
  Key_Handler(Data_Base* Data_base);
  // ~Key_Handler();
  void create_note(std::string uid, std::string action, std::string key_number);

  bool save_to_spiffs();
  bool load_from_spiffs();

  // void clear_notes();
  void save_notes();

  bool send_notes_to_PC();

  bool take_key(std::vector<uint8_t> UID, int key_number);
  bool return_key(std::vector<uint8_t> UID, int key_number);
};

#endif