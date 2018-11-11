#pragma once

#include <string>


#include <Arduino.h>

#include <Chrono.h>
#include <Time.h>
#include <TimeLib.h>

#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <SparkFunSX1509.h>

#include "WiThrottle.h"
#include "RGBLED.h"



// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   (0x70)

// I2C address of the SX1509 GPIO expander.
#define SX1509_ADDRESS  (0x3E)

// the INTR output pin of the SX1509 is attached to this pin on the ESP32
#define SX1509_INTR_PIN (A10)


////////////////////////////////////////////////////////////////////////////////
//
// These are pin numbers on the SX1509 GPIO extender
//
#define RGB_RED (15)
#define RGB_GREEN (14)
#define RGB_BLUE (13)
#define BUTTON1 (9)
#define BUTTON2 (10)
#define BUTTON3 (11)
#define BUTTON4 (12)
#define BUTTON5 (7)
#define BRAKE (8)

#define LED1 (0)
#define LED2 (1)
#define LED3 (2)
#define LED4 (3)
#define LED5 (4)
#define LED6 (5)
#define LED7 (6)
////////////////////////////////////////////////////////////////////////////////

#define SPEED_KNOB (A4)
#define DIR_LEFT (15)
#define DIR_RIGHT (33)


typedef enum TogglePosition {
  Left = 0,
  Right = 1,
  CenterOff = 2,
  Unknown = 3
} TogglePosition;



class ThrottleController: public WiThrottleDelegate
{
  public:
    ThrottleController();

    bool begin();

    void loop();

    // WiThrottleDelegate methods
    void receivedDirection(Direction dir);
    void receivedFastTime(uint32_t time);
    void receivedFastTimeRate(double rate);
    void receivedFunctionState(uint8_t func, bool state);
    void receivedSpeed(int speed);
    void receivedSpeedSteps(int steps);
    void receivedTrackPower(TrackPower state);
    void receivedVersion(String version);
    void receivedWebPort(int port);

    // WiFi callback methods
    void wifiOnConnect();
    void wifiOnDisconnect();
    void wifiEvent(WiFiEvent_t event);




  private:
    Direction directionFromTogglePosition(TogglePosition position);
    void readButton(int intrStatus, int buttonPin, int funcNum, const char *name);
    void readButtons();
    void readSpeed();
    void readSpeedPot();
    TogglePosition readTogglePosition();
    void setupButtonPin(int pin);
    void sx1509_isr();
    void updateFastTimeDisplay();


    WiFiClient client;
    Adafruit_7segment clockDisplay;
    SX1509RGBLED statusLED;
    SX1509 sx1509;
    WiThrottle wiThrottle;

    TogglePosition previousTogglePosition;

    int speedAccumulator;
    int speedCount;

    int previousSpeedValue;
    int penultimateSpeedValue;

    bool handleSX1509Interrupt;

    Chrono speedPotRead;
    Chrono speedCheck;

    bool wifiConnected;

    std::string ssid;
    std::string password;
    std::string host;
    int port;


};
