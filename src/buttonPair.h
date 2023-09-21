
#ifndef __BUTTON_PAIR_H__
#define __BUTTON_PAIR_H__

#include <Arduino.h>

class ButtonPair
{
private:
    void (*onPressed)();
    uint8_t pinA;
    uint8_t pinB;
    bool released;

public:
    ButtonPair(uint8_t pinA, uint8_t pinB, void (*callback)());
    void loop();
};

#endif
