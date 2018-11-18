#pragma once

#include "Arduino.h"

#include <string>
#include <iostream>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define WIFI_SERVICE_UUID                  "426c7565-3600-4688-b7f5-4b646f626279"
#define WIFI_SSID_CHARACTERISTIC_UUID      "426c7565-36e1-4688-b7f5-4b646f626279"
#define WIFI_PASSWORD_CHARACTERISTIC_UUID  "426c7565-36e2-4688-b7f5-4b646f626279"
#define WIFI_SERVER_CHARACTERISTIC_UUID    "426c7565-36e3-4688-b7f5-4b646f626279"
#define WIFI_STATUS_CHARACTERISTIC_UUID    "426c7565-36e4-4688-b7f5-4b646f626279"
#define WIFI_COMMAND_CHARACTERISTIC_UUID   "426c7565-36e5-4688-b7f5-4b646f626279"

class WifiInfo : public BLECharacteristicCallbacks
{
  public:
    WifiInfo();
    void begin(BLEServer *bleServer);

    void onWrite(BLECharacteristic *characteristic);
    void onRead(BLECharacteristic *characteristic);

    std::string ssid;
    std::string password;
    std::string serverAddress;

  private:
    BLEService *wifiService;
    BLECharacteristic *ssidCharacteristic;
    BLECharacteristic *passwordCharacteristic;
    BLECharacteristic *serverCharacteristic;
    BLECharacteristic *statusCharacteristic;
    BLECharacteristic *commandCharacteristic;

    BLEAdvertising *advertisements;
};
