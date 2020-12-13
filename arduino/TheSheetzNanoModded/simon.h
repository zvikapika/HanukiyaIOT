#define MAX_SIMON_TURNS 100
#define SIMON_PLAY_DURATION 400
#define SIMON_MIN_PLAY_DURATION 50

#define SIMON_STATE_ADD_NOTE 10
#define SIMON_STATE_PLAYBACK 20
#define SIMON_STATE_RECORD 30
#define SIMON_STATE_FAILED 40

#define SIMON_RECORD_TIMEOUT 1500

void turnCandle(int candle, int intensity);
void turnCandle(int candle, int intensity, long col);
int getLitCandle(int candles);

long simonRecordStartedTstamp = 0;
int simonNotes[CANDLE_COUNT] = { // major pentatonic scale
  NOTE_C4,
  NOTE_D4,
  NOTE_E4,
  NOTE_G4,
  NOTE_A4,
  NOTE_C5,
  NOTE_D5,
  NOTE_E5
};

byte simonSequence[MAX_SIMON_TURNS] = {0};
int simonState = SIMON_STATE_ADD_NOTE;
int simonIndex = 0;
int simonLevel = 0;
int simonRecordIndex = 0;

boolean simonCandleStates[CANDLE_COUNT] = {false};

long getSimonCandleColor(int candle) {
  return strip.gamma32(strip.ColorHSV(65536L / CANDLE_COUNT * candle));
}

float majScale[] = {
  14080,  15804,  17739,  18794,  21096,  23679,  26579,  28160,   31608,  35479,  37589,  42192,  47359,  53159,  56320
};

void simonCheesyEffect() {
  int ptime;
  int   k,  x, dur, freq, t;
  int i, j;
  float ps;         // variable for pow pitchShift routine
  float noteval;
  //note values

  for (i = 0; i <= 4; i++) {
    ps = (float)i / 12;         // choose new transpose interval every loop
    turnCandle(i, MAX_INTENSITY, getSimonCandleColor(i));
    turnCandle(i + 4, MAX_INTENSITY, getSimonCandleColor(i));
    for (x = 0; x <= 15; x++) {
      noteval = (majScale[x] / 64) * pow(2, ps);   // transpose scale up 12 tones - pow function generates transpostion
      dur = 50 - i * 8 - x;
      tone(SPEAKER_PIN, (int)noteval, dur);
      delay(dur * 1.1);
    }
    turnCandle(i, MIN_INTENSITY);
    turnCandle(i + 4, MIN_INTENSITY);
  }
  for (i = 0; i <= 6; i++) {
    ps = (float)i / 12;         // choose new transpose interval every loop
    turnCandle(i % 4, MAX_INTENSITY, getSimonCandleColor(i));
    turnCandle(i % 4 + 4, MAX_INTENSITY, getSimonCandleColor(i));
    for (x = 0; x <= 15; x++) {
      noteval = (majScale[x] / 64) * pow(2, ps);   // transpose scale up 12 tones - pow function generates transpostion
      dur = 5;
      tone(SPEAKER_PIN, (int)noteval, dur);
      delay(dur * 1.1);
    }
    turnCandle(i % 4, MIN_INTENSITY);
    turnCandle(i % 4 + 4, MIN_INTENSITY);
  }
  for (i = 0; i <= CANDLE_COUNT; i++) {
    turnCandle(i, MIN_INTENSITY);
  }
}

void simonFailureSound() {
  int i = 0;
  //  for(int t = 0; t < 3; ++t) {
  for (int i = 0; i < 4; ++i) {
    int j;
    for (j = 50; j > 0; j--) {
      if (j % 5 == 0) {
        int fsoundCandleIndex = j % CANDLE_COUNT;
        simonCandleStates[fsoundCandleIndex] = !simonCandleStates[fsoundCandleIndex];
        turnCandle(
          fsoundCandleIndex,
          simonCandleStates[fsoundCandleIndex] ? MIN_INTENSITY : MAX_INTENSITY,
          getSimonCandleColor(fsoundCandleIndex));
      }
      tone(SPEAKER_PIN, (5 - i) * 200 + j * 20);
      //        delayMicroseconds(500*(4-t));
      delay(7);
    }
  }
  for (int candle = 0; candle < CANDLE_COUNT; ++candle) {
    turnCandle(candle, MIN_INTENSITY);
  }
  noTone(SPEAKER_PIN);
}

void simonPlayNote(int index, boolean withDelay) {
  int candle = index;
  int delayDuration = constrain(SIMON_PLAY_DURATION - simonLevel * (SIMON_PLAY_DURATION / 10), SIMON_MIN_PLAY_DURATION, SIMON_PLAY_DURATION);
  turnCandle(candle, MAX_INTENSITY, getSimonCandleColor(candle));
  tone(SPEAKER_PIN, simonNotes[candle]);
  delay(delayDuration);
  turnCandle(candle, MIN_INTENSITY);
  noTone(SPEAKER_PIN);
  if (withDelay) {
    delay(delayDuration / 5);
  }
}

void simonPlayNote(int index) {
  simonPlayNote(index, true);
}

void signalSimonMode() {
  simonLevel = 5;
  for (int candle = 0; candle < CANDLE_COUNT; ++candle) {
    simonPlayNote(candle, false);
  }
  simonLevel = 0;
  delay(1000);
}

void resetSimon() {
  for (int i = 0; i < MAX_SIMON_TURNS; ++i) {
    simonSequence[i] = 0;
  }
  simonIndex = 0;
  simonLevel = 0;
  simonRecordStartedTstamp = 0;
  simonRecordIndex = 0;
}

void handleSimon() {
  switch (simonState) {
    case SIMON_STATE_ADD_NOTE:
      simonSequence[simonIndex++] = random(0, CANDLE_COUNT);
      if ((simonIndex % 5) == 0) {
        simonCheesyEffect();
        simonLevel++;
        delay(500);
      }
      simonState = SIMON_STATE_PLAYBACK;
      break;

    case SIMON_STATE_PLAYBACK:
      for (int i = 0; i < simonIndex; ++i) {
        simonPlayNote(simonSequence[i]);
      }
      simonState = SIMON_STATE_RECORD;
      simonRecordStartedTstamp = millis();
      simonRecordIndex = 0;
      break;

    case SIMON_STATE_RECORD:
      if ((millis() - simonRecordStartedTstamp) < SIMON_RECORD_TIMEOUT) {
        int lit = getLitCandle(CANDLE_COUNT);
        if (lit > -1) {
          if (lit == simonSequence[simonRecordIndex++]) {
            simonPlayNote(lit);
            if (simonRecordIndex == simonIndex) {
              simonState = SIMON_STATE_ADD_NOTE;
              delay(500);
            }
            else {
              simonRecordStartedTstamp = millis();
            }
          }
          else {
            simonState = SIMON_STATE_FAILED;
          }
        }
      }
      else {
        simonState = SIMON_STATE_FAILED;
      }
      break;

    case SIMON_STATE_FAILED:
      simonFailureSound();
      delay(500);
      // show correct sequence
      for (int i = 0; i < simonIndex; ++i) {
        simonPlayNote(simonSequence[i]);
      }
      // allow player remorse time
      delay(1000);
      // start again
      resetSimon();
      simonState = SIMON_STATE_ADD_NOTE;
      break;
  }
}
