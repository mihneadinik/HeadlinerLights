#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#include "hsv.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

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
#define SHOWCOLOR 1
#define REPEATTWINKLESINGLE 2
#define REPEATTWINKLEBOTH 3
#define NOTHING 4

#define WIDE_LED_PIN 9
#define NARROW_LED_PIN 8
#define REMOTE_PIN 7
#define NUMPIXELS 7
#define MAXVAL 250
#define MINVAL 0
#define COLORNUM 45
#define WHITEINDEX 44
#define REDINDEX 4
#define PINKINDEX 38
#define TWINKLEDELAY 150
#define MAXHUE 256*6
#define MIDHUE 256*3

Adafruit_NeoPixel pixelsWide(NUMPIXELS, WIDE_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsNarrow(NUMPIXELS, NARROW_LED_PIN, NEO_GRB + NEO_KHZ800);
int hueNarrow = 0, saturationNarrow = 0, saturationWide = 0, hueWide = 0, position = 0;
int brightnessWide = 200, brightnessNarrow = 150;
int colorSelectWide = 0, colorSelectNarrow = 0;
bool selectWide = false, selectNarrow = false, rainbowNarrow = false, rainbowWide = false;
uint16_t currCommand = 0, prevCommand = IR_7;

unsigned long currTime, startTime;

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

void startWide() {
  pixelsWide.begin();
  pixelsWide.clear();
  pixelsWide.setBrightness(brightnessWide);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixelsWide.setPixelColor(i, 255, 255, 255);
  }

  pixelsWide.show();
}

void startNarrow() {
  pixelsNarrow.begin();
  pixelsNarrow.clear();
  pixelsNarrow.setBrightness(brightnessWide);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixelsNarrow.setPixelColor(i, 255, 255, 255);
  }

  pixelsNarrow.show();
}

void resetValues() {
  brightnessNarrow = 150;
  brightnessWide = 200;
  selectNarrow = false;
  selectWide = false;
  colorSelectWide = 0;
  colorSelectNarrow = 0;
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

  if (selectWide) {
    decreaseRGBWide();
  }
  if (selectNarrow) {
    decreaseRGBNarrow();
  }
}

void turnWideOff() {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixelsWide.setPixelColor(i, 0, 0, 0); 
  }
}

void turnNarrowOff() {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixelsNarrow.setPixelColor(i, 0, 0, 0); 
  }
}

void twinkleExecutor(bool single) {
  // only narrow is changing
  if (single) {
    pixelsWide.setBrightness(brightnessWide);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixelsWide.setPixelColor(i, colors[colorSelectWide]);      
    }
    pixelsWide.show();
  } else {
    wideCycle(position);
  }
  
  narrowCycle(position);
  position = (position + 1) % NUMPIXELS;  
}

void twinkleCommander(bool single) {
  bool changed = false;
  
  while (true) {
    if (changed) {
      currTime = millis();
      if (currTime > startTime + TWINKLEDELAY) {
        // change the twinkle
        startTime = millis();
        twinkleExecutor(single);
      } else {
        // don't change twinkle, check new command
        if (IrReceiver.decode()) {
          IrReceiver.resume();
          selectCommand(IrReceiver.decodedIRData.command);
          break;
        }        
      }
    } else {
      changed = true;
      startTime = millis();
      twinkleExecutor(single);
    }
  }
}

void narrowCycle(int position) {
  pixelsNarrow.setBrightness(brightnessNarrow);
  for (int i = 0; i < NUMPIXELS; i++) {
    pixelsNarrow.setPixelColor((i + position) % NUMPIXELS, getPixelColorHsv(i, hueNarrow, saturationNarrow, pixelsNarrow.gamma8(i * (255 / NUMPIXELS))));    
  }
  pixelsNarrow.show();

  if (rainbowNarrow) {
    hueNarrow = hueNarrow + 4;
    hueNarrow %= MAXHUE;
  }
}

void wideCycle(int position) {
  pixelsWide.setBrightness(brightnessWide);
  for (int i = 0; i < NUMPIXELS; i++) {
    pixelsWide.setPixelColor((i + position) % NUMPIXELS, getPixelColorHsv(i, hueWide, saturationWide, pixelsWide.gamma8(i * (255 / NUMPIXELS))));    
  }
  pixelsWide.show();

  if (rainbowWide) {
    hueWide = hueWide + 4;
    hueWide %= MAXHUE;
  }
}

