// -*- c++ -*-
#pragma once

#include <Arduino.h>




typedef enum TogglePosition {
    Left = 0,
    Right = 1,
    CenterOff = 2,
    UnknownPosition = 3
} TogglePosition;


typedef enum TimeStatus {
    UnknownStatus = 0,
    Running = 1,
    Paused  = 2,    // Paused intends that the clock will resume shortly
    Stopped = 3,    // Stopped indicates that the clock is not expected to restart this session
    Inactive = 4
} TimeStatus;


class ThrottleHWDelegate
{
public:
    virtual void speedChanged(int newSpeed, TogglePosition togglePosition) {}
    virtual void togglePositionChanged(TogglePosition newPosition) {}
    virtual void throttleMoved() {}
    virtual void throttleFell() {}
    virtual void batteryLevelChanged(int batteryLevel) {}
    virtual void functionButtonChanged(int func, bool pressed) {}
};

class ThrottleHW
{
  public:
    const int LIGHT_OFF = 0;
    const int LIGHT_ON  = 0xFF;

    ThrottleHW() : delegate(NULL) {}

    // initialize the HW
    //  return true if the HW is okay, false otherwise
    virtual bool begin() = 0;

    // check the HW for new state, to be called VERY frequently
    //   changes to state will be notified via delegate callbacks
    //   return true if something changed, false otherwise
    virtual bool check() = 0;

    virtual void setLight(int light, uint8_t state) = 0;  // state: 0=off

    virtual void setRGB(int light, uint8_t red, uint8_t green, uint8_t blue) = 0;  // all colors: 0=off, !0=PWM

    virtual void triggerHapticMotor(int mode) = 0;

    virtual void setTimeDisplay(int hour, int minute) = 0;
    virtual void setTimeStatus(TimeStatus status) = 0;

    Stream* console;

    ThrottleHWDelegate* delegate;
};
