
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
#define MASK_BC 0b0110

#define LETTER_O 0b00111111
#define LETTER_F 0b01110001
#define LETTER_N 0b00110111
#define LETTER_L 0b00111000
#define LETTER_SPACE 0

uint8_t buttonPins[] = {PIN_BTN_A, PIN_BTN_B, PIN_BTN_C, PIN_BTN_D};

ButtonSet buttonSet;

Timer timer;

TM1637Display display(DISPLAY_CLK, DISPLAY_DIO);

uint8_t displayData[] = {0xff, 0xff, 0xff, 0xff};

bool mute = false;

unsigned long freezeDisplayUntil = 0;

void onButtonLongPressed(uint8_t pressedMask)
{
    unsigned long longPressStart = millis();

    while (buttonSet.isAnyPressed())
    {
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
        }

        refreshDisplay();

        delay(max(2, 200 / (1 + (millis() - longPressStart) / 1000)));
    }
}

void onButtonPressed(uint8_t pressedMask)
{
    if (freezeDisplayUntil > 0)
    {
        freezeDisplayUntil = 0;
        return;
    }

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
    case MASK_BC:
        onButtonBCPressed();
        break;
    default:
        break;
    }
}

void writeOnDisplay(uint8_t l0 = 0, uint8_t l1 = 0, uint8_t l2 = 0, uint8_t l3 = 0)
{
    displayData[0] = l0;
    displayData[1] = l1;
    displayData[2] = l2;
    displayData[3] = l3;

    display.setSegments(displayData);
}

void onButtonBCPressed()
{

    // See this article for an in-depth explanation.
    // https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
    // tl;dr: we switch the ADC to measure the internal 1.1v reference using Vcc as reference, the rest is simple math.

    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

    delay(2);
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC))
        ;

    analogReference(DEFAULT);

    long measuredVcc = 1125300L / (1 + (ADCL | (ADCH << 8)));

    if (measuredVcc < 2780)
    {
        writeOnDisplay(LETTER_SPACE, LETTER_SPACE, LETTER_L, LETTER_O);
    }
    else
    {
        uint8_t percentage = min(100, (measuredVcc - 2700) / 3); // Range 3000->2700, delta 300, /3 => 0-100%
        display.showNumberDec(percentage);
    }

    freezeDisplayUntil = millis() + 2000;
}

void onButtonACPressed()
{
    if (timer.isRunning())
    {
        timer.stop();
    }

    timer.setTime(0);
    digitalWrite(PIN_LED, LOW);
}

void blip()
{
    digitalWrite(PIN_BUZZER, HIGH);
    digitalWrite(PIN_LED, HIGH);
    delay(10);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED, LOW);
}

void onButtonABPressed()
{
    mute = !mute;

    if (mute)
    {
        writeOnDisplay(LETTER_O, LETTER_F, LETTER_F);
    }
    else
    {
        writeOnDisplay(LETTER_O, LETTER_N);
        blip();
    }

    freezeDisplayUntil = millis() + 2000;
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

void onTimerPreExpired()
{
    static unsigned long lastPreBeepTime = 0;

    if (millis() - lastPreBeepTime < 1000 || mute)
    {
        return;
    }

    blip();

    lastPreBeepTime = millis();
}

void onTimerExpired()
{
    printSeconds(0);
    timer.setTime(0);
    timer.setMode(MODE_COUNT_UP);
    timer.start();

    unsigned long buzzStart = millis();
    unsigned long buzzingTime = 0;
    while ((buzzingTime = (millis() - buzzStart)) <= 4000)
    {
        if ((buzzingTime / 500) % 2 == 0)
        {
            digitalWrite(PIN_BUZZER, mute ? LOW : HIGH);
            digitalWrite(PIN_LED, HIGH);
        }
        else
        {
            digitalWrite(PIN_BUZZER, LOW);
            digitalWrite(PIN_LED, LOW);
        }

        printSeconds(timer.getTimeElapsed());

        if (buttonSet.isAnyPressed())
        {
            while (buttonSet.isAnyPressed())
            {
                delay(10);
            }
            timer.stop();
            timer.setTime(0);
            digitalWrite(PIN_BUZZER, LOW);
            return;
        }
    }

    digitalWrite(PIN_LED, HIGH);
    digitalWrite(PIN_BUZZER, LOW);
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED, OUTPUT);

    timer.registerOnExpiredHandler(onTimerExpired, onTimerPreExpired);

    display.setBrightness(7);

    buttonSet.setup(buttonPins, 4, onButtonPressed, onButtonLongPressed);
}

void printSeconds(uint16_t totalSeconds)
{
    uint16_t offsetInSecond = millis() % 1000;

    // Show hours:minutes from 1h onwards
    if (totalSeconds >= 3600)
    {
        totalSeconds = totalSeconds / 60;
    }

    uint8_t minutes = floor(totalSeconds / 60.0);
    uint8_t seconds = totalSeconds % 60;

    bool dotOn = true;
    if (timer.isRunning() && !timer.isPaused() && offsetInSecond < 500)
    {
        dotOn = false;
    }

    displayData[0] = display.encodeDigit(minutes / 10);
    displayData[1] = display.encodeDigit(minutes % 10) + (dotOn ? DISPLAY_COLON : 0);
    displayData[2] = display.encodeDigit(seconds / 10);
    displayData[3] = display.encodeDigit(seconds % 10);

    display.setSegments(displayData);

    if (timer.isRunning() && !timer.isPaused() && offsetInSecond < 60)
    {
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        delay(50);
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    }
}

void refreshDisplay()
{
    if (millis() < freezeDisplayUntil)
    {
        return;
    }

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
}

void loop()
{
    buttonSet.loop();

    refreshDisplay();

    timer.loop();
}