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

#include <SparkFunSX1509.h> // Include SX1509 library

class SX1509RGBLED
{
  public:
    SX1509RGBLED(SX1509& sx1509, int redPin, int greenPin, int bluePin, bool commonAnode = false):
        sx1509(sx1509),
        redPin(redPin),
        greenPin(greenPin),
        bluePin(bluePin),
        commonAnode(commonAnode)
    {
    }

    /* Configure the appropriate SX1509 pins and then turn LED OFF
     */
    void
    begin()
    {
        sx1509.pinMode(redPin, ANALOG_OUTPUT);
        sx1509.pinMode(greenPin, ANALOG_OUTPUT);
        sx1509.pinMode(bluePin, ANALOG_OUTPUT);
        set(0x00, 0x00, 0x00);
    }

    /* Set the LED brightness levels for each of the three colors
     *
     * 0 is off, 0xFF is full brightness
     *
     * LEDs configured with a common anode still use these same logical
     * values, but the actual values written must be handled separately
     * as physical 0xFF is off and 0x00 is full brightness
     */

    void
    set(uint8_t red, uint8_t green, uint8_t blue)
    {
        if (commonAnode) {
            red = 255 - red;
            green = 255 - green;
            blue = 255 - blue;
        }
        sx1509.analogWrite(redPin, red);
        sx1509.analogWrite(greenPin, green);
        sx1509.analogWrite(bluePin, blue);
    }

  private:
    SX1509& sx1509;
    int redPin;
    int greenPin;
    int bluePin;
    bool commonAnode;
};
