

#define SIMON_MASTER_NUDGE 10
#define SIMON_MASTER_RECORD 20
#define SIMON_MASTER_WAIT_FOR_REPLIES 30

#define SIMON_PLAYER_WAITING_FOR_NEXT_NOTE 10
#define SIMON_PLAYER_PLAYBACK 20
#define SIMON_PLAYER_RECORD 30
#define SIMON_PLAYER_FAILED 40

#define SIMON_MASTER_RECORD_MAX_DURATION 3000

#define SIMON_INITIAL_MASTER_WAIT_FOR_REPLIES_SECS 10
#define SIMON_PER_TURN_MASTER_WAIT_FOR_REPLIES_SECS 2

#define NUDGE_INTERVAL_DURATION 120


int simonMasterState = SIMON_MASTER_RECORD;
int simonPlayerState = SIMON_PLAYER_WAITING_FOR_NEXT_NOTE;
long startWaitForRepliesTstamp = 0;
int currWaitForRepliesCandle = 0;
int currWaitForRepliesCandleIntensity = 0;
int currWaitForRepliesCandleDirection = 1;
int masterWaitForRepliesDuration;

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

  handleComm();
  handleRemoteSwitch();
  if (!mode) { // no mode (mode = 0) is normal Hanukiya mode
    handleCandles();
    handleScreensaver();
  }
  else {
    switch (mode) {
      case MODE_SIMON:
        if (isMaster()) {
          handleSimonMaster();
        }
        else {
          handleSimonPlayer();
        }
        break;
    }
  }
}


void handleSimonMaster() {
  switch (simonMasterState) {
    case SIMON_MASTER_NUDGE:
      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j <= CANDLE_COUNT; ++j) {
          turnCandle(j, MAX_INTENSITY, 0x0000ff); // red is the color of nudge
        }
        tone(SPEAKER_PIN, 1760);
        delay(NUDGE_INTERVAL_DURATION);
        for (int j = 0; j <= CANDLE_COUNT; ++j) {
          turnCandle(j, MIN_INTENSITY, 0x000000);
        }
        noTone(SPEAKER_PIN);
        delay(NUDGE_INTERVAL_DURATION);
      }
      simonMasterState = SIMON_MASTER_RECORD;
      recordStartedTstamp = millis();
      break;

    case SIMON_MASTER_RECORD:
      if ((millis() - recordStartedTstamp) < SIMON_MASTER_RECORD_MAX_DURATION) {
        for (int candle = 0; candle < CANDLE_COUNT; ++candle) {
          if (sensorBeingLit(candle)) {
            turnCandle(candle, MAX_INTENSITY, strip.gamma32(strip.ColorHSV(65536L / CANDLE_COUNT * candle)));
            tone(SPEAKER_PIN, simonNotes[candle]);
            delay(SIMON_PLAY_DURATION);
            noTone(SPEAKER_PIN);
            turnCandle(candle, MIN_INTENSITY);
            delay(100);
            param1++;
            updateParam1(param1);
            delay(100);
            //            param2 += char(49+candle);
            //            param2 = "" + char(49+candle);
            //            updateParam2(param2);
            simonMasterState = SIMON_MASTER_WAIT_FOR_REPLIES;
            startWaitForRepliesTstamp = millis();
            currWaitForRepliesCandle = 0;
            currWaitForRepliesCandleIntensity = 0;
            currWaitForRepliesCandleDirection = 1;
          }
        }
      }
      else {
        simonMasterState = SIMON_MASTER_NUDGE;
      }
      break;
    case SIMON_MASTER_WAIT_FOR_REPLIES:
      masterWaitForRepliesDuration =
        SIMON_INITIAL_MASTER_WAIT_FOR_REPLIES_SECS +
        SIMON_PER_TURN_MASTER_WAIT_FOR_REPLIES_SECS;
      if ((millis() - startWaitForRepliesTstamp) / 1000 < masterWaitForRepliesDuration) {
        currWaitForRepliesCandleIntensity += currWaitForRepliesCandleDirection;
        currWaitForRepliesCandleIntensity = constrain(currWaitForRepliesCandleIntensity, MIN_INTENSITY, MAX_INTENSITY);
        turnCandle(
          currWaitForRepliesCandle,
          currWaitForRepliesCandleIntensity,
          0xffffff);
        if (currWaitForRepliesCandleIntensity == MAX_INTENSITY) {
          currWaitForRepliesCandleDirection *= -1;
        }
        else if (currWaitForRepliesCandleIntensity == MIN_INTENSITY) {
          currWaitForRepliesCandleDirection *= -1;
          currWaitForRepliesCandle++;
          currWaitForRepliesCandle %= CANDLE_COUNT;
        }
      }
      else {
        simonMasterState = SIMON_MASTER_NUDGE;
      }
      break;
  }
}

void handleSimonPlayer() {
}

void resetSimon() {
  for (int i = 0; i < MAX_SIMON_TURNS; ++i) {
    simonSequence[i] = 0;
  }
  simonMasterState = SIMON_MASTER_RECORD;
  simonPlayerState = SIMON_PLAYER_WAITING_FOR_NEXT_NOTE;
  recordStartedTstamp = 0;
  startWaitForRepliesTstamp = 0;
  currWaitForRepliesCandle = 0;
  currWaitForRepliesCandleIntensity = 0;
  currWaitForRepliesCandleDirection = 1;
}
