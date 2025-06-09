#include <FastLED.h>
#include <avr/sleep.h>

#define NUM_LEDS 88 // Количество светодиодов в ленте
#define DATA_PIN 2
#define BUTTON_PIN 3 // Пин для кнопки

// Параметры для эффекта огня
#define COOLING 55 // Охлаждение пламени
#define SPARKING 120 // Вероятность появления искр

int battery_voltage; // Напряжение батареи
int mode = 4; // Переменная для хранения текущего режима
bool buttonPressed = false; // Флаг нажатия кнопки
unsigned long lastDebounceTime = 0; // Время последнего срабатывания кнопки
const unsigned long debounceDelay = 50; // Задержка для антидребезга

CRGB leds[NUM_LEDS]; // Массив для управления светодиодами

void setup() { 
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS); // Инициализация ленты
  FastLED.setBrightness(255); // Установка максимальной яркости
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Настройка пина кнопки как вход с подтяжкой
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Настройка режима сна

}

// Функция для плавного уменьшения яркости всех светодиодов
void fadeall() { 
  for(int i = 0; i < NUM_LEDS; i++) { 
    leds[i].nscale8(250); 
  } 
}

// Функция для мигания светодиодов красным цветом (при разряде)
void blink() { 
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red; // Установка i-го светодиода в красный цвет
    FastLED.show(); // Обновление ленты
  }
  delay(500);

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black; // Выключение i-го светодиода
    FastLED.show();
  }
  delay(500);

  battery_voltage = readVcc(); // Чтение напряжения батареи
  if (battery_voltage < 2900) { // Если напряжение ниже 2.9V
    sleep_mode(); // Переход в режим сна
  }
  blink();
}

// Функция для чтения напряжения батареи
long readVcc() {
  long result;
    // Настройка ADC для чтения внутреннего опорного напряжения 1.1V
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(2); // Ожидание стабилизации напряжения
    ADCSRA |= _BV(ADSC); // Запуск преобразования
    while (bit_is_set(ADCSRA, ADSC)); // Ожидание завершения преобразования
    result = ADCL;
    result |= ADCH << 8;
    result = 1126400L / result; // Пересчёт результата в милливольты

  return result;
}



// Режим 0: Изначальный режим (движение светодиодов в одном направлении и обратно)
void originalMode() {
  static uint8_t hue = 0; // Переменная для хранения оттенка цвета

  // Движение светодиодов в одном направлении
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue++, 255, 255); // Установка i-го светодиода в цвет с оттенком hue
    FastLED.show(); // Обновление ленты
    fadeall(); // Плавное уменьшение яркости
    delay(10);
  }

  // Движение светодиодов в обратном направлении
  for(int i = NUM_LEDS - 1; i >= 0; i--) {
    leds[i] = CHSV(hue++, 255, 255); // Установка i-го светодиода в цвет с оттенком hue
    FastLED.show();
    fadeall(); 
    delay(10);
  }
}

// Режим 1: Огонь (горение с двух концов ленты)
void fire() {
  static uint8_t heat[NUM_LEDS / 2]; // Массив для хранения "температуры" пламени

  // Охлаждение каждого пикселя
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / (NUM_LEDS / 2)) + 2));
  }

  // Распространение тепла
  for (int k = (NUM_LEDS / 2) - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Добавление новых искр
  if (random8() < SPARKING) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Отображение пламени с двух концов ленты
  for (int j = 0; j < NUM_LEDS / 2; j++) {
    CRGB color = HeatColor(heat[j]);
    leds[j] = color;
    leds[NUM_LEDS - 1 - j] = color;
  }

  FastLED.show();
  FastLED.delay(20);
}

// Режим 2: Радуга
void rainbow() {
  static uint8_t hue = 0;
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + i * 5, 255, 255); // Радужный эффект
  }
  FastLED.show();
  hue++;
  delay(10);
}

// Режим 3: Плавное изменение цвета
void colorFade() {
  static uint8_t hue = 0;
  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255)); // Плавное изменение цвета
  FastLED.show();
  hue++;
  delay(20);
}

// Режим 4: Красный цвет
void redColor() {
  fill_solid(leds, NUM_LEDS, CRGB::Red); // Заполнение ленты красным цветом
  FastLED.show();
}

// Режим 5: Зелёный цвет
void greenColor() {
  fill_solid(leds, NUM_LEDS, CRGB::Green); // Заполнение ленты зелёным цветом
  FastLED.show();
}

// Режим "Кометы" 
void comets() {
  const int numComets = 2; // Количество комет
  const int cometSize = 12 ; // Длина хвоста кометы
  static uint8_t hue = 0; // Переменная для хранения оттенка цвета
  static int cometPositions[numComets]; // Позиции комет
  static uint8_t cometHues[numComets]; // Цвета комет

  // Инициализация позиций и цветов комет при первом запуске
  static bool firstRun = true;
  if (firstRun) {
    for (int i = 0; i < numComets; i++) {
      cometPositions[i] = i * (NUM_LEDS / numComets); // Равномерное распределение комет
      cometHues[i] = random(255); // Случайный начальный цвет
    }
    firstRun = false;
  }

  // Очистка ленты (вся лента тёмная)
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Отрисовка комет с хвостами
  for (int i = 0; i < numComets; i++) {
    // Двигаем комету
    cometPositions[i] = (cometPositions[i] + 1) % NUM_LEDS;

    // Рисуем комету с хвостом
    for (int j = 0; j < cometSize; j++) {
      int pos = (cometPositions[i] - j + NUM_LEDS) % NUM_LEDS; // Позиция с учётом хвоста
      uint8_t brightness = 255 * (cometSize - j) / cometSize; // Явное затухание яркости хвоста
      leds[pos] = CHSV(cometHues[i], 255, brightness); // Цвет с затуханием
    }

    // Меняем цвет кометы
    cometHues[i] += 5;
  }

  FastLED.show();
  delay(50); // Задержка для скорости движения
}

void loop() { 
  // Чтение состояния кнопки с антидребезгом
  int reading = digitalRead(BUTTON_PIN);
  if (reading == LOW) { // Если кнопка нажата (LOW, так как подключена к GND)
    if (!buttonPressed && (millis() - lastDebounceTime) > debounceDelay) {
      buttonPressed = true;
      lastDebounceTime = millis();
      mode = (mode + 1) % 7; // Переключение режима (теперь 6 режимов)
    }
  } else {
    buttonPressed = false;
  }

  // Выбор режима в зависимости от значения переменной mode
  switch (mode) {
    case 0:
      originalMode(); // гекса
      break;
    case 1:
      fire(); // Огонь с двух концов ленты
      break;
    case 2:
      rainbow(); // Радуга
      break;
    case 3:
      colorFade(); // Плавное изменение цвета
      break;
    case 4:
      comets(); // Кометы
      break;
    case 5:
      redColor(); // Красный цвет
      break;
    case 6:
      greenColor(); // Зелёный цвет
      break;
  }

  // Проверка напряжения батареи
  battery_voltage = readVcc();
  if (battery_voltage < 3000) { // Если напряжение ниже 3.0V
    battery_voltage = readVcc(); // Повторное чтение для исключения случайных значений
    if (battery_voltage < 3000) { // Если напряжение всё ещё ниже 3.0V
      blink();
    }
  }
}
