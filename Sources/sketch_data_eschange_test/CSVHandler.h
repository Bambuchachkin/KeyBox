#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <Arduino.h>

class CSVHandler {
private:
    String _content;
    bool _receiving;
    
public:
    CSVHandler();
    void startReceiving();
    void processIncomingData();

private:
    void printContent();
};

#endif