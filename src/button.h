

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <Arduino.h>

class Button
{
private:
    void (*onPressed)();
    uint8_t pin;
    bool released;

public:
    Button(uint8_t pin, void (*callback)());        
    void loop();        
    bool isPressed();
};

#endif
