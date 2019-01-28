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
