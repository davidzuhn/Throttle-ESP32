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
#define WIFI_SSID_LIST_CHARACTERISTIC_UUID "426c7565-36e7-4688-b7f5-4b646f626279"

#define WIFI_DEVICE_ADDRESS_CHARACTERISTIC_UUID "426c7565-36e8-4688-b7f5-4b646f626279"
#define WIFI_DEVICE_NETMASK_CHARACTERISTIC_UUID "426c7565-36e9-4688-b7f5-4b646f626279"
#define WIFI_DEVICE_GATEWAY_CHARACTERISTIC_UUID "426c7565-36ea-4688-b7f5-4b646f626279"
#define WIFI_DEVICE_MAC_CHARACTERISTIC_UUID     "426c7565-36eb-4688-b7f5-4b646f626279"

#define DEVICE_NAME_CHARACTERISTIC_UUID    "426c7565-36f0-4688-b7f5-4b646f626279"


class WifiServiceDelegate
{
  public:
    virtual void wifiCommandReceived(std::string command) { }
};



class WifiService:
    public BLECharacteristicCallbacks
{
  public:
    WifiService(ThrottleData& flashData);
    void begin(BLEServer *bleServer, Stream *console);

    void onWrite(BLECharacteristic *characteristic);
    void onRead(BLECharacteristic *characteristic);
    void setConnectionState(std::string state);

    void setDeviceAddress(IPAddress address);
    void setDeviceNetmask(IPAddress netmask);
    void setDeviceGateway(IPAddress gateway);
    void setDeviceMac(std::string mac);

    void scanNetworks();

    WifiServiceDelegate *delegate;

  private:
    std::string ssid;
    std::string password;
    std::string serverAddress;
    std::string serverPort;
    IPAddress deviceAddress;
    IPAddress deviceNetmask;
    IPAddress deviceGateway;
    std::string deviceMac;


    BLEService *wifiService;
    BLECharacteristic *ssidCharacteristic;
    BLECharacteristic *passwordCharacteristic;
    BLECharacteristic *serverCharacteristic;
    BLECharacteristic *portCharacteristic;
    BLECharacteristic *statusCharacteristic;
    BLECharacteristic *commandCharacteristic;
    BLECharacteristic *ssidListCharacteristic;

    BLECharacteristic *deviceNameCharacteristic;

    BLECharacteristic *deviceAddressCharacteristic;
    BLECharacteristic *deviceNetmaskCharacteristic;
    BLECharacteristic *deviceGatewayCharacteristic;
    BLECharacteristic *deviceMacCharacteristic;

    BLEAdvertising *advertisements;

    ThrottleData& flashData;
    std::string connectionState;

    Stream *console;
};
