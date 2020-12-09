#include <Adafruit_NeoPixel.h>
#include "util_utils.h"

#define TURN_ON_OFF_SOUND

#define LED_PIN    6
#define SPEAKER_PIN 7

byte ANALOG_INPUT_PINS[] = {A0, A1, A2, A3, A4, A5, A6, A7 };

#define CANDLE_COUNT 8
#define LEDS_PER_CANDLE 1
#define SENSOR_TURN_ON_THRESHOLD 50
#define SENSOR_TURN_OFF_THRESHOLD 950
#define LIGHTUP_DURATION 1000
#define PUTOUT_DURATION 1000

#define MAX_INTENSITY 255
#define MIN_INTENSITY 0

Adafruit_NeoPixel strip(CANDLE_COUNT * LEDS_PER_CANDLE, LED_PIN, NEO_GRB + NEO_KHZ800);

long lightChangeTimestamp[CANDLE_COUNT] = { -1};
boolean candleIsLit[CANDLE_COUNT] = {false};
long ledColors[CANDLE_COUNT * LEDS_PER_CANDLE];
int candleStrength[CANDLE_COUNT] = {0};

void setup() {
  Serial.begin(115200);
  
  pinMode(SPEAKER_PIN, OUTPUT);
  strip.begin();
  strip.show();
  strip.setBrightness(255);
  for (int i = 0; i < strip.numPixels(); i++) {
    ledColors[i] = strip.gamma32(strip.ColorHSV(65536L / strip.numPixels() * i));
  }
  rainbow(100);
}

void loop() {
  for (int candle = 0; candle < sizeof(ANALOG_INPUT_PINS); ++candle) {
    if (!candleIsLit[candle]) {
      checkCandleOn(candle);
    }
    else {
      checkCandleOff(candle);
    }
  }
  delay(1);
}

void checkCandleOn(int candleIndex) {
  int sensorValue = analogRead(ANALOG_INPUT_PINS[candleIndex]);
  //Serial.print(sensorValue); Serial.print("\t");
  if (sensorValue < SENSOR_TURN_ON_THRESHOLD) {
    if (lightChangeTimestamp[candleIndex] > 0) {
      if ((millis() - lightChangeTimestamp[candleIndex]) > LIGHTUP_DURATION) {
        candleIsLit[candleIndex] = true;
        turnCandle(candleIndex, MAX_INTENSITY);
#ifdef TURN_ON_OFF_SOUND
        noTone(SPEAKER_PIN);
        delay(100);
#endif
        beep(2, 50, 50);
        lightChangeTimestamp[candleIndex] = -1;
      }
      else {
        int intensity = constrain(map((millis() - lightChangeTimestamp[candleIndex]), 0, LIGHTUP_DURATION, 0, 255), 0, 255);
        turnCandle(candleIndex, intensity);
#ifdef TURN_ON_OFF_SOUND
          tone(SPEAKER_PIN, 440 + 5 * intensity, 10);
          delay((int)fscale(0, 255, 40, 100, intensity, 2));
#endif
      }
    }
    else {
      lightChangeTimestamp[candleIndex] = millis();
    }
  }
  else {
    if (candleStrength[candleIndex] > 0) {
      turnCandle(candleIndex, candleStrength[candleIndex] - 1);
    }
    lightChangeTimestamp[candleIndex] = -1;
  }
}

void checkCandleOff(int candleIndex) {
  int sensorValue = analogRead(ANALOG_INPUT_PINS[candleIndex]);
  //Serial.print(sensorValue); Serial.print("\t");
  if (sensorValue > SENSOR_TURN_OFF_THRESHOLD) {
    if (lightChangeTimestamp[candleIndex] > 0) {
      if ((millis() - lightChangeTimestamp[candleIndex]) > PUTOUT_DURATION) {
        candleIsLit[candleIndex] = false;
        turnCandle(candleIndex, MIN_INTENSITY);
        lightChangeTimestamp[candleIndex] = -1;
#ifdef TURN_ON_OFF_SOUND
        noTone(SPEAKER_PIN);
#endif
      }
      else {
        int intensity = constrain(map((millis() - lightChangeTimestamp[candleIndex]), 0, PUTOUT_DURATION, 255, 0), 0, 255);
        turnCandle(candleIndex, intensity);
#ifdef TURN_ON_OFF_SOUND
          tone(SPEAKER_PIN, 440 + 5 * intensity, 10);
          delay((int)fscale(0, 255, 40, 100, intensity, 5));
          delay(20);
#endif
      }
    }
    else {
      lightChangeTimestamp[candleIndex] = millis();
    }
  }
  else {
    lightChangeTimestamp[candleIndex] = -1;
  }
}

void rainbow(int d) {
  for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, ledColors[i]);
    strip.show();
    delay(d);
  }
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
    strip.show();
    delay(d);
  }
}

void beep(int times, int onDuration, int offDuration) {
  for (int i = 0; i < times; ++i) {
    tone(SPEAKER_PIN, 1760, onDuration);
    delay(onDuration + offDuration);
  }
}

void turnCandle(int candle, int intensity) {
  candleStrength[candle] = intensity;
  for (int ledInCandle = 0; ledInCandle < LEDS_PER_CANDLE; ++ledInCandle) {
    int ledIndex = candle * LEDS_PER_CANDLE + ledInCandle;
    long col = ledColors[ledIndex];
    int r = ((ledColors[ledIndex] & 0xff) * intensity) / 255;
    int g = (((ledColors[ledIndex] >> 8) & 0xff) * intensity) / 255;
    int b = (((ledColors[ledIndex] >> 16) & 0xff) * intensity) / 255;
    col = strip.Color(r, g, b);
    strip.setPixelColor(ledIndex, col);
    strip.show();
  }
}
