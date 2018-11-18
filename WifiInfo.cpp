#include "Arduino.h"

#include "WifiInfo.h"


WifiInfo::WifiInfo():
    ssid(),
    password(),
    serverAddress(),
    advertisements(NULL)
{
    Serial.println("WifiInfo constructor");
}


void
WifiInfo::begin(BLEServer *bleServer)
{
    if (bleServer) {
        wifiService = bleServer->createService(WIFI_SERVICE_UUID);

        if (wifiService) {
            ssidCharacteristic = wifiService->createCharacteristic(WIFI_SSID_CHARACTERISTIC_UUID,
                                                                   BLECharacteristic::PROPERTY_READ
                                                                   | BLECharacteristic::PROPERTY_WRITE);
            ssidCharacteristic->setCallbacks(this);

            Serial.println("SSID Characteristic: ");
            Serial.println(ssidCharacteristic->toString().c_str());


            passwordCharacteristic = wifiService->createCharacteristic(WIFI_PASSWORD_CHARACTERISTIC_UUID,
                                                                       BLECharacteristic::PROPERTY_READ
                                                                       | BLECharacteristic::PROPERTY_WRITE);
            //passwordCharacteristic->addDescriptor(&stringTypeDescriptor);
            passwordCharacteristic->setCallbacks(this);


            serverCharacteristic = wifiService->createCharacteristic(WIFI_SERVER_CHARACTERISTIC_UUID,
                                                                     BLECharacteristic::PROPERTY_READ
                                                                     | BLECharacteristic::PROPERTY_WRITE);
            //serverCharacteristic->addDescriptor(&stringTypeDescriptor);
            serverCharacteristic->setCallbacks(this);


            statusCharacteristic = wifiService->createCharacteristic(WIFI_STATUS_CHARACTERISTIC_UUID,
                                                                     BLECharacteristic::PROPERTY_READ
                                                                     | BLECharacteristic::PROPERTY_NOTIFY);
            statusCharacteristic->setCallbacks(this);

            commandCharacteristic = wifiService->createCharacteristic(WIFI_COMMAND_CHARACTERISTIC_UUID,
                                                                      BLECharacteristic::PROPERTY_WRITE);
            commandCharacteristic->setCallbacks(this);


            wifiService->start();
            Serial.println("BLE wifiService started");

            advertisements = bleServer->getAdvertising();

            advertisements->addServiceUUID(wifiService->getUUID());
            //advertisements->addServiceUUID((uint16_t) 0xFEED);
            advertisements->start();
            Serial.println("BLE advertising started");
        }
        else {
            Serial.println("no wifiService");
        }
    }
    else {
        Serial.println("no bleServer");
    }
}


void WifiInfo::onWrite(BLECharacteristic *characteristic)
{


}


void WifiInfo::onRead(BLECharacteristic *characteristic)
{
}
