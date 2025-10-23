#include <Arduino.h>
#include "CSVHandler.h"

CSVHandler csv;

void setup() {
    Serial.begin(115200);
    csv.startReceiving();
}

void loop() {
    csv.processIncomingData();
}