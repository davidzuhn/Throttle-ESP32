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
}


void
WifiInfo::begin(BLEServer *bleServer, Stream *console)
{
    this->console = console;

    if (bleServer) {
        wifiService = bleServer->createService(WIFI_SERVICE_UUID);

        if (wifiService) {
            ssidCharacteristic = wifiService->createCharacteristic(WIFI_SSID_CHARACTERISTIC_UUID,
                                                                   BLECharacteristic::PROPERTY_READ
                                                                   | BLECharacteristic::PROPERTY_WRITE);
            ssidCharacteristic->setCallbacks(this);

            console->println("SSID Characteristic: ");
            console->println(ssidCharacteristic->toString().c_str());


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
            console->println("BLE wifiService started");

            advertisements = bleServer->getAdvertising();

            advertisements->addServiceUUID(wifiService->getUUID());
            advertisements->includeName(false);
            advertisements->start();
            console->println("BLE advertising started");
        }
        else {
            console->println("no wifiService created");
        }
    }
    else {
        console->println("no bleServer in WifiInfo::begin");
    }

    console->println("WifiInfo setup complete");
}


void
WifiInfo::onWrite(BLECharacteristic *characteristic)
{
    if (!characteristic) {
        return;
    }

    console->print("write for UUID: ");
    console->println(characteristic->getUUID().toString().c_str());

    if (characteristic->getUUID().equals(BLEUUID(WIFI_SSID_CHARACTERISTIC_UUID))) {
        ssid = characteristic->getValue();
        flashData.saveWifiSSID(ssid);
        console->print("write for ssid "); console->println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PASSWORD_CHARACTERISTIC_UUID))) {
        password = characteristic->getValue();
        flashData.saveWifiPassword(password);
        console->print("write for password "); console->println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_SERVER_CHARACTERISTIC_UUID))) {
        serverAddress = characteristic->getValue();
        flashData.saveServerAddress(serverAddress);
        console->print("write for server address "); console->println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PORT_CHARACTERISTIC_UUID))) {
        serverPort = characteristic->getValue();
        flashData.saveServerPort(serverPort);
        console->print("write for server port "); console->println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_COMMAND_CHARACTERISTIC_UUID))) {
        console->print("write for command "); console->println(characteristic->getValue().c_str());
        if (delegate) {
            delegate->wifiCommandReceived(characteristic->getValue());
        }
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(DEVICE_NAME_CHARACTERISTIC_UUID))) {
        auto deviceName = characteristic->getValue();
        flashData.saveDeviceName(deviceName);
        console->print("write for device name "); console->println(characteristic->getValue().c_str());
    }
    else {
        console->printf("received write for unknown UUID: %s\n", characteristic->getUUID().toString());
    }
}


void
WifiInfo::onRead(BLECharacteristic *characteristic)
{
    if (!characteristic) {
        return;
    }

    console->print("read value for ");
    console->println(characteristic->getUUID().toString().c_str());

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
    console->print("BLE: setConnectionState "); console->println(state.c_str());
    connectionState = state;
    if (statusCharacteristic) {
        statusCharacteristic->setValue(connectionState);
        statusCharacteristic->notify();
    }
}
