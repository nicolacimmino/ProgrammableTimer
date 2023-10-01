
#include <TM1637Display.h>
#include "timer.h"
#include "buttonSet.h"

#define PIN_BTN_A 4
#define PIN_BTN_B 7
#define PIN_BTN_C 8
#define PIN_BTN_D A0
#define PIN_BUZZER 3
#define PIN_LED A2

#define DISPLAY_CLK 6
#define DISPLAY_DIO 5
#define DISPLAY_COLON 128

#define MASK_A 0b0001
#define MASK_B 0b0010
#define MASK_C 0b0100
#define MASK_D 0b1000
#define MASK_AB 0b0011
#define MASK_AC 0b0101

uint8_t buttonPins[] = {PIN_BTN_A, PIN_BTN_B, PIN_BTN_C, PIN_BTN_D};

ButtonSet buttonSet;

Timer timer;

TM1637Display display(DISPLAY_CLK, DISPLAY_DIO);

uint8_t displayData[] = {0xff, 0xff, 0xff, 0xff};

bool mute = false;
bool buttonPairAction = false;

void onButtonPressed(uint8_t pressedMask)
{
    Serial.println(pressedMask, 2);
    switch (pressedMask)
    {
    case MASK_A:
        onButtonAPressed();
        break;
    case MASK_B:
        onButtonBPressed();
        break;
    case MASK_C:
        onButtonCPressed();
        break;
    case MASK_D:
        onButtonDPressed();
        break;
    case MASK_AB:
        onButtonABPressed();
        break;
    case MASK_AC:
        onButtonACPressed();
        break;
    default:
        break;
    }
}

void onButtonACPressed()
{
    if (timer.isRunning())
    {
        timer.stop();
    }

    timer.setTime(0);

    buttonPairAction = true;
}

void onButtonABPressed()
{
    mute = !mute;

    if (mute)
    {
        displayData[0] = 0b00111111; // O
        displayData[1] = 0b01110001; // F
        displayData[2] = 0b01110001; // F
        displayData[3] = 0b00000000;
    }
    else
    {
        displayData[0] = 0b00111111; // O
        displayData[1] = 0b00110111; // N
        displayData[2] = 0b00000000;
        displayData[3] = 0b00000000;
    }

    display.setSegments(displayData);

    if (!mute)
    {
        digitalWrite(PIN_BUZZER, HIGH);
        digitalWrite(PIN_LED, HIGH);
        delay(10);
        digitalWrite(PIN_BUZZER, LOW);
        digitalWrite(PIN_LED, LOW);
    }

    delay(1000);

    displayData[0] = 0b00000000;
    displayData[1] = 0b00000000;
    displayData[2] = 0b00000000;
    displayData[3] = 0b00000000;
    display.setSegments(displayData);

    buttonPairAction = true;
}

void onButtonAPressed()
{
    if (!timer.isRunning())
    {
        timer.setTime(timer.getTimePreset() + 60);
    }
}

void onButtonBPressed()
{
    if (!timer.isRunning())
    {
        timer.setTime(timer.getTimePreset() + 10);
    }
}

void onButtonCPressed()
{
    if (!timer.isRunning())
    {
        timer.setTime(timer.getTimePreset() + 1);
    }
}
void onButtonDPressed()
{
    if (!timer.isRunning())
    {
        timer.setMode(timer.getTimePreset() > 0 ? MODE_COUNT_DOWN : MODE_COUNT_UP);
        timer.start();
    }
    else
    {
        if (!timer.isPaused())
        {
            timer.pause();
        }
        else
        {
            timer.start();
        }
    }
}

void onTimerExpired()
{
    printSeconds(0);
    timer.setTime(0);

    unsigned long buzzStart = millis();
    unsigned long buzzingTime = 0;
    while ((buzzingTime = (millis() - buzzStart)) <= 4000)
    {
        if ((buzzingTime / 1000) % 2 == 0)
        {
            digitalWrite(PIN_BUZZER, mute ? LOW : HIGH);
            digitalWrite(PIN_LED, HIGH);
            delay(10);
            digitalWrite(PIN_BUZZER, LOW);
            digitalWrite(PIN_LED, LOW);
            delay(20);
        }

        if (buttonSet.isAnyPressed())
        {
            while (buttonSet.isAnyPressed())
            {
                delay(10);
            }
            return;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED, OUTPUT);

    timer.registerOnExpiredHandler(onTimerExpired);

    display.setBrightness(0x0f);

    buttonSet.setup(buttonPins, 4, onButtonPressed);
}

void printSeconds(uint16_t totalSeconds)
{
    uint8_t minutes = floor(totalSeconds / 60.0);
    uint8_t seconds = totalSeconds % 60;

    bool dotOn = true;
    if (timer.isRunning() && millis() % 1000 < 500)
    {
        dotOn = false;
    }
    if (timer.isPaused() && millis() % 1000 < 900)
    {
        dotOn = false;
    }

    displayData[0] = display.encodeDigit(minutes / 10);
    displayData[1] = display.encodeDigit(minutes % 10) + (dotOn ? DISPLAY_COLON : 0);
    displayData[2] = display.encodeDigit(seconds / 10);
    displayData[3] = display.encodeDigit(seconds % 10);

    display.setSegments(displayData);
}

void loop()
{
    buttonSet.loop();

    if (!timer.isRunning())
    {
        printSeconds(timer.getTimePreset());
    }
    else
    {
        if (timer.getMode() == MODE_COUNT_DOWN)
        {
            printSeconds(timer.getTimeRemaining());
        }
        else
        {
            printSeconds(timer.getTimeElapsed());
        }
    }

    timer.loop();
}