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
#define UP_PIN 4
#define DOWN_PIN 5
#define DEBOUNCE_TIME 300

int num = 0000;


RTC_DS3231 rtc;
TM1637 display(DIO, CLK);

void modeSwitch(int&);

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
    byte flash = _BV(0);


    int hours = now.hour();
    int minutes = now.minute();
    int seconds = now.second();
    int timeToDisplay = (hours * 100) + minutes;

    if(digitalRead(SET_PIN) == LOW && millis() - lastDebounce > DEBOUNCE_TIME) {
        lastDebounce = millis();
        modeSwitch(settingMode);
    }

    if(seconds % 2 == 0 && millis() - lastColon > 1000) {
        flash = _BV(2);

        if(settingMode == SET_HOURS) {
            flash |= _BV(1);
        }

        if(settingMode == SET_MINUTES) {
            flash |= _BV(3);
        }
    }

    display.setDisplayToDecNumber(timeToDisplay, flash);
}

void modeSwitch(int& settingMode) {
    if(settingMode == SET_NONE) {
        settingMode = SET_HOURS;
    } else if(settingMode == SET_HOURS) {
        settingMode = SET_MINUTES;
    } else if(settingMode == SET_MINUTES) {
        settingMode = SET_NONE;
    }
}