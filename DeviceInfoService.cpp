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
