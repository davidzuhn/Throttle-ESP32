/*
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

#include <FunctionalInterrupt.h>

#include "ESP32HW.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Hardware specification / definition for the ESP32 based BKT-0 throttle
//
//

// HW Data Processing Rates (measured in ms)

// update whether or not the throttle has moved
#define ACCELEROMETER_MOTION_READ_RATE  (1000/5)   // 5Hz

// how frequently we report (via the delegate) the speed & direction
#define SPEED_POT_REPORT_RATE           (1000/20)  // 20Hz

// internally, how often we read the potentiometer (to implement oversampling)
// read the speed pot 5 times for every report we make upstream
#define SPEED_POT_READ_RATE             (SPEED_POT_REPORT_RATE/5)

// how frequently we read the battery level
#define BATTERY_CHECK_READ_RATE         (2500)     // every 2.5 seconds


////////////////////////////////////////////////////////////////////////////////
//
// HW submodules that have individual addressing (I2C or SPI)
//

// I2C address of the 14 segment display.  Stick with the default address of
// 0x70 unless you've changed the address jumpers on the back of the
// display.
#define ALPHANUM_DISPLAY_ADDRESS (0x77)   // NOTE: jumpers soldered to change address

// I2C address of the SX1509 GPIO expander.
#define SX1509_I2C_ADDRESS       (0x3E)

// the INTR output pin of the SX1509 is attached to this pin on the ESP32
#define SX1509_INTR_PIN          (27)  // ESP32 A10

// I2C address of the LIS3DH accelerometer
#define ACCEL_I2C_ADDRESS        (0x18)

// the INTR output pin of the LIS3DH is attached to this pin on the ESP32
#define ACCEL_INTR_PIN           (39)  // ESP32 A3

// I2C address of the DRV2605 haptic feedback motor controller (this is not
// needed in code, included here for completeness in mapping the hardware
// pin assignments)
#define HAPTIC_I2C_ADDRESS       (DRV2605_ADDR)   // usually 0x5A

////////////////////////////////////////////////////////////////////////////////
//
// Pin assignments directly on the ESP32 module (currently on the Huzzah32 feather)
//

// Linear potentiometer used for speed control (analog value read via ADC)
#define SPEED_KNOB               (36)   // ESP32 A4

// Toggle Switch for direction selection.  At most ONE of these two will be
// connected to ground.  Using a CENTER OFF toggle, it is possible that
// neither of these will be connected to ground.
#define DIR_LEFT                 (15)
#define DIR_RIGHT                (33)

// Basic Pilot Light -- blink under direct ESP32 control
#define PILOT_LIGHT              (21)

// battery level is read through this pin (analog value read via ADC)
#define BATTERY_LEVEL_PIN        (36)   // A13

// do not report any changes to the current battery level until they exceed this
// value (in mV)
#define BATTERY_CHANGE_THRESHOLD (50)


// mini tactile buttons
#define OTA_BUTTON               (14)   // A6
#define EXTRA_BUTTON             (32)   // A7


// Serial1 console pins (not the USB serial port)
#define CONSOLE_TX               (17)
#define CONSOLE_RX               (16)

// whether to use the external TTL console (true) or the USB console (false)
#define TTL_CONSOLE              true

// GREEN TTL serial cable outermost, WHITE next, BLACK to ground


////////////////////////////////////////////////////////////////////////////////
//
// These are pin numbers on the SX1509 GPIO extender
//
#define STATUS_RED    (15)
#define STATUS_GREEN  (14)
#define STATUS_BLUE   (13)

#define BUTTON1    (7)
#define BUTTON2    (6)
#define BUTTON3    (5)
#define BUTTON4    (4)
#define BUTTON5    (3)
#define BUTTON6    (2)
#define BUTTON7    (1)
#define BUTTON8    (0)

#define BRAKE      (8)

#define LED1       (9)

//
////////////////////////////////////////////////////////////////////////////////


// LEDs are in an active-low configuration
#define GPIO_OFF (1)
#define GPIO_ON  (0)


// define ONLY one of the following time displays
//  TODO: make this a runtime configurable
#define TWELVE_HOUR_TIME      (1)
#define TWENTYFOUR_HOUR_TIME  (0)


//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


ESP32HW::ESP32HW() :
    numericDisplay(),
    accelerometer(),
    hapticMotorController(),
    gpio(),
    pilotLight(),
    statusLED(gpio, STATUS_RED, STATUS_GREEN, STATUS_BLUE),
    handle_gpio(false),
    handle_accelerometer(false),
    speedAccumulator(0),
    speedCount(0),
    previousTogglePosition(UnknownPosition),
    previousSpeedValue(0),
    penultimateSpeedValue(0),
    batteryCheck(),
    accelerometerCheck(),
    speedPotReadCheck(),
    speedCheck()
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, CONSOLE_TX, CONSOLE_RX);
#if TTL_CONSOLE
    console = &Serial1;
#else
    console = &Serial;
#endif
}


void
ESP32HW::resetStats()
{
    previousSpeedValue = penultimateSpeedValue = -1;
    previousTogglePosition = UnknownPosition;
}


bool
ESP32HW::begin()
{
    bool rv = true;

    // anything that's externally visible should be done before anything
    // else to eliminate noticeable flashes or other oddities that the user
    // can see
    rv &= setup_numeric_display();
    rv &= setup_pilot_light();

    rv &= setup_gpio();
    rv &= setup_accelerometer();
    rv &= setup_haptic_motor();
    rv &= setup_speed_direction();

    return rv;
}

// This method is called every time the accelerometer interrupt changes state
//
void
ESP32HW::accelerometer_isr()
{
    handle_accelerometer = true;
    accelerometer_value_at_intr = digitalRead(ACCEL_INTR_PIN);
}


// This method is called every time the SX1509 interrupt drops low
// (indicating that one or more inputs have been registered).
//
void
ESP32HW::gpio_isr()
{
    handle_gpio = true;
}


bool
ESP32HW::setup_pilot_light()
{
    pilotLight.begin(PILOT_LIGHT);
}


bool
ESP32HW::setup_gpio()
{
    // Call .begin(<address>) to initialize the SX1509. If it successfully
    // communicates, it'll return true.
    //
    if (!gpio.begin(SX1509_I2C_ADDRESS)) {
        return false;
    }

    gpio.debounceTime(16);

    statusLED.begin();

#ifdef BRAKE
    setup_button(BRAKE);
#endif
#ifdef BUTTON1
    setup_button(BUTTON1);
#endif
#ifdef BUTTON2
    setup_button(BUTTON2);
#endif
#ifdef BUTTON3
    setup_button(BUTTON3);
#endif
#ifdef BUTTON4
    setup_button(BUTTON4);
#endif
#ifdef BUTTON5
    setup_button(BUTTON5);
#endif
#ifdef BUTTON6
    setup_button(BUTTON6);
#endif
#ifdef BUTTON7
    setup_button(BUTTON7);
#endif
#ifdef BUTTON8
    setup_button(BUTTON8);
#endif

#ifdef LED1
    setup_led(LED1);
#endif
#ifdef LED2
    setup_led(LED2);
#endif
#ifdef LED3
    setup_led(LED3);
#endif
#ifdef LED4
    setup_led(LED4);
#endif
#ifdef LED5
    setup_led(LED5);
#endif
#ifdef LED6
    setup_led(LED6);
#endif
#ifdef LED7
    setup_led(LED7);
#endif

    attachInterrupt(SX1509_INTR_PIN, std::bind(&ESP32HW::gpio_isr, this), FALLING);

    console->println("gpio initialized");
    return true;
}


bool
ESP32HW::setup_accelerometer()
{
    if (! accelerometer.begin(ACCEL_I2C_ADDRESS)) {
        return false;
    }

    accelerometer.setRange(LIS3DH_RANGE_4_G);

    attachInterrupt(ACCEL_INTR_PIN, std::bind(&ESP32HW::accelerometer_isr, this), CHANGE);

    console->println("accelerometer initialized");
    return true;
}


bool
ESP32HW::setup_haptic_motor()
{
    hapticMotorController.begin();
    hapticMotorController.selectLibrary(1);
    hapticMotorController.setMode(DRV2605_MODE_INTTRIG);

    console->println("haptic controller initialized");
    return true;
}


bool
ESP32HW::setup_numeric_display()
{
    numericDisplay.begin(ALPHANUM_DISPLAY_ADDRESS);
    numericDisplay.clear();
    numericDisplay.writeDisplay();

    console->println("numeric display initialized");
    return true;
}


bool
ESP32HW::setup_speed_direction()
{
  pinMode(DIR_LEFT, INPUT_PULLUP);
  pinMode(DIR_RIGHT, INPUT_PULLUP);

  analogReadResolution(12);   // 0-4095, no matter what the hardware support
}


void
ESP32HW::setup_button(int pin)
{
    gpio.pinMode(pin, INPUT_PULLUP);
    gpio.enableInterrupt(pin, CHANGE);
    gpio.debouncePin(pin);
}


void
ESP32HW::setup_led(int pin)
{
    gpio.pinMode(pin, OUTPUT);
    gpio.digitalWrite(pin, GPIO_OFF);

}



void
ESP32HW::read_one_button(int intrStatus, int buttonPin, int funcNum, const char *name)
{
  if (intrStatus & (1 << buttonPin)) {
    int pressed = !gpio.digitalRead(buttonPin);
    //console->printf("%s %s\n", name, pressed ? "PRESSED" : "RELEASED");

    if (delegate) {
        delegate->functionButtonChanged(funcNum, pressed ? true : false);
    }
  }
}



/* This function gets called whenever the interrupt handler indicates that
 * we need to process a button change event from the SX1509.  Only those
 * buttons that have changed will be indicated in the interrupt source,
 * so we read the new value and send the function state accordingly.
 */
