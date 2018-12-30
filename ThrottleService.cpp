#include "Arduino.h"

#include "ThrottleService.h"

ThrottleService::ThrottleService() :
    speed(0),
    direction(Forward),
    togglePosition(UnknownPosition)
{
}


void
ThrottleService::begin(BLEServer *bleServer, Stream *console)
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

        toggleCharacteristic = throttleService->createCharacteristic(
            THROTTLE_TOGGLE_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
        toggleCharacteristic->setCallbacks(this);

        addressCharacteristic = throttleService->createCharacteristic(
            THROTTLE_ADDRESS_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
        addressCharacteristic->setCallbacks(this);

        throttleService->start();
    }
    else {
        console->println("no bleServer in ThrottleService::begin");
    }
}


void
ThrottleService::setSpeed(int speed)
{
    this->speed =  (uint8_t) speed;
    speedCharacteristic->setValue(&(this->speed), 1);
    speedCharacteristic->notify();
}


void
ThrottleService::setDirection(Direction direction)
{
    this->direction = direction;
    std::string value = directionString(direction);

    directionCharacteristic->setValue(value);
    directionCharacteristic->notify();
}


void
ThrottleService::setTogglePosition(TogglePosition newTogglePosition)
{
    if (togglePosition != newTogglePosition) {
        togglePosition = newTogglePosition;

        std::string value = togglePositionString(togglePosition);
        toggleCharacteristic->setValue(value);
        toggleCharacteristic->notify();
    }
}


void
ThrottleService::setSelectedAddress(std::string newAddress)
{
    if (address != newAddress) {
        address = newAddress;

        addressCharacteristic->setValue(address);
        addressCharacteristic->notify();
    }
}



std::string
ThrottleService::directionString(Direction direction)
{
    std::string directionString;
    switch (direction) {
        case Reverse:
            directionString = "Reverse";
            break;
        case Forward:
            directionString = "Forward";
            break;
    }

    return directionString;
}

std::string
ThrottleService::togglePositionString(TogglePosition togglePosition)
{
    std::string togglePositionString;
    switch (togglePosition) {
        case Left:
            togglePositionString = "Left";
            break;
        case Right:
            togglePositionString = "Right";
            break;
        case CenterOff:
            togglePositionString = "Center-Off";
            break;
        default:
            togglePositionString = "Unknown";
            break;
    }

    return togglePositionString;
}



void
ThrottleService::onWrite(BLECharacteristic *characteristic)
{
    console->print("write for UUID: ");
    console->println(characteristic->getUUID().toString().c_str());

    if (characteristic->getUUID().equals(BLEUUID(THROTTLE_ADDRESS_CHARACTERISTIC_UUID))) {
        if (delegate) {
            delegate->throttleAddressChanged(characteristic->getValue());
        }
    }
}


void
ThrottleService::onRead(BLECharacteristic *characteristic)
{
    console->print("read value for ");
    console->println(characteristic->getUUID().toString().c_str());

    if (characteristic->getUUID().equals(BLEUUID(THROTTLE_SPEED_CHARACTERISTIC_UUID))) {
        characteristic->setValue(&speed, 1);
    }
    else if (characteristic->getUUID().equals(BLEUUID(THROTTLE_DIRECTION_CHARACTERISTIC_UUID))) {
        std::string value = directionString(direction);
        characteristic->setValue(value);
    }
    else if (characteristic->getUUID().equals(BLEUUID(THROTTLE_TOGGLE_CHARACTERISTIC_UUID))) {
        std::string value = togglePositionString(togglePosition);
        characteristic->setValue(value);
    }
    else if (characteristic->getUUID().equals(BLEUUID(THROTTLE_ADDRESS_CHARACTERISTIC_UUID))) {
        characteristic->setValue(address);
    }
}
