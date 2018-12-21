// -*- c++ -*-
#pragma once

#include <string>

#include "Arduino.h"

#include <Chrono.h>
#include <Time.h>
#include <TimeLib.h>

#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include <SparkFunSX1509.h>

#include "WiThrottle.h"
#include "WifiInfo.h"
#include "RGBLED.h"
#include "PilotLight.h"

#include "ThrottleData.h"
#include "BLEThrottle.h"


#define ADAFRUIT_ALPHANUM 1


typedef enum TogglePosition {
  Left = 0,
  Right = 1,
  CenterOff = 2,
  Unknown = 3
} TogglePosition;



class ThrottleController:
    public WiThrottleDelegate,
    public WifiInfoDelegate
{
  public:
    ThrottleController(ThrottleData& flashData);

    bool begin();

    void loop();

    void wifiCommandReceived(std::string command);

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
    void setupLEDPin(int pin);
    void setupSX1509();
    void sx1509_isr();
    void updateFastTimeDisplay();

    void accel_isr();
    void setupAccelerometer();
    void readAccelerometer();

    WiFiClient client;

#if !ADAFRUIT_ALPHANUM
    Adafruit_7segment clockDisplay;
#else
    Adafruit_AlphaNum4 clockDisplay;
#endif

    Adafruit_LIS3DH accel;

    SX1509RGBLED statusLED;
    SX1509 sx1509;
    PilotLight pilotLight;
    WiThrottle wiThrottle;

    TogglePosition previousTogglePosition;

    int speedAccumulator;
    int speedCount;

    int previousSpeedValue;
    int penultimateSpeedValue;

    bool handleSX1509Interrupt;
    bool handleAccelInterrupt;
    int accelIntrValue;

    Chrono accelerometerCheck;
    Chrono speedPotRead;
    Chrono speedCheck;


    bool wifiConnected;

    int port;

    WifiInfo wifiInfo;

    BLEThrottle throttleInfo;

    BLEServer *bleServer;

    ThrottleData& flashData;

    bool restartWifiOnNextCycle;
};
