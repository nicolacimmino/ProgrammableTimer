#ifndef __BUTTON_SET_H__
#define __BUTTON_SET_H__

#include <Arduino.h>

class ButtonSet
{
private:
    void (*onPressed)(uint8_t pressedMask);
    void (*onLongPressed)(uint8_t pressedMask);
    uint8_t *buttonPins;
    uint8_t buttons;
    bool waitingRelease = false;
    uint8_t getPressedMask();

public:
    void ButtonSet::setup(
        uint8_t *buttonPins, 
        uint8_t size, 
        void (*onPressed)(uint8_t pressedMask),
        void (*onLongPressed)(uint8_t pressedMask)
        );
    void loop();
    bool isPressed(uint8_t buttonIndex);
    bool isAnyPressed();
};

#endif