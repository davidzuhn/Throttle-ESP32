// -*- c++ -*-

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