void
ESP32HW::read_buttons()
{
    if (!delegate) {
        return;
    }

    unsigned int intrStatus = gpio.interruptSource();
    // For debugging handiness, print the intrStatus variable.
    // console->print("button interrupt: "); console->print(intrStatus, BIN); console->println("");

    // Each bit in intStatus represents a single SX1509 I/O.  Check all thet
    // we know about each time we get notified of a change.

    // TODO: put the buttons into a map (name:pin)
#ifdef BRAKE
    read_one_button(intrStatus, BRAKE,   9, "BRAKE");
#endif

#ifdef BUTTON1
    read_one_button(intrStatus, BUTTON1, 0, "BUTTON1");
#endif
#ifdef BUTTON2
    read_one_button(intrStatus, BUTTON2, 1, "BUTTON2");
#endif
#ifdef BUTTON3
    read_one_button(intrStatus, BUTTON3, 2, "BUTTON3");
#endif
#ifdef BUTTON4
    read_one_button(intrStatus, BUTTON4, 3, "BUTTON4");
#endif
#ifdef BUTTON5
    read_one_button(intrStatus, BUTTON5, 4, "BUTTON5");
#endif
#ifdef BUTTON6
    read_one_button(intrStatus, BUTTON6, 5, "BUTTON6");
#endif
#ifdef BUTTON7
    read_one_button(intrStatus, BUTTON7, 6, "BUTTON7");
#endif
#ifdef BUTTON8
    read_one_button(intrStatus, BUTTON8, 7, "BUTTON8");
#endif
}



