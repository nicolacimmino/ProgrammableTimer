#include "buttonSet.h"

void ButtonSet::setup(
    uint8_t *buttonPins,
    uint8_t size,
    void (*onPressed)(uint8_t pressedMask),
    void (*onLongPressed)(uint8_t pressedMask))
{
    this->onPressed = onPressed;
    this->onLongPressed = onLongPressed;
    this->buttonPins = buttonPins;
    this->buttons = size;

    for (uint8_t ix; ix < size; ix++)
    {
        pinMode(buttonPins[ix], INPUT_PULLUP);
    }
}

void ButtonSet::loop()
{
    if (!this->isAnyPressed())
    {
        this->waitingRelease = false;

        return;
    }

    if (this->waitingRelease)
    {
        return;
    }

    this->waitingRelease = true;

    uint8_t pressedMask = this->getPressedMask();
    unsigned long pressStart = millis();
    while (this->getPressedMask() >= pressedMask)
    {
        uint8_t newPressedMask = this->getPressedMask();

        if (newPressedMask > pressedMask)
        {
            pressStart = millis();
        }

        if (millis() - pressStart > 1000)
        {
            onLongPressed(pressedMask);
        }

        pressedMask = newPressedMask;
        delay(10);
    }

    onPressed(pressedMask);
}

bool ButtonSet::isPressed(uint8_t buttonIndex)
{
    return (digitalRead(this->buttonPins[buttonIndex]) == LOW);
}

bool ButtonSet::isAnyPressed()
{
    return this->getPressedMask() != 0;
}

uint8_t ButtonSet::getPressedMask()
{
    uint8_t pressedMask = 0;
    for (uint8_t ix = 0; ix < this->buttons; ix++)
    {
        if (this->isPressed(ix))
        {
            pressedMask |= 1 << ix;
        }
    }

    return pressedMask;
}