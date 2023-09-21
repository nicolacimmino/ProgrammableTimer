#include "timer.h"

Timer::Timer()
{
    this->onExpired = NULL;
    this->mode = MODE_COUNT_DOWN;
}

void Timer::loop()
{
    if (!this->isRunning())
    {
        return;
    }

    if (mode == MODE_COUNT_DOWN && this->getTimeRemaining() == 0)
    {
        this->stop();
        if (this->onExpired != NULL)
        {
            this->onExpired();
        }
    }
}

void Timer::start()
{
    this->startTimeMs = millis();
}

void Timer::stop()
{
    this->startTimeMs = 0;
}

void Timer::setTime(uint16_t seconds)
{
    this->timeS = seconds;
}

void Timer::setMode(uint8_t mode)
{
    this->mode = mode;
}

void Timer::registerOnExpiredHandler(void (*callback)())
{
    this->onExpired = callback;
}

uint16_t Timer::getTimeRemaining()
{
    uint16_t timeElapsed = this->getTimeElapsed();

    if (timeElapsed > this->timeS)
    {
        return 0;
    }

    return this->timeS - timeElapsed;
}

uint16_t Timer::getTimeElapsed()
{
    if (!this->isRunning())
    {
        return 0;
    }

    return (millis() - this->startTimeMs) / 1000;
}

bool Timer::isRunning()
{
    return (this->startTimeMs != 0);
}