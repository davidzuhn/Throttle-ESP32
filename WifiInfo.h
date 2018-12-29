// -*- c++ -*-

#pragma once

#include "Arduino.h"

#include <string>
#include <iostream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "ThrottleData.h"

#define WIFI_SERVICE_UUID                  "426c7565-3600-4688-b7f5-4b646f626279"
#define WIFI_SSID_CHARACTERISTIC_UUID      "426c7565-36e1-4688-b7f5-4b646f626279"
#define WIFI_PASSWORD_CHARACTERISTIC_UUID  "426c7565-36e2-4688-b7f5-4b646f626279"
#define WIFI_SERVER_CHARACTERISTIC_UUID    "426c7565-36e3-4688-b7f5-4b646f626279"
#define WIFI_STATUS_CHARACTERISTIC_UUID    "426c7565-36e4-4688-b7f5-4b646f626279"
#define WIFI_COMMAND_CHARACTERISTIC_UUID   "426c7565-36e5-4688-b7f5-4b646f626279"
#define WIFI_PORT_CHARACTERISTIC_UUID      "426c7565-36e6-4688-b7f5-4b646f626279"

#define DEVICE_NAME_CHARACTERISTIC_UUID    "426c7565-36f0-4688-b7f5-4b646f626279"


class WifiInfoDelegate
{
  public:
    virtual void wifiCommandReceived(std::string command) { }
};



class WifiInfo:
    public BLECharacteristicCallbacks
{
  public:
    WifiInfo(ThrottleData& flashData);
    void begin(BLEServer *bleServer, Stream *console);

    void onWrite(BLECharacteristic *characteristic);
    void onRead(BLECharacteristic *characteristic);
    void setConnectionState(std::string state);


    WifiInfoDelegate *delegate;

  private:
    std::string ssid;
    std::string password;
    std::string serverAddress;
    std::string serverPort;

    BLEService *wifiService;
    BLECharacteristic *ssidCharacteristic;
    BLECharacteristic *passwordCharacteristic;
    BLECharacteristic *serverCharacteristic;
    BLECharacteristic *portCharacteristic;
    BLECharacteristic *statusCharacteristic;
    BLECharacteristic *commandCharacteristic;

    BLECharacteristic *deviceNameCharacteristic;

    BLEAdvertising *advertisements;

    ThrottleData& flashData;
    std::string connectionState;

    Stream *console;
};
