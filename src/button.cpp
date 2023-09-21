#include "button.h"

Button::Button(uint8_t pin, void (*callback)())
{
    this->pin = pin;
    this->released = true;
    this->onPressed = callback;

    pinMode(this->pin, INPUT_PULLUP);
}

void Button::loop()
{
    if (digitalRead(this->pin) == LOW && this->released)
    {
        if (this->onPressed != NULL)
        {
            this->onPressed();
        }
        this->released = false;
    }

    if (digitalRead(this->pin) == HIGH)
    {
        this->released = true;
    }
}
