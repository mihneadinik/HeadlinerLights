#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#include "hsv.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

// Arduino pins
#define WIDE_LED_PIN 9
#define NARROW_LED_PIN 8
#define REMOTE_PIN 7
#define CAMERA_SENSOR_TRIGGER 6
#define REVERSE_TRIGGER A0

// Button decoded values
#define IR_1 69
#define IR_2 70
#define IR_3 71
#define IR_4 68
#define IR_5 64
#define IR_6 67
#define IR_7 7
#define IR_8 21
#define IR_9 9
#define IR_STAR 22
#define IR_0 25
#define IR_HASHTAG 13
#define IR_up 24
#define IR_left 8
#define IR_ok 28
#define IR_right 90
#define IR_down 82

// Application states
#define SHOW_COLOR 1
#define REPEAT_TWINKLE_SINGLE 2
#define REPEAT_TWINKLE_BOTH 3
#define NOTHING 4

// Constants
#define NUM_PIXELS 7
#define MAXVAL 250
#define MINVAL 0
#define TWINKLE_DELAY 200
#define MAXHUE 256*6
#define MIDHUE 256*3
#define CAMERA_SENSOR_DELAY 10000
#define REVERSE_THRESHOLD 100

// Colors
#define COLORNUM 45
#define WHITEINDEX 44
#define REDINDEX 4
#define PINKINDEX 38

