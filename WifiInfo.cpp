#include "Arduino.h"

#include "WifiInfo.h"


WifiInfo::WifiInfo(ThrottleData& flashData):
    ssid(),
    password(),
    serverAddress(),
    serverPort(),
    connectionState("DISCONNECTED"),
    delegate(NULL),
    advertisements(NULL),
    flashData(flashData)
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

            portCharacteristic = wifiService->createCharacteristic(WIFI_PORT_CHARACTERISTIC_UUID,
                                                                     BLECharacteristic::PROPERTY_READ
                                                                     | BLECharacteristic::PROPERTY_WRITE);
            //serverCharacteristic->addDescriptor(&stringTypeDescriptor);
            portCharacteristic->setCallbacks(this);


            statusCharacteristic = wifiService->createCharacteristic(WIFI_STATUS_CHARACTERISTIC_UUID,
                                                                     BLECharacteristic::PROPERTY_READ
                                                                     | BLECharacteristic::PROPERTY_NOTIFY);
            statusCharacteristic->setCallbacks(this);

            commandCharacteristic = wifiService->createCharacteristic(WIFI_COMMAND_CHARACTERISTIC_UUID,
                                                                      BLECharacteristic::PROPERTY_WRITE);
            commandCharacteristic->setCallbacks(this);


            deviceNameCharacteristic = wifiService->createCharacteristic(DEVICE_NAME_CHARACTERISTIC_UUID,
                                                                         BLECharacteristic::PROPERTY_READ
                                                                         | BLECharacteristic::PROPERTY_WRITE);
            deviceNameCharacteristic->setCallbacks(this);



            wifiService->start();
            Serial.println("BLE wifiService started");

            advertisements = bleServer->getAdvertising();

            advertisements->addServiceUUID(wifiService->getUUID());
            advertisements->includeName(false);
            advertisements->start();
            Serial.println("BLE advertising started");
        }
        else {
            Serial.println("no wifiService created");
        }
    }
    else {
        Serial.println("no bleServer in WifiInfo::begin");
    }
}


void
WifiInfo::onWrite(BLECharacteristic *characteristic)
{
    Serial.print("write for UUID: ");
    Serial.println(characteristic->getUUID().toString().c_str());

    if (characteristic->getUUID().equals(BLEUUID(WIFI_SSID_CHARACTERISTIC_UUID))) {
        ssid = characteristic->getValue();
        flashData.saveWifiSSID(ssid);
        Serial.print("write for ssid "); Serial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PASSWORD_CHARACTERISTIC_UUID))) {
        password = characteristic->getValue();
        flashData.saveWifiPassword(password);
        Serial.print("write for password "); Serial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_SERVER_CHARACTERISTIC_UUID))) {
        serverAddress = characteristic->getValue();
        flashData.saveServerAddress(serverAddress);
        Serial.print("write for server address "); Serial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PORT_CHARACTERISTIC_UUID))) {
        serverPort = characteristic->getValue();
        flashData.saveServerPort(serverPort);
        Serial.print("write for server port "); Serial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_COMMAND_CHARACTERISTIC_UUID))) {
        Serial.print("write for command "); Serial.println(characteristic->getValue().c_str());
        if (delegate) {
            delegate->wifiCommandReceived(characteristic->getValue());
        }
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(DEVICE_NAME_CHARACTERISTIC_UUID))) {
        auto deviceName = characteristic->getValue();
        flashData.saveDeviceName(deviceName);
        Serial.print("write for device name "); Serial.println(characteristic->getValue().c_str());
    }
    else {
        Serial.printf("received write for unknown UUID: %s\n", characteristic->getUUID().toString());
    }
}


void
WifiInfo::onRead(BLECharacteristic *characteristic)
{
    Serial.print("read value for ");
    Serial.println(characteristic->getUUID().toString().c_str());

    if (characteristic->getUUID().equals(BLEUUID(WIFI_SSID_CHARACTERISTIC_UUID))) {
        auto ssid = flashData.getWifiSSID();
        characteristic->setValue(ssid);
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PASSWORD_CHARACTERISTIC_UUID))) {
        auto password = flashData.getWifiPassword();
        characteristic->setValue(password);
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_SERVER_CHARACTERISTIC_UUID))) {
        auto serverAddress = flashData.getServerAddress();
        characteristic->setValue(serverAddress);
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PORT_CHARACTERISTIC_UUID))) {
        auto serverPort = flashData.getServerPort();
        characteristic->setValue(serverPort);
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_STATUS_CHARACTERISTIC_UUID))) {
        characteristic->setValue(connectionState);
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(DEVICE_NAME_CHARACTERISTIC_UUID))) {
        characteristic->setValue(flashData.getDeviceName());
    }
}


void
WifiInfo::setConnectionState(std::string state)
{
    Serial.print("BLE: setConnectionState "); Serial.println(state.c_str());
    connectionState = state;
    if (statusCharacteristic) {
        statusCharacteristic->setValue(connectionState);
        statusCharacteristic->notify();
    }
}
