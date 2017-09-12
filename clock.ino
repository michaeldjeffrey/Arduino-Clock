#include <Wire.h>
#include "libs/Adafruit_LEDBackpack.h"
#include "libs/Adafruit_GFX.h"
#include "libs/RTClib.h"

char *CURRENT_MODE_PROMPTS[] = {
  "Showing Time",
  "12 - 24 Selection",
  "Change Hours",
  "Change Minutes",
  "Change Brightness"
};

RTC_DS3231 RTC;
Adafruit_7segment disp = Adafruit_7segment();

bool IS_PM = false;
bool TIME_24_HOUR = false;
bool SHOW_COLON = false;

int COLON_BLINK_RATE = 500;  // half second
int MODE_TIMEOUT = 30000;    // 30 seconds

int BRIGHTNESS = 10;
int MODE = 0;
int TIME = 0;

long COLON_TIMER;
long MODE_TIMER;

void setup () {
  Serial.begin(9600);
  
  Wire.begin();
  RTC.begin();
  // Uncomment to set to time of compilation
  // RTC.adjust(DateTime(__DATE__, __TIME__));
  
  disp.begin(0x70);
  disp.setBrightness(BRIGHTNESS);
  
  COLON_TIMER = millis();
  MODE_TIMER = millis();

  Serial.println("Showing time.");  // Know when interactive.
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
      if (SHOW_COLON) disp.print((TIME_24_HOUR ? 24 : 12) * 100);
      break;
    case 2:  // Change Hours.
      if (SHOW_COLON) showHour();
      showMinute();
      break;
    case 3:  // Change Minutes.
      showHour();
      if (SHOW_COLON) showMinute();
      break;
    case 4:  // Change Brightness
      disp.print(BRIGHTNESS);
      break;
    default: // catch
      disp.print(MODE);
  }
  blinkColon();
  disp.writeDisplay();
}

void modeTimeout () {
  if ((millis() - MODE_TIMER) > MODE_TIMEOUT) {
    MODE = 0;
  }
}

void blinkColon () {
  if ((millis() - COLON_TIMER) > COLON_BLINK_RATE) {
    COLON_TIMER = millis();
    SHOW_COLON = !SHOW_COLON;
  }
  disp.drawColon(SHOW_COLON);
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
  disp.writeDigitNum(4, minute % 10, IS_PM);
}

void readOptions () {
  if (Serial.available()) {
    char ch = Serial.read();
    // reset modeTimer when interaction.
    if (ch) MODE_TIMER = millis();

    if (ch == '1') {
      incrementMode();
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

void incrementMode () {
  MODE_TIMER = millis();
  MODE++;
  if ( MODE > 4 ) MODE = 0;
  Serial.println(CURRENT_MODE_PROMPTS[MODE]);
}

void incrementHour () {
  DateTime now = RTC.now();
  TimeSpan oneHour = TimeSpan(60 * 60);
  RTC.adjust(now + oneHour);
}

void incrementMinute () {
  DateTime now = RTC.now();
  // Don't increase the hour when adjusting minutes.
  if (now.minute() == 59) {
    RTC.adjust(now - TimeSpan(60 * 59));
  } else {
    RTC.adjust(now + TimeSpan(60));
  }
}

void incrementBrightness () {
  BRIGHTNESS++;
  if (BRIGHTNESS > 15) {
    BRIGHTNESS = 0;
  }
  disp.setBrightness(BRIGHTNESS);
}

int getHour () {
  DateTime now = RTC.now();
  int hour = now.hour();

  IS_PM = hour > 12;

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

