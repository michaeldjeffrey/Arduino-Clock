#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "RTClib.h"

RTC_DS3231 RTC;
Adafruit_7segment disp = Adafruit_7segment();

int COLON_BLINK_RATE = 500;  // half second
int MODE_TIMEOUT = 30000;    // 30 seconds
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
long modeTimer;

void setup () {
  Serial.begin(9600);
  
  Wire.begin();
  RTC.begin();
  // Uncomment to set to time of compilation
  // RTC.adjust(DateTime(__DATE__, __TIME__));
  
  disp.begin(0x70);
  disp.setBrightness(BRIGHTNESS);
  
  colonTimer = millis();
  modeTimer = millis();
}

void loop () {
  readOptions();
  modeTimeout();
  disp.clear();
  switch (MODE) {
  case 0:  // Display Time
    showHour();
    showMinute();
    break;
  case 1:  // 12 - 24 selection
    disp.print((TIME_24_HOUR ? 24 : 12) * 100);
    break;
  case 2:  // Change Hours.
    if (showColon) showHour();
    showMinute();
    break;
  case 3:  // Change Minutes.
    showHour();
    if (showColon) showMinute();
    break;
  case 4:  // Change Brightness
    disp.print(BRIGTHNESS);
    break;
  default: // catch
    disp.print(MODE);
  }
  blinkColon();
  disp.writeDisplay();
}

void modeTimeout () {
  if (millis() - modeTimer > MODE_TIMEOUT) {
    MODE = 0;
  }
}

void blinkColon () {
  if (millis() - colonTimer > COLON_BLINK_RATE) {
    colonTimer = millis();
    showColon = !showColon;
  }
  disp.drawColon(showColon);
}

void showHour () {
  int hour = getHour();

  // Only print first digit for 24 hour time
  // And greater than 10
  if (TIME_24_HOUR || (hour / 10) > 0) {
    disp.writeDigitNum(0, hour / 10);
  }
  disp.writeDigitNum(1, hour % 10);
}

void showMinute () {
  int minute = getMinute();
  disp.writeDigitNum(3, minute / 10);
  disp.writeDigitNum(4, minute % 10);
}

void readOptions () {
  if (Serial.available()) {
    char ch = Serial.read();
    // reset modeTimer when interaction.
    if (ch) modeTimer = millis();
    if (ch == 'r') {
      Serial.println("Resetting time");
      TIME = 0;
    }
    if (ch == '1') {
      cycleMode();
      Serial.print("Current Mode:");
      Serial.println(MODE);
    } else if (ch == '2') {
      switch (MODE) {
      case 1:
        TIME_24_HOUR = !TIME_24_HOUR;
        break;
      case 2:
        incrementHour();
        break;
      case 3:
        incrementMinute();
        break;
      case 4:
        incrementBrightness();
        break;
      default:
        break;
      }
    }
  }
}

void incrementHour () {}

void incrementMinute () {}

void incrementBrightness () {
  BRIGHTNESS++;
  if (BRIGHTNESS > 15) {
    BRIGHTNESS = 0;
  }
}

void cycleMode () {
  modeTimer = millis();
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

int getHour () {
  DateTime now = RTC.now();
  int hour = now.hour();

  if (!TIME_24_HOUR) {
    if (hour > 12) hour -=12;
    if (hour == 0) hour = 12;
  }
  return hour;
}

int getMinute () {
  DateTime now = RTC.now();
  return now.minute();
}

