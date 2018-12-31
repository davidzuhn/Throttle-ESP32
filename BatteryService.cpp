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
