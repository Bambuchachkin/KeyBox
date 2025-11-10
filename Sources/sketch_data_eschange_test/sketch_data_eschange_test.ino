#include <Arduino.h>
#include <Arduino_JSON.h>

class SimpleJSONHandler {
private:
    HardwareSerial* comPort;
    
public:
    SimpleJSONHandler(int portNumber = 0) {
        comPort = &Serial;
        Serial.begin(115200);
        delay(2000);
        Serial.println("=== ESP32 JSON Handler ===");
        Serial.println("Готов к приему JSON...");
    }
    
    void waitAndProcessJSON() {
        if (comPort->available()) {
            String receivedData = comPort->readStringUntil('\n');
            receivedData.trim();
            
            Serial.print("Получены данные: ");
            Serial.println(receivedData);
            Serial.print("Длина данных: ");
            Serial.println(receivedData.length());
            
            if (receivedData.length() == 0) {
                Serial.println("Пустая строка");
                return;
            }
            
            // Парсим JSON
            JSONVar jsonDoc = JSON.parse(receivedData);
            String jsonType = JSON.typeof(jsonDoc);
            Serial.print("Тип JSON: ");
            Serial.println(jsonType);
            
            if (jsonType == "undefined") {
                Serial.println("Ошибка: Неверный JSON формат");
                return;
            }
            
            Serial.println("JSON таблица получена");
            
            // Добавляем пометку
            jsonDoc["processed_by_esp32"] = true;
            jsonDoc["processed_at"] = (long)millis();
            
            // Отправляем обратно
            String output = JSON.stringify(jsonDoc);
            Serial.println(output);
        }
    }
};

SimpleJSONHandler jsonHandler(0);

void setup() {}

void loop() {
    jsonHandler.waitAndProcessJSON();
    delay(50);
}