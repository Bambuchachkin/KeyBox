#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include <map>
#include <string>
#include <vector>
#include <Arduino_JSON.h>

class Json_Handler {
private:
    HardwareSerial* comPort;
public:
    Json_Handler();
    int waitAndProcessJSON();
};

#endif