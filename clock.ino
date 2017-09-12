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

TimeSpan ONE_MINUTE = TimeSpan(60);
TimeSpan ONE_HOUR   = TimeSpan(60 * 60);

bool IS_PM        = false;
bool TIME_24_HOUR = false;
bool SHOW_COLON   = false;

int COLON_BLINK_RATE = 500;  // half second
int MODE_TIMEOUT     = 30000;    // 30 seconds

int BRIGHTNESS = 10;
int MODE       = 0;
int HOUR       = 0;
int MINUTE     = 0;

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
  getTime();
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
  // Only print first digit for 24 hour time
  // And greater than 10
  if (TIME_24_HOUR || (HOUR / 10) > 0) {
    disp.writeDigitNum(0, HOUR / 10);
  }
  disp.writeDigitNum(1, HOUR % 10);
}

void showMinute () {
  disp.writeDigitNum(3, MINUTE / 10);
  disp.writeDigitNum(4, MINUTE % 10, IS_PM);
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
      }
    }
  }
}

void incrementMode () {
  MODE_TIMER = millis();
  MODE += 1;
  if (MODE > 4) MODE = 0;
  Serial.println(CURRENT_MODE_PROMPTS[MODE]);
}

void incrementHour () {
  RTC.adjust(RTC.now() + ONE_HOUR);
}

void incrementMinute () {
  if (MINUTE == 59) {
    // Don't increase the hour when adjusting minutes.
    RTC.adjust(RTC.now() - (ONE_HOUR - ONE_MINUTE));
  } else {
    RTC.adjust(RTC.now() + ONE_MINUTE);
  }
}

void incrementBrightness () {
  BRIGHTNESS += 1;
  if (BRIGHTNESS > 15) BRIGHTNESS = 0;
  disp.setBrightness(BRIGHTNESS);
}

void getTime () {
  DateTime now = RTC.now();
  HOUR = now.hour();
  MINUTE = now.minute();

  IS_PM = HOUR > 12;

  if (!TIME_24_HOUR) {
    if (HOUR > 12) HOUR -= 12;
    if (HOUR == 0) HOUR  = 12;
  }
}


