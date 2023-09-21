
#include <Wire.h>
#include <TM1650.h>
#include "timer.h"
#include "button.h"

#define PIN_BTN_MIN 4
#define PIN_BTN_SEC_10 7
#define PIN_BTN_SEC_1 8
#define PIN_BTN_FUNC A0
#define PIN_BUZZER 3

Button buttonMinutes(PIN_BTN_MIN, onButtonMinutesPressed);
Button buttonSeconds10(PIN_BTN_SEC_10, onButtonSeconds10Pressed);
Button buttonSeconds1(PIN_BTN_SEC_1, onButtonSeconds1Pressed);
Button buttonFunc(PIN_BTN_FUNC, onButtonFuncPressed);

Timer timer;

TM1650 display;

uint16_t presetSeconds = 0;
bool running = false;

void onButtonMinutesPressed()
{
    if (!running)
    {
        presetSeconds += 60;
    }
}

void onButtonSeconds10Pressed()
{
    if (!running)
    {
        presetSeconds += 10;
    }
}

void onButtonSeconds1Pressed()
{
    if (!running)
    {
        presetSeconds += 1;
    }
}
void onButtonFuncPressed()
{
    if (!running)
    {
        running = true;
        timer.setTime(presetSeconds);
        timer.start();
    }
    else
    {
        running = false;
        timer.stop();
    }
}

void onTimerExpired()
{
    running = false;
    printSeconds(0);
    for (uint8_t ix = 0; ix < 4; ix++)
    {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(200);
        digitalWrite(PIN_BUZZER, LOW);
        delay(500);
    }
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    display.init();
    display.displayOn();
    display.setBrightness(TM1650_MIN_BRIGHT);
    pinMode(PIN_BUZZER, OUTPUT);
    timer.registerOnExpiredHandler(onTimerExpired);
}

void printSeconds(uint16_t totalSeconds)
{
    uint8_t minutes = floor(totalSeconds / 60.0);
    uint8_t seconds = totalSeconds % 60;

    char buf[5];

    buf[0] = (char)('0' + (minutes / 10));
    buf[1] = (char)('0' + (minutes % 10)) | 128;
    buf[2] = (char)('0' + (seconds / 10));
    buf[3] = (char)('0' + (seconds % 10));
    buf[4] = 0;

    display.displayString(buf);
}

void loop()
{    
    buttonMinutes.loop();
    buttonSeconds10.loop();
    buttonSeconds1.loop();
    buttonFunc.loop();

    if (!running)
    {
        printSeconds(presetSeconds);
    }
    else
    {
        printSeconds(timer.getTimeRemaining());
    }

    timer.loop();
}