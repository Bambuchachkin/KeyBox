#include <Adafruit_NeoPixel.h>

#define PIN 22
#define NUMPIXELS 30

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(255);  // Средняя яркость
  strip.clear();
  strip.show();
}

void loop() {
  for(int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(50, 50, 50));  // Белый
  }
  strip.show();  // Отправляем команду один раз
  delay(50);
  
  // Ничего не делаем - светодиоды остаются включенными
  // delay(1000);  // Можно добавить небольшую паузу, но не обязательно
}