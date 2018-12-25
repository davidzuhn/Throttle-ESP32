#include <FunctionalInterrupt.h>

#include "ESP32HW.h"

#include "BSP.h"

// LEDs are in an active-low configuration
#define GPIO_OFF (1)
#define GPIO_ON  (0)


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


void
ESP32HW::accelerometer_isr()
{
    handle_accelerometer = true;
    accelerometer_value_at_intr = digitalRead(ACCEL_INTR_PIN);
}


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
    Serial.println("LED1 setup");
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

    Serial.println("gpio initialized");
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

    Serial.println("accelerometer initialized");
    return true;
}


bool
ESP32HW::setup_haptic_motor()
{
    hapticMotorController.begin();
    hapticMotorController.selectLibrary(1);
    hapticMotorController.setMode(DRV2605_MODE_INTTRIG);

    Serial.println("haptic controller initialized");
    return true;
}


bool
ESP32HW::setup_numeric_display()
{
    numericDisplay.begin(ALPHANUM_DISPLAY_ADDRESS);
    numericDisplay.clear();
    numericDisplay.writeDisplay();

    Serial.println("numeric display initialized");
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
    Serial.printf("%s %s\n", name, pressed ? "PRESSED" : "RELEASED");

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
    // For debugging handiness, print the intStatus variable.
    Serial.print("button interrupt: "); Serial.print(intrStatus, BIN); Serial.println("");

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

    if (togglePosition == CenterOff) {
        // center off position
        speedValue = 0;
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
        Serial.printf("toggle position changed: %d\n", togglePosition);
        delegate->togglePositionChanged(togglePosition);
    }
    if (speed_changed) {
        Serial.printf("speed changed: %d, toggle position: %d\n", speedValue, togglePosition);
        delegate->speedChanged(speedValue, togglePosition);
    }
}


void
ESP32HW::report_battery_level()
{
    if (!delegate) {
        return;
    }

    int battery_level = 100;
    static int last_battery_level = -1;

    if (battery_level != last_battery_level) {
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
        Serial.printf("* ACCEL INTR ==> %d\n", accelerometer_value_at_intr);
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
}


void
ESP32HW::setRGB(int light, uint8_t red, uint8_t green, uint8_t blue)
{
}


void
ESP32HW::triggerHapticMotor(int mode)
{
}


void
ESP32HW::setTimeDisplay(int hour, int minute)
{
}


void
ESP32HW::setTimeStatus(TimeStatus status)
{
}
