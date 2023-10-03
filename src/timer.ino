

#include "timer.h"
#include "buttonSet.h"
#include "display.h"

#define PIN_BTN_A 4
#define PIN_BTN_B 7
#define PIN_BTN_C 8
#define PIN_BTN_D A0
#define PIN_BUZZER 3
#define PIN_LED A2

#define MASK_BUTTON_A 0b0001
#define MASK_BUTTON_B 0b0010
#define MASK_BUTTON_C 0b0100
#define MASK_BUTTON_D 0b1000
#define MASK_BUTTON_AB 0b0011
#define MASK_BUTTON_AC 0b0101
#define MASK_BUTTON_BC 0b0110

uint8_t buttonPins[] = {PIN_BTN_A, PIN_BTN_B, PIN_BTN_C, PIN_BTN_D};

ButtonSet buttonSet;

Timer timer;
Display display;

bool mute = false;

void onButtonLongPressed(uint8_t pressedMask)
{
    unsigned long longPressStart = millis();

    while (buttonSet.isAnyPressed())
    {
        switch (pressedMask)
        {
        case MASK_BUTTON_A:
            onButtonAPressed();
            break;
        case MASK_BUTTON_B:
            onButtonBPressed();
            break;
        case MASK_BUTTON_C:
            onButtonCPressed();
            break;
        }

        refreshDisplay();

        delay(max(2, 200 / (1 + (millis() - longPressStart) / 1000)));
    }
}

void onButtonPressed(uint8_t pressedMask)
{
    if (display.isFrozen() > 0)
    {
        display.defrost();
        return;
    }

    switch (pressedMask)
    {
    case MASK_BUTTON_A:
        onButtonAPressed();
        break;
    case MASK_BUTTON_B:
        onButtonBPressed();
        break;
    case MASK_BUTTON_C:
        onButtonCPressed();
        break;
    case MASK_BUTTON_D:
        onButtonDPressed();
        break;
    case MASK_BUTTON_AB:
        onButtonABPressed();
        break;
    case MASK_BUTTON_AC:
        onButtonACPressed();
        break;
    case MASK_BUTTON_BC:
        onButtonBCPressed();
        break;
    default:
        break;
    }
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
        display.writeOnDisplay(LETTER_SPACE, LETTER_SPACE, LETTER_L, LETTER_O);
    }
    else
    {
        uint8_t percentage = min(100, (measuredVcc - 2700) / 3); // Range 3000->2700, delta 300, /3 => 0-100%
        display.showNumber(percentage);
    }

    display.freeze(2 /* seconds */);
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
    digitalWrite(PIN_BUZZER, !mute ? HIGH : LOW);
    digitalWrite(PIN_LED, HIGH);
    delay(10);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED, LOW);
}

void onButtonABPressed()
{
    mute = !mute;

    (mute) ? display.writeOnDisplay(LETTER_O, LETTER_F, LETTER_F) : display.writeOnDisplay(LETTER_O, LETTER_N);

    blip();
    display.freeze(2 /* seconds */);
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
    display.printSeconds(0);
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

        display.printSeconds(timer.getTimeElapsed());

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

    display.setup();

    buttonSet.setup(buttonPins, 4, onButtonPressed, onButtonLongPressed);
}

void refreshDisplay()
{
    if (!timer.isRunning())
    {
        display.printSeconds(timer.getTimePreset());
    }
    else
    {
        if (timer.getMode() == MODE_COUNT_DOWN)
        {
            display.printSeconds(timer.getTimeRemaining());
        }
        else
        {
            display.printSeconds(timer.getTimeElapsed());
        }
    }
}

void loop()
{
    buttonSet.loop();

    refreshDisplay();
    display.loop();

    display.blinkDots = timer.isRunning() && !timer.isPaused();

    if (timer.isRunning() && !timer.isPaused() && (millis() % 1000) < 60)
    {
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        delay(50);
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    }

    timer.loop();
}