Adafruit_NeoPixel pixelsWide(NUM_PIXELS, WIDE_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsNarrow(NUM_PIXELS, NARROW_LED_PIN, NEO_GRB + NEO_KHZ800);
int hueNarrow = 0, saturationNarrow = 0, saturationWide = 0, hueWide = 0, position = 0;
int brightnessWide = 200, brightnessNarrow = 150;
int colorSelectWide = 0, colorSelectNarrow = 0;
bool selectWide = false, selectNarrow = false, rainbowNarrow = false, rainbowWide = false;
bool reverse_triggered = false, sensors_on = false;
uint16_t prevCommand = IR_7;

unsigned long currTime, twinkleStartTime, reverseStartTime;

uint32_t colors[COLORNUM];

void initialiseColors() {
  colors[0] = pixelsWide.Color(139, 0, 0);
  colors[1] = pixelsWide.Color(178, 34, 34);
  colors[2] = pixelsWide.Color(205, 92, 92);
  colors[3] = pixelsWide.Color(220, 20, 60);
  colors[4] = pixelsWide.Color(255, 0, 0);
  colors[5] = pixelsWide.Color(255, 69, 0);
  colors[6] = pixelsWide.Color(240, 128, 128);
  colors[7] = pixelsWide.Color(255, 127, 80);
  colors[8] = pixelsWide.Color(255, 160, 122);
  colors[9] = pixelsWide.Color(255, 165, 0);
  colors[10] = pixelsWide.Color(255, 215, 0);
  colors[11] = pixelsWide.Color(218, 165, 32);
  colors[12] = pixelsWide.Color(189, 183, 107);
  colors[13] = pixelsWide.Color(240, 230, 140);
  colors[14] = pixelsWide.Color(255, 255, 0);
  colors[15] = pixelsWide.Color(154, 205, 50);
  colors[16] = pixelsWide.Color(124, 252, 0);
  colors[17] = pixelsWide.Color(173, 255, 47);
  colors[18] = pixelsWide.Color(0, 128, 0);
  colors[19] = pixelsWide.Color(0, 255, 0);
  colors[20] = pixelsWide.Color(152, 251, 152);
  colors[21] = pixelsWide.Color(0, 250, 154);
  colors[22] = pixelsWide.Color(102, 205, 170);
  colors[23] = pixelsWide.Color(32, 178, 170);
  colors[24] = pixelsWide.Color(0, 206, 209);
  colors[25] = pixelsWide.Color(0, 255, 255);
  colors[26] = pixelsWide.Color(176, 224, 230);
  colors[27] = pixelsWide.Color(0, 191, 255);
  colors[28] = pixelsWide.Color(30, 144, 255);
  colors[29] = pixelsWide.Color(0, 0, 139);
  colors[30] = pixelsWide.Color(0, 0, 255);
  colors[31] = pixelsWide.Color(75, 0, 130);
  colors[32] = pixelsWide.Color(138, 43, 226);
  colors[33] = pixelsWide.Color(123, 104, 238);
  colors[34] = pixelsWide.Color(148, 0, 211);
  colors[35] = pixelsWide.Color(186, 85, 211);
  colors[36] = pixelsWide.Color(128, 0, 128);
  colors[37] = pixelsWide.Color(255, 0, 255);
  colors[38] = pixelsWide.Color(255, 20, 147);
  colors[39] = pixelsWide.Color(255, 192, 203);
  colors[40] = pixelsWide.Color(255, 228, 196);
  colors[41] = pixelsWide.Color(244, 164, 96);
  colors[42] = pixelsWide.Color(210, 105, 30);
  colors[43] = pixelsWide.Color(139, 69, 19);
  colors[44] = pixelsWide.Color(255, 255, 255); 
}

// Start wide LED ring on white
void startWide() {
  pixelsWide.begin();
  pixelsWide.clear();
  pixelsWide.setBrightness(brightnessWide);

  for (int i = 0; i < NUM_PIXELS; i++) {
    pixelsWide.setPixelColor(i, 255, 255, 255);
  }

  pixelsWide.show();
}

// Start narrow LED ring on white
void startNarrow() {
  pixelsNarrow.begin();
  pixelsNarrow.clear();
  pixelsNarrow.setBrightness(brightnessWide);

  for (int i = 0; i < NUM_PIXELS; i++) {
    pixelsNarrow.setPixelColor(i, 255, 255, 255);
  }

  pixelsNarrow.show();
}

void resetValues() {
  brightnessNarrow = 150;
  brightnessWide = 200;
  selectNarrow = false;
  selectWide = false;
  rainbowNarrow = false;
  rainbowWide = false;
  colorSelectWide = 0;
  colorSelectNarrow = 0;
  hueNarrow = 0;
  saturationNarrow = 0;
  saturationWide = 0;
  hueWide = 0;
  position = 0;
}

// returns true and selects a new state if a new command arrived, false otherwise
bool checkIRRemoteCommand() {
  if (IrReceiver.decode()) {
    IrReceiver.resume();
    selectCommand(IrReceiver.decodedIRData.command);
    return true;
  }

  return false;
}

// Pull down the relay trigger to ground for a msec (as if you would push the button)
void signalCameraAndSensor() {
  pinMode(CAMERA_SENSOR_TRIGGER, OUTPUT);
  digitalWrite(CAMERA_SENSOR_TRIGGER, LOW);
  delay(100);

  // set pin back to high impedance
  pinMode(CAMERA_SENSOR_TRIGGER, INPUT);
  // change state
  sensors_on = !sensors_on;
}

void checkReverseStatus() {
  // range 0 to 1023 (ADC converted)
  int reverse_value = analogRead(REVERSE_TRIGGER);

  currTime = millis();
  if (reverseStartTime + CAMERA_SENSOR_DELAY >= currTime && sensors_on) {
    // turn off sensors if time has passed and they are already on
    signalCameraAndSensor();
  }

  if (reverse_value > REVERSE_THRESHOLD) {
    // reverse is engaged => set trigger
    reverse_triggered = true;
  } else {
    // reverse is not engaged => check if you need to turn on front sensor
    if (reverse_triggered) {
      // start timer, turn on sensors and reset trigger event
      reverseStartTime = millis();
      reverse_triggered = false;

      if (!sensors_on) {
        // if sensors are already on, just reset the countdown timer
        signalCameraAndSensor();
      }
    }    
  }
}

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(REMOTE_PIN);
  startWide();
  startNarrow();
  initialiseColors();
}

void increaseBrightnessWide() {
  brightnessWide = min(brightnessWide + 25, MAXVAL);
}

void decreaseBrightnessWide() {
  brightnessWide = max(brightnessWide - 25, MINVAL);
}

void increaseBrightnessNarrow() {
  brightnessNarrow = min(brightnessNarrow + 25, MAXVAL);
}

void decreaseBrightnessNarrow() {
  brightnessNarrow = max(brightnessNarrow - 25, MINVAL);
}

void increaseBrightness() {
  // nothing selected => increase both and exit
  if (!selectWide && !selectNarrow) {
    increaseBrightnessNarrow();
    increaseBrightnessWide();
    return;
  }

  // otherwise increase the selected one(s)
  if (selectWide) {
    increaseBrightnessWide();
  }
  if (selectNarrow) {
    increaseBrightnessNarrow();
  }
}

