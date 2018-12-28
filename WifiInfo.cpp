#include "Arduino.h"
#include "DebugSerial.h"

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
    DebugSerial.println("WifiInfo constructor");
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

            DebugSerial.println("SSID Characteristic: ");
            DebugSerial.println(ssidCharacteristic->toString().c_str());


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
            DebugSerial.println("BLE wifiService started");

            advertisements = bleServer->getAdvertising();

            advertisements->addServiceUUID(wifiService->getUUID());
            advertisements->includeName(false);
            advertisements->start();
            DebugSerial.println("BLE advertising started");
        }
        else {
            DebugSerial.println("no wifiService created");
        }
    }
    else {
        DebugSerial.println("no bleServer in WifiInfo::begin");
    }
}


void
WifiInfo::onWrite(BLECharacteristic *characteristic)
{
    if (!characteristic) {
        return;
    }

    DebugSerial.print("write for UUID: ");
    DebugSerial.println(characteristic->getUUID().toString().c_str());

    if (characteristic->getUUID().equals(BLEUUID(WIFI_SSID_CHARACTERISTIC_UUID))) {
        ssid = characteristic->getValue();
        flashData.saveWifiSSID(ssid);
        DebugSerial.print("write for ssid "); DebugSerial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PASSWORD_CHARACTERISTIC_UUID))) {
        password = characteristic->getValue();
        flashData.saveWifiPassword(password);
        DebugSerial.print("write for password "); DebugSerial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_SERVER_CHARACTERISTIC_UUID))) {
        serverAddress = characteristic->getValue();
        flashData.saveServerAddress(serverAddress);
        DebugSerial.print("write for server address "); DebugSerial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_PORT_CHARACTERISTIC_UUID))) {
        serverPort = characteristic->getValue();
        flashData.saveServerPort(serverPort);
        DebugSerial.print("write for server port "); DebugSerial.println(characteristic->getValue().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_COMMAND_CHARACTERISTIC_UUID))) {
        DebugSerial.print("write for command "); DebugSerial.println(characteristic->getValue().c_str());
        if (delegate) {
            delegate->wifiCommandReceived(characteristic->getValue());
        }
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(DEVICE_NAME_CHARACTERISTIC_UUID))) {
        auto deviceName = characteristic->getValue();
        flashData.saveDeviceName(deviceName);
        DebugSerial.print("write for device name "); DebugSerial.println(characteristic->getValue().c_str());
    }
    else {
        DebugSerial.printf("received write for unknown UUID: %s\n", characteristic->getUUID().toString());
    }
}


void
WifiInfo::onRead(BLECharacteristic *characteristic)
{
    if (!characteristic) {
        return;
    }

    DebugSerial.print("read value for ");
    DebugSerial.println(characteristic->getUUID().toString().c_str());

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
    DebugSerial.print("BLE: setConnectionState "); DebugSerial.println(state.c_str());
    connectionState = state;
    if (statusCharacteristic) {
        statusCharacteristic->setValue(connectionState);
        statusCharacteristic->notify();
    }
}
