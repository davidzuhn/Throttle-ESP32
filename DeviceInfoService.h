// -*- c++ -*-
#pragma once

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#define DEVICEINFO_SERVICE_UUID                   ((uint16_t) 0x180A)

#define DEVICEINFO_MFGNAME_CHARACTERISTIC_UUID    ((uint16_t) 0x2A29)
#define DEVICEINFO_MODELNUM_CHARACTERISTIC_UUID   ((uint16_t) 0x2A24)
#define DEVICEINFO_SERIALNUM_CHARACTERISTIC_UUID  ((uint16_t) 0x2A25)
#define DEVICEINFO_HWREV_CHARACTERISTIC_UUID      ((uint16_t) 0x2A27)
#define DEVICEINFO_FWREV_CHARACTERISTIC_UUID      ((uint16_t) 0x2A26)
#define DEVICEINFO_SWREV_CHARACTERISTIC_UUID      ((uint16_t) 0x2A28)


class DeviceInfoService
{
  public:
    DeviceInfoService();
    void begin(BLEServer *bleServer, Stream *console);

    void setMfgName(std::string mfgName);
    void setModelNumber(std::string modelNumber);
    void setSerialNumber(std::string serialNumber);
    void setHWRevision(std::string hwRevision);
    void setFWRevision(std::string fwRevision);
    void setSWRevision(std::string swRevision);

private:
    BLEService *deviceInfoService;
    BLECharacteristic *mfgNameCharacteristic;
    BLECharacteristic *modelNumberCharacteristic;
    BLECharacteristic *serialNumberCharacteristic;
    BLECharacteristic *hwRevisionCharacteristic;
    BLECharacteristic *fwRevisionCharacteristic;
    BLECharacteristic *swRevisionCharacteristic;

    Stream *console;
};
