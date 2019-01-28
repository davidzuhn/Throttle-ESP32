/* -*- c++ -*-
 *
 * Copyright © 2018-2019 Blue Knobby Systems Inc.
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 */

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
        digitalWrite(pin, 1);  // turn off the light at the start
    }

    bool check()
    {
        if (timer.hasPassed(PILOT_LIGHT_BLINK_TIME)) {
            timer.restart();

            int state = digitalRead(pin);
            state = !state;
            digitalWrite(pin, state);

            return true;
        }
        return false;
    }

  private:
    int pin;
    Chrono timer;
};
