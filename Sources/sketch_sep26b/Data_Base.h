#ifndef DATA_BASE_H
#define DATA_BASE_H

#include <map>
#include <string>
// #include <EEPROM.h>

#include "Person.h"

class Data_Base {
  private:
    std::map<std::vector<uint8_t>, Person*> person_data;
  public:
    Data_Base();
    ~Data_Base();

    auto find_person(std::vector<uint8_t> person_UID);
    auto find_person(std::string person_name);
    auto find_person(int p_number);

    auto get_map_end();

    bool add_person(std::vector<uint8_t> person_UID);
    bool delete_person(std::vector<uint8_t> person_UID);
    bool delete_person(int p_number);

    void print_persons_data();
    void print_persons_data(std::string p_name);
};

bool operator==(const std::vector<uint8_t>& lhs, const std::vector<uint8_t>& rhs);

#endif