#ifndef TERMINAL_H
#define TERMINAL_H

#include <map>
#include <string>
#include <vector>

#include "Data_Base.h"

class Terminal {
  private:
    Data_Base data_base;
    std::vector<uint8_t> buffered_UID;
    // std::map<std::string, std::string> HELP;
  public:
    Terminal();
    ~Terminal();

    void process_add(std::string new_user);
    void process_info(std::string who);
    void process_check(std::string who);
    void process_delete(std::string who);
    void process_help();

    void process_command(std::vector<std::string> commands);
    void buffer_UID(std::vector<uint8_t>& new_UID);
    std::vector<std::string> read_command();
};

#endif