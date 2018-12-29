#include "Arduino.h"

#include "BLEThrottle.h"

BLEThrottle::BLEThrottle() :
    speed(0),
    direction(Forward)
{
}


void
BLEThrottle::begin(BLEServer *bleServer, Stream *console)
{
    this->console = console;

    if (bleServer) {
        throttleService = bleServer->createService(THROTTLE_SERVICE_UUID);
        speedCharacteristic = throttleService->createCharacteristic(
            THROTTLE_SPEED_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
        speedCharacteristic->setCallbacks(this);

        directionCharacteristic = throttleService->createCharacteristic(
            THROTTLE_DIRECTION_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
        directionCharacteristic->setCallbacks(this);

        throttleService->start();

        if (throttleService) {
        }
        else {
            console->print("no throttleService created");
        }

    }
    else {
        console->println("no bleServer in BLEThrottle::begin");
    }
}


void
BLEThrottle::setSpeed(int speed)
{
    this->speed =  (uint8_t) speed;
    speedCharacteristic->setValue(&(this->speed), 1);
    speedCharacteristic->notify();

}


void
BLEThrottle::setDirection(Direction direction)
{
    this->direction = direction;
    std::string directionString;
    switch (direction) {
        case Reverse:
            directionString = "Reverse";
            break;
        case Forward:
            directionString = "Forward";
            break;
    }

    directionCharacteristic->setValue(directionString);
    directionCharacteristic->notify();
}


void
BLEThrottle::onWrite(BLECharacteristic *characteristic)
{
    console->print("write for UUID: ");
    console->println(characteristic->getUUID().toString().c_str());
}


void
BLEThrottle::onRead(BLECharacteristic *characteristic)
{
    console->print("read value for ");
    console->println(characteristic->getUUID().toString().c_str());

    if (characteristic->getUUID().equals(BLEUUID(THROTTLE_SPEED_CHARACTERISTIC_UUID))) {
        characteristic->setValue(&speed, 1);
    }
    else if (characteristic->getUUID().equals(BLEUUID(THROTTLE_DIRECTION_CHARACTERISTIC_UUID))) {
        std::string directionString;
        switch (direction) {
            case Reverse:
                directionString = "Reverse";
                break;
            case Forward:
                directionString = "Forward";
                break;
        }
        characteristic->setValue(directionString);
    }
}
