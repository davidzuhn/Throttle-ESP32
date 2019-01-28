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
