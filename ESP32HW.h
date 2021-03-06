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

#include "ThrottleHW.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include <SparkFunSX1509.h>

#include <Adafruit_DRV2605.h>

#include "PilotLight.h"
#include "RGBLED.h"

class ESP32HW : public ThrottleHW
{
  public:
    ESP32HW();

    bool begin();

    bool check();

    void setLight(int light, uint8_t state);  // state: 0=off

    void setRGB(int light, uint8_t red, uint8_t green, uint8_t blue);  // all colors: 0=off, !0=PWM

    void triggerHapticMotor(int mode);

    void setTimeDisplay(int hour, int minute);

    void setTimeStatus(TimeStatus status);

    void resetStats();

    std::string getHWVersion();


private:
    // HW interfaces
    Adafruit_AlphaNum4 numericDisplay;
    Adafruit_LIS3DH    accelerometer;
    Adafruit_DRV2605   hapticMotorController;
    SX1509             gpio;

    PilotLight         pilotLight;
    SX1509RGBLED       statusLED;

    // methods
    void               accelerometer_isr();
    void               gpio_isr();

    bool               setup_pilot_light();
    bool               setup_gpio();
    bool               setup_accelerometer();
    bool               setup_haptic_motor();
    bool               setup_numeric_display();
    bool               setup_speed_direction();

    void               setup_led(int pin);
    void               setup_button(int pin);

    void               read_one_button(int intrStatus, int buttonPin, int funcNum, const char *name);
    void               read_buttons();

    TogglePosition     read_toggle_position();
    void               read_speed_potentiometer();
    void               report_speed();

    int                read_battery_level();
    void               report_battery_level();
    void               report_motion();

    // internal state
    bool               handle_gpio;
    bool               handle_accelerometer;

    int                accelerometer_value_at_intr;

    int                speedAccumulator;
    int                speedCount;

    TogglePosition     previousTogglePosition;
    int                previousSpeedValue;
    int                penultimateSpeedValue;

    // internal timer helpers
    Chrono             batteryCheck;
    Chrono             accelerometerCheck;
    Chrono             speedPotReadCheck;
    Chrono             speedCheck;

};
