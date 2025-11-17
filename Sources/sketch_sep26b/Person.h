#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <map>
#include <vector>
#include <string>

class Person {
  private:
    std::vector<uint8_t> UID;
    std::string name = "none";
    int p_number;
    static int P_NUMBER;
    std::map<int, int> keys;
  public:
    Person(std::vector<uint8_t> person_UID);
    ~Person();

    void rename(std::string new_name);
    std::string get_name();
    int get_person_number();

    bool add_key_access(int key_number);
    bool remove_key_access(int key_number);

    bool check_key_access(int key_number);

    int get_key_status(int key_number); // 0 - inaccessible; 1 - in place; 2 - in users hands

    bool take_key(int key_number);
    bool return_key(int key_number);

    void print_info();

    static int get_global_person_count();

    std::map<int, int> get_keys() const;
    void set_key_status(int key_number, int status);
    // static void set_global_person_count(int count);
};

#endif