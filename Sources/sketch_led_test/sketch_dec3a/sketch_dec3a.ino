#include <Adafruit_NeoPixel.h>

#define PIN  22
#define NUMPIXELS 30

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== ESP32 + 30 NeoPixel ===");
  Serial.println("Питание: 5V 1A через плату разработки");
  Serial.println("ВНИМАНИЕ: Яркость ограничена!");
  
  strip.begin();
  strip.setBrightness(20);  // МАКСИМУМ для вашей схемы!
  strip.clear();
  strip.show();
  
  // Быстрый тест всех светодиодов
  testSequence();
}

void testSequence() {
  Serial.println("Тест: Красный (тускло)");
  for(int i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(15, 0, 0));
  }
  strip.show();
  delay(1000);
  
  Serial.println("Тест: Зеленый (тускло)");
  for(int i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 15, 0));
  }
  strip.show();
  delay(1000);
  
  Serial.println("Тест: Синий (тускло)");
  for(int i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 15));
  }
  strip.show();
  delay(1000);
  
  strip.clear();
  strip.show();
  Serial.println("Тест завершен");
}

void loop() {
  // Режим 1: Бегущий огонь (экономит энергию)
  Serial.println("Режим: Бегущий огонь");
  runningDot(15, 0, 0, 50);  // Красный
  
  // Режим 2: Волна (включает постепенно)
  Serial.println("Режим: Волна");
  waveEffect(0, 15, 0, 100);  // Зеленый
  
  // Режим 3: Мерцание (редкие включения)
  Serial.println("Режим: Мерцание");
  sparkleEffect(0, 0, 15, 500);  // Синий
}

// Бегущая точка
void runningDot(int r, int g, int b, int delayTime) {
  for(int i=0; i<NUMPIXELS; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(r, g, b));
    strip.show();
    delay(delayTime);
  }
}

// Волновой эффект
void waveEffect(int r, int g, int b, int delayTime) {
  for(int i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
    strip.show();
    delay(delayTime);
  }
  for(int i=0; i<NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
    delay(delayTime);
  }
}

// Эффект мерцания
void sparkleEffect(int r, int g, int b, int delayTime) {
  for(int i=0; i<10; i++) {
    strip.clear();
    // Включаем случайные светодиоды
    for(int j=0; j<5; j++) {  // Только 5 одновременно!
      int pixel = random(NUMPIXELS);
      strip.setPixelColor(pixel, strip.Color(r, g, b));
    }
    strip.show();
    delay(delayTime);
  }
}