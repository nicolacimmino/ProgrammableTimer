
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <TM1637Display.h>
#include <Arduino.h>

#define DISPLAY_CLK 6
#define DISPLAY_DIO 5
#define DISPLAY_COLON 128

#define LETTER_O 0b00111111
#define LETTER_F 0b01110001
#define LETTER_N 0b00110111
#define LETTER_L 0b00111000
#define LETTER_SPACE 0

class Display
{
private:
    TM1637Display *display;
    uint8_t displayData[4];
    unsigned long freezeDisplayUntil = 0;
public:    
    void setup();
    void loop();
    void writeOnDisplay(uint8_t l0 = 0, uint8_t l1 = 0, uint8_t l2 = 0, uint8_t l3 = 0);
    void showNumber(uint16_t number);
    void printSeconds(uint16_t totalSeconds, bool dotOn);
    void freeze(uint8_t seconds);
    bool isFrozen();
    void defrost();
};

#endif