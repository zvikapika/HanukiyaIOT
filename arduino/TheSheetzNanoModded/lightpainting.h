void turnCandle(int candle, int intensity);

void handleLightPainting() {
  for (int t = 50; t > 0; t -= 8) {
    for (int i = 0; i < CANDLE_COUNT; ++i) {
      turnCandle(i, MAX_INTENSITY);
      delay(t);
      turnCandle(i, MIN_INTENSITY);
    }
  }
}
