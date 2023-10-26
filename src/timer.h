
#ifndef __TIMER_H__
#define __TIMER_H__

#include <Arduino.h>

#define MODE_COUNT_UP 1
#define MODE_COUNT_DOWN 2

class Timer
{
private:
    unsigned long startTimeMs = 0;
    unsigned long pauseTimeMs = 0;
    void (*onExpired)();
    void (*onPreExpired)();
    uint8_t mode;
    uint16_t timeS;
    // Don't go past 59:59 for the countdown.
    uint16_t maxCountDownSeconds = 3599;
public:
    bool expired = false;
    Timer();
    void loop();
    void start();
    void pause();
    void stop();
    bool isRunning();
    bool isPaused();
    void setTime(uint16_t seconds);
    uint16_t getTimePreset();
    void setMode(uint8_t mode);
    uint8_t getMode();
    uint16_t getTimeRemaining();
    uint16_t getTimeElapsed();
    void registerOnExpiredHandler(void (*onExpired)(),void (*onPreExpired)());
};

#endif