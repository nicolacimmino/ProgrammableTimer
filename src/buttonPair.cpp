#include "buttonPair.h"

ButtonPair::ButtonPair(uint8_t pinA, uint8_t pinB, void (*callback)())
{
    this->pinA = pinA;
    this->pinB = pinB;
    this->released = true;
    this->onPressed = callback;

    pinMode(this->pinA, INPUT_PULLUP);
    pinMode(this->pinB, INPUT_PULLUP);
}

void ButtonPair::loop()
{
    if (digitalRead(this->pinA) == LOW && digitalRead(this->pinB) == LOW && this->released)
    {
        if (this->onPressed != NULL)
        {
            this->onPressed();
        }
        this->released = false;
    }

    if (digitalRead(this->pinA) == HIGH || digitalRead(this->pinB) == HIGH)
    {
        this->released = true;
    }
}
