#ifndef DATA_BASE_H
#define DATA_BASE_H

#include <map>
#include <string>

#include "Person.h"

class Data_Base {
  private:
    std::map<uint8_t, Person*> person_data;
  public:
    Data_Base();
    ~Data_Base();

    bool add_person(uint8_t person_UID);
    bool delete_person(uint8_t person_UID);

    void print_persons_data();
};

#endif