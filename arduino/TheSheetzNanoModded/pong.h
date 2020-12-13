#define PONG_STATE_WAITING_FIRST_STRIKE 10
#define PONG_STATE_BALL_MIDAIR 20
#define PONG_STATE_AWAIT_REPLY_STRIKE 30
#define PONG_STATE_FAILURE 40
//#define PONG_STATE_ 20
//#define PONG_STATE_ 20
//#define PONG_MIDAIR_BALL_INTERVAL 250
#define PONG_REPLY_TIMEOUT 250

int pongState = PONG_STATE_WAITING_FIRST_STRIKE;
int pongDir = 1;
int pongPos = 0;
int pongBrightness = 0;
int pongBrightnessDir = 1;
long pongLastMidairMoveTstamp = 0;
int  pongLevel = 0;
long waitForReplyStartTstamp = 0;

void turnCandle(int candle, int intensity);
void turnCandle(int candle, int intensity, long col);
int getLitCandle(int candles);

void resetPong() {
  pongState = PONG_STATE_WAITING_FIRST_STRIKE;
  //  pongDir = 1;
  //  pongPos = 0;
  pongBrightness = 0;
  pongBrightnessDir = 1;
  pongLastMidairMoveTstamp = 0;
  pongLevel = 0;
  waitForReplyStartTstamp = 0;
}

int getPongDuration(int level) {
  return constrain(200-level*5, 10, 200);
}

void drawPong() {
  for (int i = 0 ; i < CANDLE_COUNT; ++i) {
    if (i == pongPos) {
      long ballColor = strip.gamma32(strip.ColorHSV((millis() * 3) % 65536L));
      turnCandle(i, MAX_INTENSITY, ballColor);
    }
    else {
      turnCandle(i, MIN_INTENSITY);
    }
  }
}

void handleWaitingFirstStrike() {
  long ballColor = strip.gamma32(strip.ColorHSV((millis() * 3) % 65536L));
  if (pongPos == 0) {
    for (int i = 0; i < 3; ++i) {
      long b = (MIN_INTENSITY + MAX_INTENSITY) / 2 / (1 << (2 - i));
      turnCandle(i, b * pongBrightness / MAX_INTENSITY, ballColor);
      pongBrightness += pongBrightnessDir;
      if (pongBrightness >= MAX_INTENSITY || pongBrightness <= MIN_INTENSITY) {
        pongBrightnessDir *= -1;
      }
    }
    // Serial.println();
  }
  else {
    for (int i = 0; i < 3; ++i) {
      long b = (MIN_INTENSITY + MAX_INTENSITY) / 2 / (1 << (2 - i));
      turnCandle(CANDLE_COUNT - i - 1, b * pongBrightness / MAX_INTENSITY, ballColor);
      pongBrightness += pongBrightnessDir;
      if (pongBrightness >= MAX_INTENSITY || pongBrightness <= MIN_INTENSITY) {
        pongBrightnessDir *= -1;
      }
    }
  }
  int candle = getLitCandle(CANDLE_COUNT);
  if (candle == ((pongDir == 1) ? 0 : CANDLE_COUNT - 1)) {
    tone(SPEAKER_PIN, (pongDir == 1) ? 880 : 1760, getPongDuration(pongLevel));
    pongState = PONG_STATE_BALL_MIDAIR;
    //    pongLastMidairMoveTstamp = millis();
    //    waitForReplyStartTstamp = millis();
    delay(getPongDuration(pongLevel));
  }
}

void handleBallMidair() {
  //  Serial.println((millis() - pongLastMidairMoveTstamp));
  if ((millis() - pongLastMidairMoveTstamp) > getPongDuration(pongLevel)) {
    pongPos += pongDir;
    if ((pongDir > 0 && pongPos >= (CANDLE_COUNT - 1)) || (pongDir < 0 && pongPos <= 0)) {
      pongState = PONG_STATE_AWAIT_REPLY_STRIKE;
      waitForReplyStartTstamp = millis();
    }
    pongLastMidairMoveTstamp = millis();
  }
  drawPong();
}

void handleAwaitReplyStrike() {
  int candle = getLitCandle(CANDLE_COUNT);
  if ((pongDir > 0 && candle == (CANDLE_COUNT - 1)) || (pongDir < 0 && candle == 0)) {
    pongLevel++;
    pongDir *= -1;
    tone(SPEAKER_PIN, (pongDir == 1) ? 880 : 1760, getPongDuration(pongLevel));
    pongState = PONG_STATE_BALL_MIDAIR;
    delay(getPongDuration(pongLevel));
  }
  else if ((millis() - waitForReplyStartTstamp) > PONG_REPLY_TIMEOUT) {
    pongDir *= -1;
    pongState = PONG_STATE_FAILURE;
  }
}

void handlePong() {
  //  Serial.println(String("pongState=") + pongState);
  switch (pongState) {

    case PONG_STATE_WAITING_FIRST_STRIKE:
      handleWaitingFirstStrike();
      break;

    case PONG_STATE_BALL_MIDAIR:
      handleBallMidair();
      break;

    case PONG_STATE_AWAIT_REPLY_STRIKE:
      handleAwaitReplyStrike();
      break;

    case PONG_STATE_FAILURE:

      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < CANDLE_COUNT; ++j) {
          turnCandle(
            j,
            MAX_INTENSITY,
            (pongDir < 0) ?
            ((j < CANDLE_COUNT / 2) ? 0x00ff00 : 0x0000ff) :
            ((j >= CANDLE_COUNT / 2) ? 0x00ff00 : 0x0000ff));
        }
        delay(250);
        for (int j = 0; j < CANDLE_COUNT; ++j) {
          turnCandle(j, MIN_INTENSITY);
        }
        delay(250);
      }
      resetPong();
      break;
  }
}
