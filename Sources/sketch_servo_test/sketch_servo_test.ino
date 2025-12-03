#include <ESP32Servo.h>

// Создаем объект сервопривода
Servo myServo;

// Пин для подключения сигнального провода сервопривода
#define SERVO_PIN 22

void setup() {
  // Привязываем сервопривод к пину
  myServo.attach(SERVO_PIN);
  
  // Необязательно: открываем последовательный порт для отладки
  Serial.begin(115200);
  Serial.println("Сервопривод инициализирован на пине 25");
}

void loop() {
  // Подаем сигнал для положения 0 градусов
  myServo.write(0);
  Serial.println("Положение: 0°");
  delay(1000);
  
  // Подаем сигнал для положения 90 градусов
  myServo.write(90);
  Serial.println("Положение: 90°");
  delay(1000);
  
  // Подаем сигнал для положения 180 градусов
  myServo.write(180);
  Serial.println("Положение: 180°");
  delay(1000);
  
  // Плавное движение от 0 до 180 градусов
  for(int pos = 0; pos <= 180; pos += 1) {
    myServo.write(pos);
    delay(15);
  }
  Serial.println("Плавное движение завершено");
  delay(1000);
}