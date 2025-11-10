#include "CSVHandler.h"

CSVHandler::CSVHandler() : _receiving(false) {}

bool CSVHandler::isReceiving() {
    return _receiving;
}

void CSVHandler::activateCSVMode() {
    startReceiving();
}

void CSVHandler::checkSerial() {
    if (!_receiving) return;
    
    // ЗАЩИТА ОТ ПЕРЕПОЛНЕНИЯ
    if (_content.length() > 10000) { // Максимум 10KB
        _receiving = false;
        Serial.println("BUFFER_OVERFLOW");
        _content = "";
        return;
    }
    
    if (Serial.available()) {
        String input = Serial.readString();

        
        if (input.indexOf("END_CSV_FILE") >= 0) {
            _receiving = false;
            Serial.println("CSV_FILE_RECEIVED");
            printContent();
            _content = "";
            Serial.println("CSV_FILE_DELETED");
        } else {
            _content += input;
        }
    }
}

void CSVHandler::startReceiving() {
    _content = "";
    _receiving = true;
    Serial.println("READY_FOR_CSV");
}

void CSVHandler::printContent() {
    Serial.println("=== CSV CONTENT ===");
    Serial.println(_content);
    Serial.println("===================");
}