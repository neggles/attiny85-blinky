/*
 * WS2812B blinkenlight firmware using an ATTiny85
 * Should work with Adafruit Trinket and Gemma also
 * CC BY-NC-SA 4.0 license
 * Andrew Holmes 2015
 * 
 * Status LED will toggle with every button press.
 * Operates at no faster than 50fps, with a frame counter.
 * At least in theory. I have no idea if that works.
 */

#include <Adafruit_NeoPixel.h> 
#define LEDS 4  // Number of LEDS in your NeoPixel string 
#define DTAPIN 0 // LED string data pin
#define LEDPIN 1 // Status LED pin. TBH, I'm not really using this,  but it's on my PCB.
#define BTNPIN 2 // Button pin. This uses a pinchange interrupt, and has the internal pullup enabled. 
#define BTNDLY 250 // Debounce delay on button.
#define STPBRT 64 // Strip brightness.

// Start NeoPixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, DTAPIN, NEO_GRB + NEO_KHZ800);

// The following is pin change interrupt black magic.
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Global Variables
uint8_t pattern = 1; // Pattern selection
uint8_t counter = 0; 
uint8_t frame_delay = 20; // 20ms/frame, ~50fps.
uint8_t colors[3];
uint8_t hue = 200;
uint8_t myPix=0;
uint8_t lastPix=0;
uint32_t setColor=0;
bool LEDstate = false;
bool firstrun = true;
bool even = false;
volatile unsigned long debounce = 0; // counter for debounce reasons
volatile unsigned long framecount = 0; // counter for FPS reasons

void setup() {
  pinMode(BTNPIN, INPUT_PULLUP);
  pinMode(LEDPIN, OUTPUT);
  toggleLED(); // turn on status light so we know it's booted
  
  sbi(GIMSK,PCIE);   // Turn on Pin Change interrupt, somehow. I don't know. Registers, man.
  sbi(PCMSK,PCINT2); // Which pins are affected by the interrupt. PCINT1 = PB1 = digital pin 1 on attiny85.

  strip.begin(); // wake the kraken
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(STPBRT); // turn this shit down oh god so bright
}

void loop() {
  if ((millis() - framecount) > frame_delay) {
    framecount = millis();
    switchPattern(pattern); // Generate next frame
    strip.show(); // Push frame to strip
    counter++; // Increment universal per-pattern counter
  }
  if (counter > 255) { 
   counter = 0;
  }
}

/* pick a pattern */
void switchPattern(uint8_t var) {
      switch (var) {
        case 1:
          // show rainbow
          rainbowCycle();
        break;
        case 2:
          // Randomly light a pixel.
          randomGlowy();
        break;
        case 3:
          fadeEvenOdd();
        break;    
//        case 4:
          // rainbow firefly, 1px at random
//          colorFirefly();
//        break;
        default: // Reset pattern to #1 if selected pattern number is over number of patterns
         pattern = 1; 
        break;
      }
}

// ------------------------------------------------------------------------------
// Patterns section;

// Cycles a rainbow across the LEDs
void rainbowCycle() {
  if(firstrun == true) { setFramerate(60); firstrun = false; }
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + counter) & 255));
    }
    
}

// Picks a random LED and makes it a random color.
void randomGlowy() {
  if(firstrun == true) { setFramerate(10); firstrun = false; }
   strip.setPixelColor(random(LEDS),Wheel(random(360)));
}

// Pulse even/odd pixels

void fadeEvenOdd() {
  if(firstrun == true) { setFramerate(250); firstrun = false; hue = random(360); }
  if(even == true) { fadeEven(); } else { fadeOdd(); }
}

