/*
 * Throttle ESP32
 *
 * This sketch implements a Model Railroad Throttle using the WiThrottle protocl.
 *
 * The hardware is use is an Adafruit Huzzah32 (ESP32 based), with additional
 * supporting hardware, including
 *   * SX1509 GPIO expander (up to 16 I/O lines)
 *   * RGB LED
 *   * 10K potentiometer (for Analog reads)
 *   * SPDT center off toggle switch
 *   * LIS3DH accelerometer
 *   * DRV2605 haptic feedback controller
 *   * 4 digit display

 * Huzzah32 - https://www.adafruit.com/product/3405
 * SX1509   - https://www.sparkfun.com/products/13601
 * LIS3DH   - https://www.adafruit.com/product/2809
 * DRV2605  - https://www.adafruit.com/product/2305
 * display  - https://www.adafruit.com/product/3106
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

#include "ThrottleController.h"

ThrottleController controller;


void setup()
{
    controller.begin();
}


void loop()
{
    controller.loop();
}
