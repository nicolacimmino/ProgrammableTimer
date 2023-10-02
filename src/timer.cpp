#include "timer.h"

Timer::Timer()
{
    this->onExpired = NULL;
    this->onPreExpired = NULL;
    this->mode = MODE_COUNT_DOWN;
}

bool Timer::isPaused()
{
    return this->pauseTimeMs != 0;
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

    if (mode == MODE_COUNT_DOWN && this->getTimeRemaining() < 6)
    {
        if (this->onPreExpired != NULL)
        {
            onPreExpired();
        }
    }
}

void Timer::start()
{
    if (this->isPaused())
    {
        this->startTimeMs += millis() - this->pauseTimeMs;
        this->pauseTimeMs = 0;

        return;
    }

    this->startTimeMs = millis();
    this->pauseTimeMs = 0;
}

void Timer::stop()
{
    this->startTimeMs = 0;
    this->pauseTimeMs = 0;
}

void Timer::pause()
{
    this->pauseTimeMs = millis();
}

void Timer::setTime(uint16_t seconds)
{
    if (seconds > this->maxCountDownSeconds)
    {
        return;
    }

    this->timeS = seconds;
}

void Timer::setMode(uint8_t mode)
{
    this->mode = mode;
}

void Timer::registerOnExpiredHandler(void (*onExpired)(), void (*onPreExpired)())
{
    this->onExpired = onExpired;
    this->onPreExpired = onPreExpired;
}

uint16_t Timer::getTimeRemaining()
{
    if (this->mode == MODE_COUNT_UP)
    {
        return 0;
    }

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

    unsigned long timeElapsed = (millis() - this->startTimeMs);

    if (this->isPaused())
    {
        timeElapsed -= millis() - this->pauseTimeMs;
    }

    return timeElapsed / 1000;
}

bool Timer::isRunning()
{
    return (this->startTimeMs != 0);
}

uint16_t Timer::getTimePreset()
{
    return this->timeS;
}

uint8_t Timer::getMode()
{
    return this->mode;
}