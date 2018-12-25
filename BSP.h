#pragma once

#ifdef __cplusplus
extern "C" {
#endif


// HW Data Processing Rates (measured in ms)

// update whether or not the throttle has moved
#define ACCELEROMETER_MOTION_READ_RATE  (1000/5)   // 5Hz

// how frequently we report (via the delegate) the speed & direction
#define SPEED_POT_REPORT_RATE           (1000/15)  // 15Hz

// internally, how often we read the potentiometer (to implement oversampling)
#define SPEED_POT_READ_RATE             (1000/(SPEED_POT_REPORT_RATE * 5))

// how frequently we read the battery level
#define BATTERY_CHECK_READ_RATE         (2500)


// I2C address of the 7 segment display.  Stick with the default address of
// 0x70 unless you've changed the address jumpers on the back of the
// display.
#define DISPLAY_ADDRESS          (0x70)

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

#define SPEED_KNOB               (A4)

// Toggle Switch for direction selection.  At most ONE of these two will be
// connected to ground.  Using a CENTER OFF toggle, it is possible that
// neither of these will be connected to ground.
#define DIR_LEFT                 (15)
#define DIR_RIGHT                (33)

// Basic Pilot Light -- blink under direct ESP32 control
#define PILOT_LIGHT              (21)




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




#ifdef __cplusplus
}
#endif
