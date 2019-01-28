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

#include <string>

#include "Arduino.h"

#include <Chrono.h>
#include <Time.h>
#include <TimeLib.h>

#include <WiFi.h>

#include "ESP32HW.h"


#include "WiThrottle.h"

#include "ThrottleData.h"

// Several BLE Services available on this device...
#include "ThrottleService.h"
#include "WifiService.h"
#include "BatteryService.h"
#include "DeviceInfoService.h"


////////////////////////////////////////////////////////////////////////////////
//
// version numbers
//
#define SW_VERSION "0.10"


////////////////////////////////////////////////////////////////////////////////
//
// manufacturer information
//
#define MANUFACTURER_NAME  "Blue Knobby Systems"
#define MODEL_NUMBER       "BKT-0revB"





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
    public WifiServiceDelegate,
    public ThrottleServiceDelegate,
    public ThrottleHWDelegate
{
  public:
    ThrottleController();

    // callables from the Arduino main program
    bool begin();
    void loop();

    void test_loop();   // run the HW tests

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
    void addressAdded(String address, String entry);
    void addressRemoved(String address, String command);
    void addressStealNeeded(String address, String entry);


    // WiFi callback methods
    void wifiOnConnect();
    void wifiOnDisconnect();
    void wifiEvent(WiFiEvent_t event);

    // Wifi service callback methods
    void wifiCommandReceived(std::string command);

    // Throttle service callback methods
    void throttleAddressChanged(std::string address);

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


    WiFiClient        client;
    ESP32HW           hw;
    WiThrottle        wiThrottle;
    bool              wifiConnected;
    int               port;
    WifiService       wifiService;
    ThrottleService   throttleService;
    BatteryService    batteryService;
    DeviceInfoService deviceInfoService;
    BLEServer         *bleServer;
    ThrottleData      flashData;
    bool              restartWifiOnNextCycle;
    ThrottleState     currentThrottleState;
    Chrono            wifiRetryCheck;

    String            selectedAddress;
    bool              addressIsSelected;
};