void decreaseBrightness() {
  // nothing selected => decrease both and exit
  if (!selectWide && !selectNarrow) {
    decreaseBrightnessNarrow();
    decreaseBrightnessWide();
    return;
  }

  // otherwise decrease the selected one(s)
  if (selectWide) {
    decreaseBrightnessWide();
  }
  if (selectNarrow) {
    decreaseBrightnessNarrow();
  }
}

void increaseRGBWide() {
  colorSelectWide = (colorSelectWide + 1) % COLORNUM;
}

void decreaseRGBWide() {
  colorSelectWide = (colorSelectWide - 1) % COLORNUM;
}

void increaseRGBNarrow() {
  colorSelectNarrow = (colorSelectNarrow + 1) % COLORNUM;
}

void decreaseRGBNarrow() {
  colorSelectNarrow = (colorSelectNarrow - 1) % COLORNUM;
}

void increaseRGB() {
  // nothing selected => increase both and exit
  if (!selectWide && !selectNarrow) {
    increaseRGBNarrow();
    increaseRGBWide();
    return;
  }

  // otherwise increase the selected one(s)
  if (selectWide) {
    increaseRGBWide();
  }
  if (selectNarrow) {
    increaseRGBNarrow();
  }
}

void decreaseRGB() {
  // nothing selected => decrease both and exit
  if (!selectWide && !selectNarrow) {
    decreaseRGBNarrow();
    decreaseRGBWide();
    return;
  }

  // otherwise decrease the selected one(s)
  if (selectWide) {
    decreaseRGBWide();
  }
  if (selectNarrow) {
    decreaseRGBNarrow();
  }
}

// Turn LED rings off
void turnWideOff() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixelsWide.setPixelColor(i, 0, 0, 0); 
  }
}

void turnNarrowOff() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixelsNarrow.setPixelColor(i, 0, 0, 0); 
  }
}

// cycling effect
void narrowCycle(int position) {
  pixelsNarrow.setBrightness(brightnessNarrow);
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixelsNarrow.setPixelColor((i + position) % NUM_PIXELS, getPixelColorHsv(i, hueNarrow, saturationNarrow, pixelsNarrow.gamma8(i * (255 / NUM_PIXELS))));    
  }
  pixelsNarrow.show();

  // cycle through colors in rainbow mode
  if (rainbowNarrow) {
    hueNarrow = hueNarrow + 5;
    hueNarrow %= MAXHUE;
  }
}

void wideCycle(int position) {
  pixelsWide.setBrightness(brightnessWide);
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixelsWide.setPixelColor((i + position) % NUM_PIXELS, getPixelColorHsv(i, hueWide, saturationWide, pixelsWide.gamma8(i * (255 / NUM_PIXELS))));    
  }
  pixelsWide.show();

  // cycle through colors in rainbow mode
  if (rainbowWide) {
    hueWide = hueWide + 5;
    hueWide %= MAXHUE;
  }
}

// executes the twinkle effect by cycling the rings
void twinkleExecutor(bool single) {
  if (single) {
    // only narrow is changing => wide is static
    pixelsWide.setBrightness(brightnessWide);
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixelsWide.setPixelColor(i, colors[colorSelectWide]);      
    }
    pixelsWide.show();
  } else {
    // wide is also cycling
    wideCycle(position);
  }
  
  // narrow is always cycling
  narrowCycle(position);

  // increase blacked out LED position on ring
  position = (position + 1) % NUM_PIXELS;  
}

// Keeps the twinkle looping with a certain speed (twinkle_delay)
void twinkleCommander(bool single) {
  // when a cycle is completed
  bool new_cycle = false;
  
  while (true) {
    if (new_cycle) {
      currTime = millis();
      if (currTime > twinkleStartTime + TWINKLE_DELAY) {
        // time to change the twinkle (move on a new position) and restart counter
        twinkleStartTime = millis();
        twinkleExecutor(single);
      } else {
        // check for a new command and exit if necessary
        checkReverseStatus();
        if (checkIRRemoteCommand()) return;     
      }
    } else {
      new_cycle = true;
      twinkleStartTime = millis();
      twinkleExecutor(single);
    }
  }
}

// LED rings are set on a solid color
void fullColor() {
  pixelsNarrow.clear();
  pixelsWide.clear();

  pixelsWide.setBrightness(brightnessWide);
  pixelsNarrow.setBrightness(brightnessNarrow);

  for (int i = 0; i < NUM_PIXELS; i++) {
    pixelsNarrow.setPixelColor(i, colors[colorSelectNarrow]);
    pixelsWide.setPixelColor(i, colors[colorSelectWide]);
  }

  pixelsNarrow.show();
  pixelsWide.show();
}

