#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include <map>
#include <string>
#include <vector>
#include <Arduino_JSON.h>

#include "Data_Base.h"
#include "Person.h"

class Json_Handler {
private:
    HardwareSerial* comPort;
    Data_Base* data_base;
public:
    Json_Handler(Data_Base* Base);
    void apdate_Data_Base(JSONVar jsonDoc);
    int waitAndProcessJSON();
};

#endif