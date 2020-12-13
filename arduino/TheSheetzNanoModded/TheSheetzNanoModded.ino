#include <Adafruit_NeoPixel.h>
#include "util_utils.h"
#include "notes.h"

#define MY_ID 0

#define LEDS_PER_CANDLE 2
//#define LEDS_PER_CANDLE 3

#define LED_PIN 6
//#define LED_PIN 8

//#define SENSOR_REVERSED true
#define SENSOR_REVERSED false

#define CANDLE_COUNT 8
#define SENSOR_TURN_ON_THRESHOLD 150
#define SENSOR_TURN_OFF_THRESHOLD 900
#define LIGHTUP_DURATION 1000
#define PUTOUT_DURATION 1000
#define STARTUP_MODE_SCAN_DURATION_SECS 1

#define MAX_INTENSITY 255
#define MIN_INTENSITY 0

#define REMOTE_TURN_ON_OFF_RATE 5
#define SCREENSAVER_ACTIVATION_DURATION_SECS 15
#define CANDLE_TURN_ON_OFF_SOUND

#define REMOTE_ENABLE_PIN 12
byte ANALOG_INPUT_PINS[] = {  A7, A6, A5, A4, A3, A2, A1, A0 };

#define SPEAKER_PIN 7
#include "song_utils.h"

#define SPECIAL_MODE_COUNT 3

#define MODE_HANUKIYA 0
#define MODE_SIMON 1
#define MODE_LIGHT_PAINTING 2
#define MODE_PONG 3

boolean remoteEnabled;
#include "comm_utils.h"

Adafruit_NeoPixel strip(CANDLE_COUNT * LEDS_PER_CANDLE, LED_PIN, NEO_GRB + NEO_KHZ800);
#include "simon.h"
#include "pong.h"
#include "lightpainting.h"

int master = -1;
int candles = 0;
int mode = MODE_HANUKIYA;
int param1 = 0;
String param2 = "";

long lightChangeTimestamp[CANDLE_COUNT] = { -1 };
boolean candleIsLit[CANDLE_COUNT] = {  false };
long ledColors[CANDLE_COUNT * LEDS_PER_CANDLE];
int candleStrength[CANDLE_COUNT] = { 0 };

boolean lastRemoteEnabledState;
int currentHanukkahSong = 0;
boolean masterInit = false;
long lastActivityTstamp = 0;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0) ^ analogRead(A1) ^ analogRead(A2) ^ analogRead(A3) ^ analogRead(A4) ^ analogRead(A5) ^ analogRead(A6) ^ analogRead(A7));
  commSetup();
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(REMOTE_ENABLE_PIN, INPUT_PULLUP);
  strip.begin();
  strip.show();
  strip.setBrightness(255);

  for (int i = 0; i < strip.numPixels(); i++) {
    ledColors[i] = strip.gamma32(strip.ColorHSV(65536L / strip.numPixels() * i));
  }
  lastRemoteEnabledState = !digitalRead(REMOTE_ENABLE_PIN);
  Serial.print("initial remote state: ");
  Serial.println(lastRemoteEnabledState ? "true" : "false");
  if (lastRemoteEnabledState) {
    rainbow(100);
  }
  else {
    colorSwipe(100, 0xff00ff); // purple
  }
  // scan for mode configuration on startup
  long modeScanStarted = millis();
  while (((millis() - modeScanStarted) / 1000) < STARTUP_MODE_SCAN_DURATION_SECS) {
    int lit = getLitCandle(SPECIAL_MODE_COUNT);
    if (lit > -1) {
      // no need to compansate for reverse sensor order, sensorBeingLit takes care of that
      mode = lit + 1;
      Serial.print("mode selected: "); Serial.println(mode);
      signalSpecialMode(mode);
    }
  }
}

void loop() {
  // for good order, master should reset Hanukiya state in the cloud upon startup.
  // but first it needs to know whether its the master or not, via network.
  // all at the same time, the local mode should be catered, if requested
  // hence this if.
  if (!masterInit && master > -1) {
    masterInit = true;
    if (isMaster()) {
      Serial.println("i AM master");
      onMasterInit();
    }
    else {
      Serial.println("i AM NOT master. maybe next time");
    }
  }
  switch (mode) {
    case MODE_HANUKIYA:
      handleComm();
      handleRemoteSwitch();
      handleCandles();
      handleScreensaver();
      break;
    case MODE_SIMON:
      handleSimon();
      break;
    case MODE_LIGHT_PAINTING:
      handleLightPainting();
      break;
    case MODE_PONG:
      handlePong();
      break;
  }
}