void
ESP32HW::read_speed_potentiometer()
{
    int rawSpeedValue = analogRead(SPEED_KNOB);

    // MAGIC NUMBERS:
    //   4095 is the max analog value for 12 bits
    //   126 is the maximum speed value for the WiThrottle protocol
    int reading = map(rawSpeedValue, 0, 4095, 0, 126);

    speedAccumulator += reading;
    speedCount += 1;
}


TogglePosition
ESP32HW::read_toggle_position()
{
    TogglePosition togglePosition = UnknownPosition; // This should never happen

    int d1 = !digitalRead(DIR_LEFT);
    int d2 = !digitalRead(DIR_RIGHT);

    if (!d1 && !d2) {
        togglePosition = CenterOff;
    }
    else if (d1) {
        togglePosition = Left;
    }
    else if (d2) {
        togglePosition = Right;
    }

    return togglePosition;
}



void
ESP32HW::report_speed()
{
    bool turnedToZero = false;
    bool turnedToMax = false;

    if (!delegate) {
        // no one is listening, just stop...
        return;
    }

    if (speedCount == 0) {
        // no data has been accumulated, so don't do anything...
        return;
    }

    bool speed_changed = false;
    bool toggle_position_changed = false;

    int speedValue = speedAccumulator / speedCount;

    TogglePosition togglePosition = read_toggle_position();

    // reset the accumulator
    speedAccumulator = speedCount = 0;

    if (speedValue == 0 && previousSpeedValue > 0) {
        turnedToZero = true;
    }
    if (speedValue == 126 && previousSpeedValue != 126) {
        turnedToMax = true;
    }

    if (togglePosition == CenterOff) {
        // center off position
        speedValue = 0;
        turnedToZero = turnedToMax = false;

        if (previousSpeedValue != 0) {
            previousSpeedValue = penultimateSpeedValue = 0;
            speed_changed = true;
        }
    }

    if (togglePosition != previousTogglePosition) {
        toggle_position_changed = true;
        previousTogglePosition = togglePosition;
    }

    if (speedValue != previousSpeedValue && speedValue != penultimateSpeedValue) {
        penultimateSpeedValue = previousSpeedValue;
        previousSpeedValue = speedValue;
        speed_changed = true;
    }

    if (toggle_position_changed) {
        //console->printf("toggle position changed: %d\n", togglePosition);
        delegate->togglePositionChanged(togglePosition);
    }
    if (speed_changed) {
        //console->printf("speed changed: %d, toggle position: %d\n", speedValue, togglePosition);
        delegate->speedChanged(speedValue, togglePosition);
    }

    if (turnedToZero) {
        triggerHapticMotor(26);
    }
    else if (turnedToMax) {
        triggerHapticMotor(17);
    }
}