void fadeEven () {
  if(counter < 128) {
    for (int i = 0; i < strip.numPixels(); i++) {
      if (i % 2) {
        HSVtoRGB(hue,255,(counter)*2,colors);
        strip.setPixelColor(i,colors[0],colors[1],colors[2]);
      }
    }
  } else {
    for (int i = 0; i < strip.numPixels(); i++) {
      if (i % 2) {
        HSVtoRGB(hue,255,255-((counter-128)*2),colors);
        strip.setPixelColor(i,colors[0],colors[1],colors[2]);
      }
    }
  }
  if(counter == 254) {
    even = false;
    hue = random(360);
  }
}

void fadeOdd() {  
  if(counter < 128) {
    for (int i = 0; i < strip.numPixels(); i++) {
      if (!(i % 2)) {
        HSVtoRGB(hue,255,(counter)*2,colors);
        strip.setPixelColor(i,colors[0],colors[1],colors[2]);
      }
    }
  } else {
    for (int i = 0; i < strip.numPixels(); i++) {
      if (!(i % 2)) {
        HSVtoRGB(hue,255,255-((counter-128)*2),colors);
        strip.setPixelColor(i,colors[0],colors[1],colors[2]);
      }
    }
  }
  if(counter == 254) {
    even = true;
    hue = random(360);
  }
}

/* // dat firefly tho
void colorFirefly() {
  if(firstrun == true) { setFramerate(10); firstrun = false; }
  if(myPix != lastPix) {
    if(counter<16) {
      int colorV = sin((6.28/30)*(float)(counter)) *255;
      HSVtoRGB((359/16)*counter, 255, colorV, colors);
      strip.setPixelColor(myPix, colors[0], colors[1], colors[2]);
    } else {
      lastPix=myPix;
      counter=0;
      colorFast(0);
    }
  } else {
    myPix=random(0,strip.numPixels());
  }
}*/

// ------------------------------------------------------------------------------
// System section;

void setFramerate(uint8_t framerate) {
  frame_delay = (1000 / framerate);
}

void toggleLED() { // if LED is on, turn it off. if LED is off, turn it on.
  if(LEDstate == true) {
    digitalWrite(LEDPIN, LOW);
    LEDstate = false;
  } else {
    digitalWrite(LEDPIN, HIGH);
    LEDstate = true; 
  }
}

ISR(PCINT0_vect) { // This is the shit that runs on the PCInt triggering.
  if (digitalRead(BTNPIN) == LOW && (millis() - debounce) > BTNDLY) {
    debounce = millis();
    counter = 0;
    pattern++;
    firstrun = true;
    colorFast(0);
    toggleLED();
  }
}

// manipulation helpers 

// Write a color to the whole string
void colorFast(uint32_t c) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
}

uint32_t Wheel(byte WheelPos) {
    if (WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}


// HSV to RGB colors
// hue: 0-359, sat: 0-255, val (lightness): 0-255
// adapted from http://funkboxing.com/wordpress/?p=1366
void HSVtoRGB(int hue, int sat, int val, uint8_t * colors) {
    int r, g, b, base;
    if (sat == 0) { // Achromatic color (gray).
        colors[0] = val;
        colors[1] = val;
        colors[2] = val;
    } else {
        base = ((255 - sat) * val) >> 8;
        switch (hue / 60) {
        case 0:
            colors[0] = val;
            colors[1] = (((val - base) * hue) / 60) + base;
            colors[2] = base;
            break;
        case 1:
            colors[0] = (((val - base) * (60 - (hue % 60))) / 60) + base;
            colors[1] = val;
            colors[2] = base;
            break;
        case 2:
            colors[0] = base;
            colors[1] = val;
            colors[2] = (((val - base) * (hue % 60)) / 60) + base;
            break;
        case 3:
            colors[0] = base;
            colors[1] = (((val - base) * (60 - (hue % 60))) / 60) + base;
            colors[2] = val;
            break;
        case 4:
            colors[0] = (((val - base) * (hue % 60)) / 60) + base;
            colors[1] = base;
            colors[2] = val;
            break;
        case 5:
            colors[0] = val;
            colors[1] = base;
            colors[2] = (((val - base) * (60 - (hue % 60))) / 60) + base;
            break;
        }

    }
}

