#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <map>
#include <vector>

class Person {
  private:
    uint8_t UID;
    std::map<int, int> keys;
  public:
    Person(uint8_t person_UID);
    ~Person();

    bool add_key_access(int key_number);
    bool remove_key_access(int key_number);

    bool check_key_access(int key_number);

    int get_key_status(int key_number); // 0 - inaccessible; 1 - in place; 2 - in users hands

    bool take_key(int key_number);
    bool return_key(int key_number);

    void print_info();
};

#endif