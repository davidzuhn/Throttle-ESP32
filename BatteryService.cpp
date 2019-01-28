/*
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

#include "BatteryService.h"


BatteryService::BatteryService() :
    batteryLevel(0)
{
}


void
BatteryService::begin(BLEServer *bleServer, Stream *console)
{
    this->console = console;

    if (bleServer) {
        batteryService = bleServer->createService(BATTERY_LEVEL_SERVICE_UUID);

        if (batteryService) {
            batteryLevelCharacteristic = batteryService->createCharacteristic(BATTERY_LEVEL_CHARACTERISTIC_UUID,
                                                                              BLECharacteristic::PROPERTY_READ
                                                                              | BLECharacteristic::PROPERTY_NOTIFY);
            batteryLevelCharacteristic->setCallbacks(this);


            batteryService->start();
        }
        console->println("battery service started");
    }
}


void
BatteryService::onWrite(BLECharacteristic *characteristic)
{
    // no writes are supported by this service
    return;
}


void
BatteryService::onRead(BLECharacteristic *characteristic)
{
    if (!characteristic) {
        return;
    }

    if (characteristic->getUUID().equals(BLEUUID(BATTERY_LEVEL_CHARACTERISTIC_UUID))) {
        characteristic->setValue(batteryLevel);
    }
}


void
BatteryService::setBatteryLevel(int batteryLevel)
{
    this->batteryLevel = batteryLevel;

    batteryLevelCharacteristic->setValue(batteryLevel);
    batteryLevelCharacteristic->notify();
}
