#include <Tone32.h>
#include "autoconnect_utils.h"
#include "firebase_utils.h"

#define USE_LEDS

#define CANDLE_COUNT 8
#define LEDS_PER_CANDLE 1

#include "led_utils.h"

#define PRE_ANALOG_READ_DELAY 1

#define LED_DECAY_RATE 4
//#define TURN_ON_OFF_SOUND

#define SPEAKER_PIN 23
// byte ANALOG_INPUT_PINS[CANDLE_COUNT] = {36 /* SVP */, 39 /* SVN */, 34, 35, 32, 33, 25, 26};
// byte ANALOG_INPUT_PINS[CANDLE_COUNT] = {26, 25, 33, 32, 35, 34, 39 /* SVN */, 36 /* SVP */};
byte ANALOG_INPUT_PINS[CANDLE_COUNT] =    {2, 4, 33, 32, 35, 34, 39, 36};
//byte ANALOG_INPUT_PINS[CANDLE_COUNT] =    {36,39,34, 35, 32, 33, 25, 26};

#define SENSOR_TURN_ON_THRESHOLD 50
#define SENSOR_TURN_OFF_THRESHOLD 4000
#define LIGHTUP_DURATION 750
#define PUTOUT_DURATION 750

#define MAX_INTENSITY 255
#define MIN_INTENSITY 0

#define MY_ID 1
int master = -1;
boolean standalone = true;
int candlesState = 0;


long lightChangeTimestamp[CANDLE_COUNT] = { -1};
boolean candleIsLit[CANDLE_COUNT] = {false};
long ledColors[CANDLE_COUNT * LEDS_PER_CANDLE];
int candleStrength[CANDLE_COUNT] = {0};

void setup() {
  Serial.begin(115200);
  pinMode(SPEAKER_PIN, OUTPUT);

  delay(500);
#ifdef USE_LEDS
  stripBegin();
  stripShow();
  stripSetBrightness(255);

  for (int i = 0; i < strip.numPixels(); i++) {
    ledColors[i] = strip.gamma32(strip.ColorHSV(65536L / strip.numPixels() * i));
  }
  fadeInOut();
#endif
  delay(1000);
  autoconnectSetup();
  //  Serial.println();
  //  Serial.print("Connected with IP: ");
  //  Serial.println(WiFi.localIP());
  //  Serial.println();
  firebaseSetup();
#ifdef USE_LEDS
  rainbow(100);
#endif
}

void loop() {
  yield();
  
  for (int candle = 0; candle < sizeof(ANALOG_INPUT_PINS); ++candle) {
    yield();
    Serial.print(candleStrength[candle]); Serial.print("\t");
    if (!candleIsLit[candle]) {
      checkCandleOn(candle);
    }
    else {
      //      checkCandleOff(candle);
    }
  }
  Serial.println();
  delay(1);
}

void checkCandleOn(int candleIndex) {
  if(PRE_ANALOG_READ_DELAY) {
    delay(PRE_ANALOG_READ_DELAY);
  }
  int sensorValue = analogRead(ANALOG_INPUT_PINS[candleIndex]);
  //Serial.print(sensorValue); Serial.print("\t");
  if (sensorValue < SENSOR_TURN_ON_THRESHOLD) {
    if (lightChangeTimestamp[candleIndex] > 0) {
      if ((millis() - lightChangeTimestamp[candleIndex]) > LIGHTUP_DURATION) {
        //        if (standalone || isMaster()) {
        candleIsLit[candleIndex] = true;
        turnCandle(candleIndex, MAX_INTENSITY);
#ifdef TURN_ON_OFF_SOUND
        noTone(SPEAKER_PIN);
        delay(100);
#endif
        beep(2, 50, 50);
        lightChangeTimestamp[candleIndex] = -1;
        candlesState |= 1 << candleIndex;
        Serial.print("candleState="); Serial.println(candlesState);
        if (isMaster()) {
          firebasePersist(candlesState);
        }
      }
      else {
        int intensity = constrain(map((millis() - lightChangeTimestamp[candleIndex]), 0, LIGHTUP_DURATION, 0, 255), 0, 255);
        turnCandle(candleIndex, intensity);
#ifdef TURN_ON_OFF_SOUND
        tone(SPEAKER_PIN, 440 + 5 * intensity, 10, 0);
#endif
      }
    }
    else {
      lightChangeTimestamp[candleIndex] = millis();
    }
  }
  else {
    if (candleStrength[candleIndex] > 0) {
      turnCandle(candleIndex, constrain(candleStrength[candleIndex] - LED_DECAY_RATE, 0, 255));
    }
    lightChangeTimestamp[candleIndex] = -1;
  }
}