void checkCandleOn(int candleIndex) {
  if (sensorBeingLit(candleIndex)) {
    if (lightChangeTimestamp[candleIndex] > 0) {
      if ((millis() - lightChangeTimestamp[candleIndex]) > LIGHTUP_DURATION) {
        if (isMaster() || !remoteEnabled) {
          candleIsLit[candleIndex] = true;
          turnCandle(candleIndex, MAX_INTENSITY);
          lightChangeTimestamp[candleIndex] = -1;
          noSound();
          beep(2, 50, 50);
          lightChangeTimestamp[candleIndex] = -1;
          candles |= (1 << candleIndex);
          Serial.print("updating candles to: "); Serial.println(candles);
          if (isMaster()) {
            updateCandles(candles);
          }
          checkForFullHouse();
        }
      }
      else {
        int intensity = constrain(map((millis() - lightChangeTimestamp[candleIndex]), 0, LIGHTUP_DURATION, 0, 255), 0, 255);
        turnCandle(candleIndex, intensity);
        // fscale shenanigans for funkier turn off sound timing
        sound(
          440 + 5 * intensity,
          10,
          (int) fscale(0, 255, 40, 100, intensity, 2));
        lastActivityTstamp = millis();
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
  if (sensorBeingDarkened(candleIndex)) {
    if (lightChangeTimestamp[candleIndex] > 0) {
      if ((millis() - lightChangeTimestamp[candleIndex]) > PUTOUT_DURATION) {
        if (isMaster() || !remoteEnabled) {
          candleIsLit[candleIndex] = false;
          turnCandle(candleIndex, MIN_INTENSITY);
          lightChangeTimestamp[candleIndex] = -1;
          candles &= (0xff ^ (1 << candleIndex));
          if (isMaster()) {
            Serial.print("updating candles to: "); Serial.println(candles);
            updateCandles(candles);
          }
          noSound();
        }
      }
      else {
        int intensity = constrain(map((millis() - lightChangeTimestamp[candleIndex]), 0, PUTOUT_DURATION, 255, 0), 0, 255);
        turnCandle(candleIndex, intensity);
        // fscale shenanigans for funkier turn off sound timing
        sound(440 + 5 * intensity, 10, (int) fscale(0, 255, 40, 100, intensity, 2) + 20);
        lastActivityTstamp = millis();
      }
    } else {
      lightChangeTimestamp[candleIndex] = millis();
    }
  } else {
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

void colorSwipe(int d, unsigned long col) {
  for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, col);
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

void turnCandle(int candle, int intensity, long col) {
  candleStrength[candle] = intensity;
  int r = ((col & 0xff) * intensity) / 255;
  int g = (((col >> 8) & 0xff) * intensity) / 255;
  int b = (((col >> 16) & 0xff) * intensity) / 255;
  for (int ledInCandle = 0; ledInCandle < LEDS_PER_CANDLE; ++ledInCandle) {
    int ledIndex = candle * LEDS_PER_CANDLE + ledInCandle;
    col = strip.Color(r, g, b);
    strip.setPixelColor(ledIndex, col);
    strip.show();
  }
}

void turnCandle(int candle, int intensity) {
  candleStrength[candle] = intensity;
  for (int ledInCandle = 0; ledInCandle < LEDS_PER_CANDLE; ++ledInCandle) {
    int ledIndex = candle * LEDS_PER_CANDLE + ledInCandle;
    long col = ledColors[ledIndex];
    int r = ((col & 0xff) * intensity) / 255;
    int g = (((col >> 8) & 0xff) * intensity) / 255;
    int b = (((col >> 16) & 0xff) * intensity) / 255;
    col = strip.Color(r, g, b);
    strip.setPixelColor(ledIndex, col);
    strip.show();
  }
}

void onRemoteSwitchChanged(boolean newRemoteState) {
  Serial.print("new remote state: ");
  Serial.println(newRemoteState ? "true" : "false");
  resetCandles();
  if (newRemoteState) {
    rainbow(100);
  }
  else {
    colorSwipe(100, 0xff00ff ); // purple
    mode = 0; // no simon for you localer
  }
}

void handleRemoteSwitch() {
  remoteEnabled = !digitalRead(REMOTE_ENABLE_PIN);
  if (remoteEnabled != lastRemoteEnabledState) {
    lastRemoteEnabledState = remoteEnabled;
    onRemoteSwitchChanged(remoteEnabled);
  }
}

void handleCandles() {
  for (int candle = 0; candle < sizeof(ANALOG_INPUT_PINS); ++candle) {
    if (!candleIsLit[candle]) {
      checkCandleOn(candle);
    }
    else {
      checkCandleOff(candle);
    }
  }
}

boolean isMaster() {
  return (master == MY_ID);
}

void onNewMaster(int newMaster) {
  if (newMaster != master) {
    Serial.print("got new master:"); Serial.println(newMaster);
    if (isMaster()) {
      Serial.println("i AM master");
      onMasterInit();
    }
    else {
      Serial.println("i AM NOT master. maybe next time");
    }
  }
  else {
    Serial.println("master unchanged");
  }
  master = newMaster;
}

void onNewCandlesState(int newCandles) {
  if (!remoteEnabled) {
    Serial.print("in local mode, rejecting CANDLES update! SURA SERVER HAL'A SHCHOR!");
    return; // just so that you don't have to scroll down
  }
  else {
    if (!isMaster()) {
      Serial.print("got candles:"); Serial.println(candles);
      if (candles != newCandles) {
        for (int candle = 0; candle < CANDLE_COUNT; ++candle) {
          int mask = 1 << candle;
          if ((newCandles & mask) && (!(candles & mask))) { // light up current candle, per remote state
            Serial.print("lighting up candle # "); Serial.println(candle);
            for (int intensity = 0; intensity < MAX_INTENSITY; intensity += REMOTE_TURN_ON_OFF_RATE) {
              turnCandle(candle, intensity);
              sound(440 + 5 * intensity, 5, 10);
            }
            noSound();
            lightChangeTimestamp[candle] = -1;
            candleIsLit[candle] = true;
            candleStrength[candle] = MAX_INTENSITY;
          }
          if ((!(newCandles & mask)) && (candles & mask)) { // turn off current candle, per remote state
            Serial.print("turning off candle # "); Serial.println(candle);
            for (int intensity = MAX_INTENSITY; intensity >= 0; intensity -= REMOTE_TURN_ON_OFF_RATE) {
              turnCandle(candle, intensity);
              sound(440 + 5 * intensity, 10, 10);
            }
            turnCandle(candle, MIN_INTENSITY);
            noSound();
            lightChangeTimestamp[candle] = -1;
            candleIsLit[candle] = false;
            candleStrength[candle] = MIN_INTENSITY;
          }
        }
      }
      candles = newCandles;
      checkForFullHouse();
    }
    else {
      Serial.println("candles set request rejected on master. oh and did I mention I am MASTER?");
    }
  }
}

void onNewMode(int newMode) {
  if (!remoteEnabled) {
    Serial.print("in local mode, rejecting MODE update! SURA SERVER HAL'A SHCHOR!");
    return; // just so that you don't have to scroll down
  }
  else {
    if (!isMaster()) {
      if (mode != newMode) {
        if (mode == 0) {
          signalHanukiyaMode();
          resetCandles();
        }
        else {
          signalSpecialMode(newMode);
        }
      }
      mode = newMode;
      Serial.print("got mode:"); Serial.println(mode);
    }
    else {
      Serial.println("mode set request rejected on master. oh and did I mention I am MASTER?");
    }
  }
}

void onNewParam1(int newParam1) {
  if (!remoteEnabled) {
    Serial.print("in local mode, rejecting PARAM1 update! SURA SERVER HAL'A SHCHOR!");
    return; // just so that you don't have to scroll down
  }
  else {
    if (!isMaster()) {
      param1 = newParam1;
      Serial.print("got param1:"); Serial.println(param1);
    }
    else {
      Serial.println("param1 set request rejected on master. oh and did I mention I am MASTER?");
    }
  }
}

void onNewParam2(String newParam2) {
  if (!remoteEnabled) {
    Serial.print("in local mode, rejecting PARAM2 update! SURA SERVER HAL'A SHCHOR!");
    return; // just so that you don't have to scroll down
  }
  else {
    if (!isMaster()) {
      param2 = newParam2;
      Serial.print("got param2:"); Serial.print(param2); Serial.println();
    }
    else {
      Serial.println("param2 set request rejected on master. oh and did I mention I am MASTER?");
    }
  }
}

void resetCandles() {
  for (int i = 0; i < CANDLE_COUNT; ++i) {
    lightChangeTimestamp[i] = -1;
    candleIsLit[i] = false;
    candleStrength[i] = 0;
  }
  candles = 0;
}

void sound(int freq, int dur, int rest) {
#ifdef CANDLE_TURN_ON_OFF_SOUND
  tone(SPEAKER_PIN, freq, dur);
#endif
  if (rest) {
    delay(rest);
  }
}

void noSound() {
#ifdef CANDLE_TURN_ON_OFF_SOUND
  noTone(SPEAKER_PIN);
#endif
  delay(100);
}

boolean screensaverOn = false;
int currSSIntensity = 0;
int currSSIntensityDir = 1;

// TODO: add perlin noise to simulate cheerful fire
void handleScreensaver() {
//  Serial.println(lastActivityTstamp);
  if (screensaverOn) {
    if ((millis() - lastActivityTstamp) / 1000 < SCREENSAVER_ACTIVATION_DURATION_SECS) {
      screensaverOn = false;
      // retrieve led state from memory
      for (int i = 0; i < CANDLE_COUNT; ++i) {
        if (candleIsLit[i]) {
          turnCandle(i, MAX_INTENSITY);
        }
        else {
          turnCandle(i, MIN_INTENSITY);
        }
      }
    }
  }
  else {
    if ((millis() - lastActivityTstamp) / 1000 > SCREENSAVER_ACTIVATION_DURATION_SECS) {
      screensaverOn = true;
      // TODO: remember existing led state
      // nothing to do, already stored in candleIsLit[] array
    }
  }
  if (screensaverOn) {
    for (int i = 0; i < CANDLE_COUNT; ++i) {
      if (candleIsLit[i]) {
        turnCandle(i, currSSIntensity);
      }
    }
    currSSIntensity += currSSIntensityDir;
    if (currSSIntensity == MAX_INTENSITY || currSSIntensity == MIN_INTENSITY) {
      currSSIntensityDir *= -1;
    }
  }
}

boolean sensorBeingLit(int candleIndex) {
  int sensorValue = analogRead(SENSOR_REVERSED ? ANALOG_INPUT_PINS[CANDLE_COUNT - candleIndex - 1] : ANALOG_INPUT_PINS[candleIndex]);
  //  int sensorValue = analogRead(SENSOR_REVERSED ? ANALOG_INPUT_PINS[candleIndex] : ANALOG_INPUT_PINS[CANDLE_COUNT - candleIndex - 1]);
  return (sensorValue < SENSOR_TURN_ON_THRESHOLD);
}


boolean sensorBeingDarkened(int candleIndex) {
  int sensorValue = analogRead(SENSOR_REVERSED ? ANALOG_INPUT_PINS[CANDLE_COUNT - candleIndex - 1] : ANALOG_INPUT_PINS[candleIndex]);
  //  int sensorValue = analogRead(SENSOR_REVERSED ? ANALOG_INPUT_PINS[candleIndex] : ANALOG_INPUT_PINS[CANDLE_COUNT - candleIndex - 1]);
  return (sensorValue > SENSOR_TURN_OFF_THRESHOLD);
}

void checkForFullHouse() {
  if (candles == 0xff) {
    song(currentHanukkahSong, 0.4);
    currentHanukkahSong = (currentHanukkahSong + 1) % SONG_COUNT;
  }
}

void onMasterInit() {
  candles = 0;
  updateCandles(candles);
  delay(500); // be nice to firebase server
  // mode = 0;
  updateMode(mode);
  delay(500); // be nice to firebase server
  param1 = 0;
  updateParam1(param1);
  delay(500); // be nice to firebase server
  param2 = "";
  updateParam2(param2);
}

byte sirenSpeed[] = {20, 15, 10, 5, 4, 3, 2, 1, 1, 1, 1};

void signalSpecialMode(int m) {
  int candle = m - 1;
  for (int s = 0; s < sizeof(sirenSpeed); ++s) {
    for (int siren = 440; siren < 3520; siren += 10) {
      tone(SPEAKER_PIN, siren);
      delayMicroseconds(sirenSpeed[s] * 100);
    }
  }
  noTone(SPEAKER_PIN);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j <= candle; ++j) {
      turnCandle(j, MAX_INTENSITY);
    }
    delay(500);
    for (int j = 0; j <= candle; ++j) {
      turnCandle(j, MIN_INTENSITY);
    }
    delay(500);
  }
  delay(1000);
  switch (mode) {
    case MODE_SIMON:
      signalSimonMode();
      break;
    case MODE_PONG:
      break;
    case MODE_LIGHT_PAINTING:
      break;
  }
}

void signalHanukiyaMode() {
  if (lastRemoteEnabledState) {
    rainbow(100);
  }
  else {
    // probably not going to happen much, remote mode should
    // have been enabled to get into this part of code
    colorSwipe(100, 0xff00ff); // purple
  }
}

int getLitCandle(int maxCandle) {
  for (int candle = 0; candle < maxCandle; ++candle) {
    if (sensorBeingLit(candle)) {
      return candle;
    }
    delay(1); // for analog reading stability
  }
  return -1;
}
