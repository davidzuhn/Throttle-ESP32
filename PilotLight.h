// -*- c++ -*-
#pragma once

#include "Arduino.h"

#include <Chrono.h>

#define PILOT_LIGHT_BLINK_TIME (1000)

class PilotLight
{
  public:
    PilotLight():
        pin(-1),
        timer()
    {
    }

    void
    begin(int pin)
    {
        this->pin = pin;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, 0);
    }

    void check()
    {
        if (timer.hasPassed(PILOT_LIGHT_BLINK_TIME)) {
            timer.restart();

            int state = digitalRead(pin);
            state = !state;
            digitalWrite(pin, state);
        }
    }

  private:
    int pin;
    Chrono timer;
};
