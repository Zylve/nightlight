#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <TM1637.h>

#define CLK 2
#define DIO 3

#define SET_NONE 0
#define SET_HOURS 1
#define SET_MINUTES 2

#define SET_PIN 6
#define UP_PIN 5
#define DOWN_PIN 4
#define DEBOUNCE_TIME 300
#define FLASH_TIME 500

const uint8_t digitToSegment[] = {
    // XGFEDCBA
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // B
    0b00111001, // C
    0b01011110, // D
    0b01111001, // E
    0b01110001 // F
};

int num = 0000;


RTC_DS3231 rtc;
TM1637 display(DIO, CLK);

void modeSwitch(int&, int&, int&);

void setup() {
    display.clearDisplay();
    display.setDisplayToDecNumber(0000, 0);

    pinMode(SET_PIN, INPUT_PULLUP);
    pinMode(UP_PIN, INPUT_PULLUP);
    pinMode(DOWN_PIN, INPUT_PULLUP);

    Wire.begin();
    rtc.begin();
}

void loop() {
    DateTime now = rtc.now();

    static int settingMode = SET_NONE;
    static int setHour = now.hour();
    static int setMinutes = now.minute();
    static unsigned long lastDebounce;
    static unsigned long lastColon;
    bool leadingZero = true;
    byte flash = _BV(0);


    int hours = now.hour();
    int minutes = now.minute();
    int seconds = now.second();
    int timeToDisplay = (hours * 100) + minutes;

    if(digitalRead(SET_PIN) == LOW && millis() - lastDebounce > DEBOUNCE_TIME) {
        lastDebounce = millis();
        modeSwitch(settingMode, setHour, setMinutes);
    }

    if(seconds % 2 == 0 && millis() - lastColon > FLASH_TIME) {
        flash = _BV(2);

        if(settingMode == SET_HOURS) {
            timeToDisplay = minutes;
            leadingZero = false;
        }

        if(settingMode == SET_MINUTES) {
            timeToDisplay = (setHour * 100);
        }

    } else {
        if(settingMode == SET_HOURS) {
            timeToDisplay = (setHour * 100) + minutes;
        }

        if(settingMode == SET_MINUTES) {
            timeToDisplay = (hours * 100) + setMinutes;
        }
    }

    if(settingMode == SET_HOURS) {
        if(digitalRead(UP_PIN) == LOW && millis() - lastDebounce > DEBOUNCE_TIME) {
            lastDebounce = millis();
            setHour++;
            if(setHour > 23) {
                setHour = 0;
            }
        }

        if(digitalRead(DOWN_PIN) == LOW && millis() - lastDebounce > DEBOUNCE_TIME) {
            lastDebounce = millis();
            setHour--;
            if(setHour < 0) {
                setHour = 23;
            }
        }

        if(seconds % 2 == 0 && millis() - lastColon > FLASH_TIME) {
            display.sendChar(0, 0x0, 0);
            display.sendChar(1, 0x0, true);
            display.sendChar(2, digitToSegment[minutes / 10], 0);
            display.sendChar(3, digitToSegment[minutes % 10], 0);
        } else {
            display.sendChar(0, digitToSegment[setHour / 10], 0);
            display.sendChar(1, digitToSegment[setHour % 10], 0);
            display.sendChar(2, digitToSegment[minutes / 10], 0);
            display.sendChar(3, digitToSegment[minutes % 10], 0);
        }
    } else if(settingMode == SET_MINUTES) {
        if(digitalRead(UP_PIN) == LOW && millis() - lastDebounce > DEBOUNCE_TIME) {
            lastDebounce = millis();
            setMinutes++;
            if(setMinutes > 59) {
                setMinutes = 0;
            }
        }

        if(digitalRead(DOWN_PIN) == LOW && millis() - lastDebounce > DEBOUNCE_TIME) {
            lastDebounce = millis();
            setMinutes--;
            if(setMinutes < 0) {
                setMinutes = 59;
            }
        }

        if(seconds % 2 == 0 && millis() - lastColon > FLASH_TIME) {
            display.sendChar(0, digitToSegment[hours / 10], 0);
            display.sendChar(1, digitToSegment[hours % 10], true);
            display.sendChar(2, 0x0, 0);
            display.sendChar(3, 0x0, 0);
        } else {
            display.sendChar(0, digitToSegment[hours / 10], 0);
            display.sendChar(1, digitToSegment[hours % 10], 0);
            display.sendChar(2, digitToSegment[setMinutes / 10], 0);
            display.sendChar(3, digitToSegment[setMinutes % 10], 0);
        }

    } else {
        display.setDisplayToDecNumber(timeToDisplay, flash, leadingZero);
    }
}

void modeSwitch(int& settingMode, int& setHour, int& setMinutes) {
    if(settingMode == SET_NONE) {
        settingMode = SET_HOURS;
    } else if(settingMode == SET_HOURS) {
        settingMode = SET_MINUTES;
        rtc.adjust(DateTime(2000, 1, 1, setHour, rtc.now().minute(), 0));
    } else if(settingMode == SET_MINUTES) {
        settingMode = SET_NONE;
        rtc.adjust(DateTime(2000, 1, 1, rtc.now().hour(), setMinutes, 0));
    }
}