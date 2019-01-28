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

#include "DeviceInfoService.h"



DeviceInfoService::DeviceInfoService()
{
}


void
DeviceInfoService::begin(BLEServer *bleServer, Stream *console)
{
    this->console = console;

    if (bleServer) {
        deviceInfoService = bleServer->createService(DEVICEINFO_SERVICE_UUID);

        if (deviceInfoService) {
            mfgNameCharacteristic = deviceInfoService->createCharacteristic(DEVICEINFO_MFGNAME_CHARACTERISTIC_UUID,
                                                                            BLECharacteristic::PROPERTY_READ);
            modelNumberCharacteristic = deviceInfoService->createCharacteristic(DEVICEINFO_MODELNUM_CHARACTERISTIC_UUID,
                                                                                BLECharacteristic::PROPERTY_READ);
            serialNumberCharacteristic = deviceInfoService->createCharacteristic(DEVICEINFO_SERIALNUM_CHARACTERISTIC_UUID,
                                                                                 BLECharacteristic::PROPERTY_READ);
            hwRevisionCharacteristic = deviceInfoService->createCharacteristic(DEVICEINFO_HWREV_CHARACTERISTIC_UUID,
                                                                               BLECharacteristic::PROPERTY_READ);
            fwRevisionCharacteristic = deviceInfoService->createCharacteristic(DEVICEINFO_FWREV_CHARACTERISTIC_UUID,
                                                                               BLECharacteristic::PROPERTY_READ);
            swRevisionCharacteristic = deviceInfoService->createCharacteristic(DEVICEINFO_SWREV_CHARACTERISTIC_UUID,
                                                                               BLECharacteristic::PROPERTY_READ);



            deviceInfoService->start();
        }

        console->println("deviceInfo service started");
    }
}


void
DeviceInfoService::setMfgName(std::string value)
{
    mfgNameCharacteristic->setValue(value);
}


void
DeviceInfoService::setModelNumber(std::string value)
{
    modelNumberCharacteristic->setValue(value);
}


void
DeviceInfoService::setSerialNumber(std::string value)
{
    serialNumberCharacteristic->setValue(value);
}


void
DeviceInfoService::setHWRevision(std::string value)
{
    hwRevisionCharacteristic->setValue(value);
}


void
DeviceInfoService::setFWRevision(std::string value)
{
    fwRevisionCharacteristic->setValue(value);
}


void
DeviceInfoService::setSWRevision(std::string value)
{
    swRevisionCharacteristic->setValue(value);
}
