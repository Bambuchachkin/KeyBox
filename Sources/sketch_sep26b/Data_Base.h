#ifndef DATA_BASE_H
#define DATA_BASE_H

#include <map>
#include <string>
#include <SPIFFS.h>

#include "Person.h"
// #include "Key_Handler.h"

class Data_Base {
  private:
    std::map<std::vector<uint8_t>, Person*> person_data;
  public:
    Data_Base();
    ~Data_Base();

    auto find_person(std::vector<uint8_t> person_UID);
    auto find_person(std::string person_name);
    auto find_person(int p_number);

    Person* get_person(std::vector<uint8_t> person_UID);

    auto get_map_end();

    bool add_person(std::vector<uint8_t> person_UID);
    bool add_person_json(std::vector<uint8_t> person_UID, std::string person_name);
    bool delete_person(std::vector<uint8_t> person_UID);
    bool delete_person(int p_number);

    bool take_key(std::vector<uint8_t> person_UID, int key_number);
    bool return_key(std::vector<uint8_t> person_UID, int key_number);
    bool give_access(std::vector<uint8_t> person_UID, int key_number);
    bool remove_access(std::vector<uint8_t> person_UID, int key_number);
    int get_status(std::vector<uint8_t> person_UID, int key_number);
    bool give_access_by_number(int p_number, int key_number);
    bool remove_access_by_number(int p_number, int key_number);
    bool check_door_access(std::vector<uint8_t> person_UID);

    void print_persons_data();
    void print_persons_data(std::string p_name);

    bool save_to_spiffs();
    bool load_from_spiffs();

    void clear_Base();
    void save_Base();

    bool send_json_to_PC();
};

bool operator==(const std::vector<uint8_t>& lhs, const std::vector<uint8_t>& rhs);

#endif