// return the battery voltage, as reported in mV
int
ESP32HW::read_battery_level()
{
    int rawBatteryADCLevel = analogRead(BATTERY_LEVEL_PIN);

    // TODO: perform an ADC calibration before using the ADCs

    // there's a voltage divider on the input, so the raw reading is half of the battery
    // voltage level.   The nominal voltage is 3300 mV, with a calibration reference of
    // 1.100 V.   The full multiplication is done before dividing by the resolution of the
    // ADC, giving us something in the ballpark of the real voltage reading.

    int actualBatteryVoltage = int((float(rawBatteryADCLevel)*2.0 * 3300 * 1.100) / 4095.0);

    return actualBatteryVoltage;
}


void
ESP32HW::report_battery_level()
{
    if (!delegate) {
        return;
    }

    int battery_level = read_battery_level();
    static int last_battery_level = -1;

    if (abs(battery_level - last_battery_level) > BATTERY_CHANGE_THRESHOLD) {
        delegate->batteryLevelChanged(battery_level);

        last_battery_level = battery_level;
    }
}


void
ESP32HW::report_motion()
{
}



bool
ESP32HW::check()
{
    bool actionTaken = false;

    if (pilotLight.check()) {
        actionTaken = true;
    }


    if (handle_gpio) {
        read_buttons();
        handle_gpio = false;
    }

    if (handle_accelerometer) {
        console->printf("* ACCEL INTR ==> %d\n", accelerometer_value_at_intr);
        handle_accelerometer = false;
    }

    if (batteryCheck.hasPassed(BATTERY_CHECK_READ_RATE)) {
        batteryCheck.restart();
        actionTaken = true;

        report_battery_level();
    }

    if (accelerometerCheck.hasPassed(ACCELEROMETER_MOTION_READ_RATE)) {
        accelerometerCheck.restart();
        actionTaken = true;

        report_motion();
    }

    if (speedPotReadCheck.hasPassed(SPEED_POT_READ_RATE)) {
        speedPotReadCheck.restart();
        actionTaken = true;

        read_speed_potentiometer();
    }

    if (speedCheck.hasPassed(SPEED_POT_REPORT_RATE)) {
        speedCheck.restart();
        actionTaken = true;

        report_speed();
    }

    return actionTaken;
}



void
ESP32HW::setLight(int light, uint8_t state)
{
    state = 255 - state;   // the LED is active LOW, so state==0 should be OFF
    if (light == 9) { // corresponds to function number, TODO: fix this
        gpio.digitalWrite(LED1, state);
    }
}


void
ESP32HW::setRGB(int light, uint8_t red, uint8_t green, uint8_t blue)
{
    if (light==0) {
        statusLED.set(red, green, blue);
    }
}


void
ESP32HW::triggerHapticMotor(int mode)
{
    hapticMotorController.setWaveform(0, mode);
    hapticMotorController.setWaveform(1, 0);
    hapticMotorController.go();
}


void
ESP32HW::setTimeDisplay(int hour, int minute)
{
#if TWELVE_HOUR_TIME
    if (hour > 12) {
        hour = hour - 12;
    }
    if (hour == 0) {
        hour = 12;
    }

    numericDisplay.writeDigitAscii(0, hour >= 10 ? '1' : ' ');
#else
    if (hour >= 20) {
        numericDisplay.writeDigitAscii(0, '2');
    }
    else if (hour >= 10) {
        numericDisplay.writeDigitAscii(0, '1');
    }
    else {
        numericDisplay.writeDigitAscii(0, '0');
    }
#endif
    numericDisplay.writeDigitAscii(1, '0' + (hour % 10), true);

    numericDisplay.writeDigitAscii(2, '0' + (minute / 10));
    numericDisplay.writeDigitAscii(3, '0' + (minute % 10));
    numericDisplay.writeDisplay();
}


void
ESP32HW::setTimeStatus(TimeStatus status)
{
    uint8_t brightness;  // Backpack brightness ranges from 0..15
    switch (status) {
        case Running:
            brightness = 15;
            break;
        case Paused:
            brightness = 2;
            break;
        default:
            brightness = 0;
            break;
    }

    //console->printf("clock brightness set to %d\n", brightness);
    numericDisplay.setBrightness(brightness);
}



std::string
ESP32HW::getHWVersion()
{
    return "0revB.01";
}
