#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "RTClib.h"

RTC_DS3231 RTC;
Adafruit_7segment disp = Adafruit_7segment();

int COLON_BLINK_RATE = 500;
bool TIME_24_HOUR = false;
int BRIGHTNESS = 10;
int MODE = 0;
int TIME = 0;
// MODES:
//  0 = show time
//  1 = display format
//  2 = adjust hours
//  3 = adjust minutes
//  4 = adjust brightness

long colonTimer;
bool showColon = false;

void setup () {
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  // Uncomment to set to time of compilation
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  
  disp.begin(0x70);
  Serial.println("[c]urrent mode | [n]ext mode");
  disp.setBrightness(BRIGHTNESS);
  colonTimer = millis();
}

void loop () {
  readOptions();
  disp.clear();
  switch (MODE) {
    case 0:
      showTime();
      break;
    case 1:
      editTimeDisplay();
      break;
    case 2: 
      editHourDisplay();
      break;
    case 3:
      editMinutesDisplay();
      break;
    case 4:
      editBrightnessDisplay();
      break;
    default:
      disp.print(MODE);
  }
  blinkColon();
  disp.writeDisplay();
}

void blinkColon () {
  if (millis() - colonTimer > COLON_BLINK_RATE) {
    colonTimer = millis();

    showColon = !showColon;
  }
  disp.drawColon(showColon);
}

void showTime () {
  int displayValue = getDecimalTime();
  disp.print(displayValue, DEC);

  // Pad hours when the time isn't big enough.
  if (TIME_24_HOUR) {
    if (displayValue < 1000) disp.writeDigitNum(0, 0);
    if (displayValue < 100 ) disp.writeDigitNum(1, 0);
  }
  // show a zero in minute1 for 00:00 through 00:09.
  if (displayValue < 10) disp.writeDigitNum(3, 0);
}

void editTimeDisplay () {
  int currentDisplay;
  if (showColon) {
    currentDisplay = TIME_24_HOUR ? 24 : 12;
    disp.print(currentDisplay * 100);
  }
}

void editHourDisplay () {
  int decimalTime = getDecimalTime();
  int minute;
  if (showColon) {
    showTime();
  } else {
    minute = decimalTime % 100;
    disp.writeDigitNum(3, minute / 10);
    disp.writeDigitNum(4, minute % 10);
  }
}

void editMinutesDisplay () {
  int decimalTime = getDecimalTime();
  int hour;
  if (showColon) {
    showTime();
  } else {
    hour = decimalTime / 100;
    disp.writeDigitNum(0, hour / 10);
    disp.writeDigitNum(1, hour % 10);
  }
}

void editBrightnessDisplay () {
  disp.print(BRIGHTNESS);
}

void readOptions () {
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'r') {
      Serial.println("Resetting time");
      TIME = 0;
    }
    if (ch == 'c') {
      Serial.print("Current Mode:");
      Serial.println(MODE);
    }
    if (ch == 'n') {
      cycleMode();
    }
    if (MODE == 1) {
      if (ch == 'x') {
        TIME_24_HOUR = !TIME_24_HOUR;
      }
    } else if (MODE == 2) {
      if (ch == 'i') {
        // increment hour
      } else if (ch == 'd') {
        // decrement hour
      }
    } else if (MODE == 3) {
      if (ch == 'i') {
        // increment minutes
      } else if (ch == 'd') {
        // decrement minutes
      }
    } else if (MODE == 4) {
      if (ch == 'i') {
        BRIGHTNESS++;
        if (BRIGHTNESS > 15) {
          BRIGHTNESS = 1;
        }
      } else if (ch == 'd') {
        BRIGHTNESS--;
        if (BRIGHTNESS < 0) {
          BRIGHTNESS = 15;
        }
      }
      disp.setBrightness(BRIGHTNESS);
    }
  }
}

void cycleMode () {
  MODE++;
  if ( MODE > 4 ) {
    MODE = 0;
  }
  switch (MODE) {
    case 0:
      Serial.println("Showing Time");
      break;
    case 1:
      Serial.println("Editing Time Display 12|24");
      Serial.println("[x] change display");
      break;
    case 2:
      Serial.println("Editing Hours");
      Serial.println("[i]ncrement hour | [d]ecrement hour");
      break;
    case 3:
      Serial.println("Editing Minutes");
      Serial.println("[i]ncrement minutes | [d]ecrement minutes");
      break;
    case 4:
      Serial.println("Editing Brightness");
      Serial.println("[i]ncrement brightness | [d]ecrement brightness");
  }
}

int getDecimalTime () {
  DateTime now = RTC.now();
  int hour = now.hour();
  int minute = now.minute();
  int decimalTime = (hour * 100) + minute;

  if (!TIME_24_HOUR) {
    if (decimalTime > 1200) decimalTime -= 1200;
    if (decimalTime < 100 ) decimalTime += 1200;
  }
  return decimalTime;
}