// IR Remote decoder
void selectCommand(uint16_t command) {
  switch (command) {
    // Command executors (reiterate last input from the remote)
    case NOTHING:
      // No changes made on LED rings
      break;
    case SHOW_COLOR:
      // Change the displayed color
      prevCommand = NOTHING;
      fullColor();
      break;      
    case REPEAT_TWINKLE_BOTH:
      // enter twinkle mode on both rings
      twinkleCommander(false);
      break;
    case REPEAT_TWINKLE_SINGLE:
      // enter twinkle mode on narrow ring
      twinkleCommander(true);
      break;

    // Command selectors (direct input from the remote) - doesn't change current LED status
    // Full Static White
    case IR_1:
      prevCommand = SHOW_COLOR;
      colorSelectNarrow = WHITEINDEX;
      colorSelectWide = WHITEINDEX;
      break;
    // Full Static Red
    case IR_2:
      prevCommand = SHOW_COLOR;
      colorSelectNarrow = REDINDEX;
      colorSelectWide = REDINDEX;
      break;
    // Full Static Random Color
    case IR_3:
      prevCommand = SHOW_COLOR;
      colorSelectNarrow = rand() % COLORNUM;
      colorSelectWide = rand() % COLORNUM;
      break;

    // Full Twinkle White Color
    case IR_4:
      prevCommand = REPEAT_TWINKLE_BOTH;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = hueWide = 255;
      saturationNarrow = saturationWide = 0;
      break;
    // Full Twinkle Previously Selected Color
    case IR_5:
      prevCommand = REPEAT_TWINKLE_BOTH;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = hueWide = 0;
      saturationNarrow = saturationWide = 255;
      break;
    // Full Twinkle Rainbow Effect
    case IR_6:
      prevCommand = REPEAT_TWINKLE_BOTH;
      rainbowWide = true;
      rainbowNarrow = true;
      hueNarrow = 0;
      hueWide = MIDHUE;
      saturationNarrow = saturationWide = 255;
      break;

    // Single Twinkle Red Static White Looping
    case IR_7:
      prevCommand = REPEAT_TWINKLE_SINGLE;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = 255;
      saturationNarrow = 0;
      colorSelectWide = REDINDEX;
      break;
    // Single Twinkle Mauve Static White Looping
    case IR_8:
      prevCommand = REPEAT_TWINKLE_SINGLE;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = 255;
      saturationNarrow = 0;
      colorSelectWide = PINKINDEX;
      break;
    // Single Twinkle Rainbow Static White Looping
    case IR_9:
      prevCommand = REPEAT_TWINKLE_SINGLE;
      rainbowWide = true;
      rainbowNarrow = false;
      hueNarrow = saturationWide = 255;
      hueWide = saturationNarrow = 0;
      break;

    // Selects or Deselects both LED rings
    case IR_0:
      if (selectWide && selectNarrow) {
        selectWide = false;
        selectNarrow = false;        
      } else {
        selectWide = true;
        selectNarrow = true;
      }
      break;
    // Selects Narrow LED ring      
    case IR_STAR:
      selectWide = false;
      selectNarrow = true;
      break;
    // Selects Wide LED ring
    case IR_HASHTAG:
      selectNarrow = false;
      selectWide = true;
      break;

    // Increases/Decreases LED brightness (if static color display mode was previously selected, restart it)
    case IR_up:
      if (prevCommand == NOTHING)
        prevCommand = SHOW_COLOR;
      increaseBrightness();
      break;
    case IR_down:
      if (prevCommand == NOTHING)
        prevCommand = SHOW_COLOR;
      decreaseBrightness();
      break;

    // Increases/Decreases LED color (if static color display mode was previously selected, restart it)
    case IR_left:
      if (prevCommand == NOTHING)
        prevCommand = SHOW_COLOR;
      decreaseRGB();
      break;
    case IR_right:
      if (prevCommand == NOTHING)
        prevCommand = SHOW_COLOR;
      increaseRGB();
      break;

    // Reset initial values
    case IR_ok:
      if (prevCommand == NOTHING)
        prevCommand = SHOW_COLOR;
      resetValues();
      break;
  }
}

void loop() {
  checkReverseStatus();
  if (!checkIRRemoteCommand()) {
    // keep the last command if nothing new selected
    selectCommand(prevCommand);
  }
}