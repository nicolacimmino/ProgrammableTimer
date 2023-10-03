#include "display.h"

void Display::setup()
{
    this->display = new TM1637Display(DISPLAY_CLK, DISPLAY_DIO);

    display->setBrightness(7);
}

void Display::writeOnDisplay(uint8_t l0 = 0, uint8_t l1 = 0, uint8_t l2 = 0, uint8_t l3 = 0)
{
    if (millis() < this->freezeDisplayUntil)
    {
        return;
    }

    this->freezeDisplayUntil = 0;

    displayData[0] = l0;
    displayData[1] = l1;
    displayData[2] = l2;
    displayData[3] = l3;

    display->setSegments(displayData);
}

void Display::showNumber(uint16_t number)
{
    if (millis() < this->freezeDisplayUntil)
    {
        return;
    }

    this->freezeDisplayUntil = 0;

    display->showNumberDec(number);
}

void Display::printSeconds(uint16_t totalSeconds)
{
    // Show hours:minutes from 1h onwards
    if (totalSeconds >= 3600)
    {
        totalSeconds = totalSeconds / 60;
    }

    uint8_t minutes = floor(totalSeconds / 60.0);
    uint8_t seconds = totalSeconds % 60;

    this->writeOnDisplay(
        display->encodeDigit(minutes / 10),
        display->encodeDigit(minutes % 10) + (this->dotsOn() ? DISPLAY_COLON : 0),
        display->encodeDigit(seconds / 10),
        display->encodeDigit(seconds % 10));
}

void Display::freeze(uint8_t seconds)
{
    this->freezeDisplayUntil = millis() + (seconds * 1000);
}

bool Display::isFrozen()
{
    return this->freezeDisplayUntil > 0;
}

void Display::defrost()
{
    this->freezeDisplayUntil = 0;
}

void Display::loop()
{
}

bool Display::dotsOn()
{
    if (this->blinkDots && (millis() % 1000) < 500)
    {
        return false;
    }

    return true;
}
