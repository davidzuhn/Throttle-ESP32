// -*- c++ -*-
#pragma once

#include <string>

#include "Arduino.h"

#include <Chrono.h>
#include <Time.h>
#include <TimeLib.h>

#include <WiFi.h>

#include "ESP32HW.h"


#include "WiThrottle.h"
#include "WifiInfo.h"

#include "ThrottleData.h"

// Several BLE Services available on this device...
#include "ThrottleService.h"


#define ADAFRUIT_ALPHANUM 1



typedef enum ThrottleState
{
    TSTATE_UNKNOWN = 0,
    TSTATE_WIFI_DISCONNECTED,
    TSTATE_WIFI_CONNECTED,
    TSTATE_WITHROTTLE_CONNECTED,
    TSTATE_WITHROTTLE_ACTIVE
} ThrottleState;



class ThrottleController:
    public WiThrottleDelegate,
    public WifiInfoDelegate,
    public ThrottleHWDelegate
{
  public:
    ThrottleController();

    // callables from the Arduino main program
    bool begin();
    void loop();

    void test_loop();   // run the HW tests

    void wifiCommandReceived(std::string command);

    void setThrottleState(ThrottleState newState);

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

    // ThrottleHW callback methods
    void speedChanged(int newSpeed, TogglePosition togglePosition);
    void togglePositionChanged(TogglePosition newPosition);
    void speedChanged(int newSpeed);
    void throttleMoved();
    void throttleFell();
    void batteryLevelChanged(int batteryLevel);
    void functionButtonChanged(int func, bool pressed);


  private:
    void updateFastTimeDisplay();
    void updateDirection(TogglePosition togglePosition);
    Direction directionFromTogglePosition(TogglePosition position);
    void setupBLE();


    WiFiClient     client;
    ESP32HW        hw;
    WiThrottle     wiThrottle;
    bool           wifiConnected;
    int            port;
    WifiInfo       wifiInfo;
    ThrottleService throttleService;
    BLEServer      *bleServer;
    ThrottleData   flashData;
    bool           restartWifiOnNextCycle;
    ThrottleState  currentThrottleState;
};
