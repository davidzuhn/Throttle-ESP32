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
#include <iostream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "ThrottleData.h"
#include "WiThrottle.h"
#include "ThrottleHW.h"


#define THROTTLE_SERVICE_UUID                  "426c7565-3700-4688-b7f5-4b646f626279"
#define THROTTLE_SPEED_CHARACTERISTIC_UUID     "426c7565-37e1-4688-b7f5-4b646f626279"
#define THROTTLE_DIRECTION_CHARACTERISTIC_UUID "426c7565-37e2-4688-b7f5-4b646f626279"
#define THROTTLE_TOGGLE_CHARACTERISTIC_UUID    "426c7565-37e3-4688-b7f5-4b646f626279"
#define THROTTLE_ADDRESS_CHARACTERISTIC_UUID   "426c7565-37e4-4688-b7f5-4b646f626279"
#define THROTTLE_DESCRIPTION_CHARACTERISTIC_UUID "426c7565-37e5-4688-b7f5-4b646f626279"

class ThrottleServiceDelegate
{
  public:
    virtual void throttleAddressChanged(std::string address) { };
};




class ThrottleService :
    public BLECharacteristicCallbacks
{
  public:
    ThrottleService();
    void begin(BLEServer *bleServer, Stream *console);

    void onWrite(BLECharacteristic *characteristic);
    void onRead(BLECharacteristic *characteristic);

    void setSpeed(int speed);
    void setDirection(Direction direction);
    void setTogglePosition(TogglePosition position);
    void setSelectedAddress(std::string address);
    void setLongDescription(std::string address);

    ThrottleServiceDelegate *delegate;


  private:
    std::string directionString(Direction direction);
    std::string togglePositionString(TogglePosition togglePosition);

    BLEService *throttleService;
    BLECharacteristic *speedCharacteristic;
    BLECharacteristic *directionCharacteristic;
    BLECharacteristic *toggleCharacteristic;
    BLECharacteristic *addressCharacteristic;
    BLECharacteristic *descriptionCharacteristic;

    uint8_t speed;
    Direction direction;
    TogglePosition togglePosition;
    std::string address;
    std::string longDescription;

    Stream *console;
};
