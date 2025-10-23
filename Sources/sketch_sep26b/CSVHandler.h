#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <Arduino.h>

class CSVHandler {
private:
    String _content;
    bool _receiving;
    
public:
    CSVHandler();
    void checkSerial();
    void startReceiving();
    bool isReceiving();
    void activateCSVMode();  // Публичный метод для активации

private:
    void printContent();
};

#endif