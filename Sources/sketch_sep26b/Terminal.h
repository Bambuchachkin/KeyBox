#ifndef TERMINAL_H
#define TERMINAL_H

#include <map>
#include <string>
#include <vector>

#include "Data_Base.h"
#include "Json_Handler.h"
#include "Key_Handler.h"

class Terminal {
  private:
    Data_Base data_base;
    std::vector<uint8_t> buffered_UID;
    Json_Handler json_handler;
    Key_Handler key_handler;
    // std::map<std::string, std::string> HELP;
    std::string mode = "default";
  public:
    Terminal();
    ~Terminal();

    void process_add(std::string new_user);
    void process_info(std::vector<std::string> command);
    void process_check(std::string who);
    void process_delete(std::string who);
    void process_help();
    void process_JSON_TEST();
    void process_JSON_SEND();
    void process_notes_send();

    void process_command(std::vector<std::string> commands);
    void buffer_UID(std::vector<uint8_t>& new_UID);
    std::vector<std::string> read_command();

    // Добавляем методы для работы с сохранением
    bool load_data();
    bool save_data();
    bool load_notes();
};

#endif