void fullColor() {
  pixelsNarrow.clear();
  pixelsWide.clear();

  pixelsWide.setBrightness(brightnessWide);
  pixelsNarrow.setBrightness(brightnessNarrow);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixelsNarrow.setPixelColor(i, colors[colorSelectNarrow]);
    pixelsWide.setPixelColor(i, colors[colorSelectWide]);
  }

  pixelsNarrow.show();
  pixelsWide.show();
}

void selectCommand(uint16_t command) {
  switch (command) {
    case NOTHING:
      break;
    case SHOWCOLOR:
      prevCommand = NOTHING;
      fullColor();
      break;      
    case REPEATTWINKLEBOTH:
      twinkleCommander(false);
      break;
    case REPEATTWINKLESINGLE:
      twinkleCommander(true);
      break;
    case IR_1:
      prevCommand = SHOWCOLOR;
      colorSelectNarrow = WHITEINDEX;
      colorSelectWide = WHITEINDEX;
      break;
    case IR_2:
      prevCommand = SHOWCOLOR;
      colorSelectNarrow = REDINDEX;
      colorSelectWide = REDINDEX;
      break;
    case IR_3:
      prevCommand = SHOWCOLOR;
      colorSelectNarrow = rand() % COLORNUM;
      colorSelectWide = rand() % COLORNUM;
      break;
    case IR_4:
      prevCommand = REPEATTWINKLEBOTH;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = hueWide = 255;
      saturationNarrow = saturationWide = 0;
      break;
    case IR_5:
      prevCommand = REPEATTWINKLEBOTH;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = hueWide = 0;
      saturationNarrow = saturationWide = 255;
      break;
    case IR_6:
      prevCommand = REPEATTWINKLEBOTH;
      rainbowWide = true;
      rainbowNarrow = true;
      hueNarrow = 0;
      hueWide = MIDHUE;
      saturationNarrow = saturationWide = 255;
      break;
    case IR_7:
      prevCommand = REPEATTWINKLESINGLE;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = 255;
      saturationNarrow = 0;
      colorSelectWide = REDINDEX;
      break;
    case IR_8:
      prevCommand = REPEATTWINKLESINGLE;
      rainbowWide = false;
      rainbowNarrow = false;
      hueNarrow = 255;
      saturationNarrow = 0;
      colorSelectWide = PINKINDEX;
      break;
    case IR_9:
      prevCommand = REPEATTWINKLESINGLE;
      rainbowWide = true;
      rainbowNarrow = false;
      hueNarrow = saturationWide = 255;
      hueWide = saturationNarrow = 0;
      break;
    case IR_0:
      if (selectWide && selectNarrow) {
        selectWide = false;
        selectNarrow = false;        
      } else {
        selectWide = true;
        selectNarrow = true;
      }
      break;
    case IR_STAR:
      selectWide = false;
      selectNarrow = true;
      break;
    case IR_HASHTAG:
      selectNarrow = false;
      selectWide = true;
      break;
    case IR_up:
      if (prevCommand == NOTHING)
        prevCommand = SHOWCOLOR;
      increaseBrightness();
      break;
    case IR_left:
      if (prevCommand == NOTHING)
        prevCommand = SHOWCOLOR;
      decreaseRGB();
      break;
    case IR_right:
      if (prevCommand == NOTHING)
        prevCommand = SHOWCOLOR;
      increaseRGB();
      break;
    case IR_down:
      if (prevCommand == NOTHING)
        prevCommand = SHOWCOLOR;
      decreaseBrightness();
      break;
    case IR_ok:
      if (prevCommand == NOTHING)
        prevCommand = SHOWCOLOR;
      resetValues();
      break;
  }
}

void loop() {
  if (IrReceiver.decode()) {
    // new command arrived
    IrReceiver.resume();
    //currCommand = IrReceiver.decodedIRData.command;
    selectCommand(IrReceiver.decodedIRData.command);
  }
  // keep the last command
  selectCommand(prevCommand);
}