void checkCandleOff(int candleIndex) {
  delay(10);
  int sensorValue = analogRead(ANALOG_INPUT_PINS[candleIndex]);
  //Serial.print(sensorValue); Serial.print("\t");
  if (sensorValue > SENSOR_TURN_OFF_THRESHOLD) {
    if (lightChangeTimestamp[candleIndex] > 0) {
      if ((millis() - lightChangeTimestamp[candleIndex]) > PUTOUT_DURATION) {
        //        if (standalone || isMaster()) {
        candleIsLit[candleIndex] = false;
        turnCandle(candleIndex, MIN_INTENSITY);
        lightChangeTimestamp[candleIndex] = -1;
#ifdef TURN_ON_OFF_SOUND
        noTone(SPEAKER_PIN);
#endif
        //        }
      }
      else {
        int intensity = constrain(map((millis() - lightChangeTimestamp[candleIndex]), 0, PUTOUT_DURATION, 255, 0), 0, 255);
        turnCandle(candleIndex, intensity);
#ifdef TURN_ON_OFF_SOUND
        tone(SPEAKER_PIN, 440 + 5 * intensity, 10, 0);
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
    stripSetPixelColor(i, (ledColors[i]>>16) &0xff, (ledColors[i]>>8) &0xff, (ledColors[i]) &0xff);
    stripShow();
    delay(d);
  }
  for (int i = 0; i < strip.numPixels(); i++) {
    stripSetPixelColor(i, 0,0,0);
    stripShow();
    delay(d);
  }
  delay(1);
  stripShow();
}

void fadeInOut() {
  for (int b = 0; b < 255; ++b) {
    for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
      stripSetPixelColor(i, 0, b, 0);
    }
    stripShow();
    delay(2);
  }
  for (int b = 255; b >= 0; --b) {
    for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
      stripSetPixelColor(i, 0, b, 0);
    }
    stripShow();
    delay(2);
  }
}


void beep(int times, int onDuration, int offDuration) {
  for (int i = 0; i < times; ++i) {
    tone(SPEAKER_PIN, 1760, onDuration, 0);
    delay(onDuration + offDuration);
  }
}

void turnCandle(int candle, int intensity) {
#ifdef USE_LEDS
  candleStrength[candle] = intensity;
  for (int ledInCandle = 0; ledInCandle < LEDS_PER_CANDLE; ++ledInCandle) {
    int ledIndex = candle * LEDS_PER_CANDLE + ledInCandle;
    long col = ledColors[ledIndex];
    int r = ((ledColors[ledIndex] & 0xff) * intensity) / 255;
    int g = (((ledColors[ledIndex] >> 8) & 0xff) * intensity) / 255;
    int b = (((ledColors[ledIndex] >> 16) & 0xff) * intensity) / 255;
    col = strip.Color(r, g, b);
    strip.setPixelColor(ledIndex, col);
    stripShow();
  }
#endif  
}

void onNewMaster(int newMaster) {
  master = newMaster;
  Serial.print("master set to: "); Serial.println(master);
}

void onNewCandlesState(int newCandles) {
  if (isMaster()) {
    // ignore the local echo
  }
  else {
    updateCandles(newCandles);
  }
}

boolean isMaster() {
  return master == MY_ID;
}

void updateCandles(int newCandles) {
  Serial.print("candles set to: "); Serial.println(newCandles);

}
