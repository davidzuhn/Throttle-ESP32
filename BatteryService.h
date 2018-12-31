// -*- c++ -*-
#pragma once

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#define BATTERY_LEVEL_SERVICE_UUID         ((uint16_t) 0x180F)
#define BATTERY_LEVEL_CHARACTERISTIC_UUID  ((uint16_t) 0x2A19)



class BatteryService :
    public BLECharacteristicCallbacks
{
  public:
    BatteryService();
    void begin(BLEServer *bleServer, Stream *console);

    void onWrite(BLECharacteristic *characteristic);
    void onRead(BLECharacteristic *characteristic);

    void setBatteryLevel(int batteryLevel);

private:
    int batteryLevel;

    BLEService *batteryService;
    BLECharacteristic *batteryLevelCharacteristic;

    Stream *console;
};
