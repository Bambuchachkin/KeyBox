#include "CSVHandler.h"
#include <Arduino.h>

CSVHandler::CSVHandler() : _receiving(false) {}

void CSVHandler::startReceiving() {
    _content = "";
    _receiving = true;
    Serial.println("READY_TO_RECEIVE");
}

void CSVHandler::processIncomingData() {
    if (!_receiving) return;
    
    if (Serial.available()) {
        String incoming = Serial.readString();
        
        if (incoming.indexOf("END_OF_FILE") >= 0) {
            _receiving = false;
            Serial.println("FILE_RECEIVED");
            printContent();
            _content = "";
            Serial.println("FILE_DELETED");
        } else {
            _content += incoming;
        }
    }
}

void CSVHandler::printContent() {
    Serial.println("=== CSV CONTENT ===");
    Serial.println(_content);
    Serial.println("===================");
}