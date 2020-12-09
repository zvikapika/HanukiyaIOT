//#define USE_NEOPIXEL
#define USE_FASTLED

//#define DISABLE_INTERRUPTS_FOR_LEDS
#define ADD_1MS_FOR_LEDS

//#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
//#endif
#ifdef USE_FASTLED
#include <FastLED.h>
#endif

#define LED_PIN     22
#define NUM_LEDS    8
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#ifdef USE_FASTLED
CRGB leds[NUM_LEDS];
#endif

//#ifdef USE_NEOPIXEL
// todo: remove and replace with fastled specific coloring
Adafruit_NeoPixel strip(CANDLE_COUNT * LEDS_PER_CANDLE, LED_PIN, NEO_GRB + NEO_KHZ800);
//#endif

void stripShow() {
#ifdef DISABLE_INTERRUPTS_FOR_LEDS  
  portDISABLE_INTERRUPTS(); 
#endif  
#ifdef ADD_1MS_FOR_LEDS  
  delay(1); 
#endif  
#ifdef USE_NEOPIXEL
   strip.show(); 
#endif
#ifdef USE_FASTLED
    FastLED.show();
#endif
#ifdef DISABLE_INTERRUPTS_FOR_LEDS  
  portENABLE_INTERRUPTS();
#endif  
}

void stripBegin() {
#ifdef USE_NEOPIXEL
 strip.begin();
#endif
#ifdef USE_FASTLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
#endif
}

void stripSetBrightness(int b) {
#ifdef USE_NEOPIXEL
  strip.setBrightness(b);  
#endif
#ifdef USE_FASTLED
  FastLED.setBrightness(b);
#endif
}

void stripSetPixelColor(int index, int r, int g, int b) {
//  leds[i] = CHSV( random8(), 255, random8());
#ifdef USE_NEOPIXEL
  strip.setPixelColor(index, r, g, b);
#endif
//  leds[i] = CRGB(r, g, b);
}
