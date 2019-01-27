#include "Arduino.h"

#include "WifiService.h"

#include <sstream>
#include <WiFi.h>


WifiService::WifiService(ThrottleData& flashData):
    ssid(),
    password(),
    serverAddress(),
    serverPort(),
    deviceAddress(),
    deviceNetmask(),
    deviceGateway(),
    deviceMac(),
    connectionState("DISCONNECTED"),
    delegate(NULL),
    advertisements(NULL),
    flashData(flashData)
{
}


void
WifiService::begin(BLEServer *bleServer, Stream *console)
{
    this->console = console;

    if (bleServer) {
        BLEUUID wifiServiceUUID(WIFI_SERVICE_UUID);
        wifiService = bleServer->createService(wifiServiceUUID, 30);

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


            ssidListCharacteristic = wifiService->createCharacteristic(WIFI_SSID_LIST_CHARACTERISTIC_UUID,
                                                                       BLECharacteristic::PROPERTY_READ
                                                                       | BLECharacteristic::PROPERTY_NOTIFY);
            ssidListCharacteristic->setCallbacks(this);


            deviceNameCharacteristic = wifiService->createCharacteristic(DEVICE_NAME_CHARACTERISTIC_UUID,
                                                                         BLECharacteristic::PROPERTY_READ
                                                                         | BLECharacteristic::PROPERTY_WRITE);
            deviceNameCharacteristic->setCallbacks(this);

            deviceAddressCharacteristic = wifiService->createCharacteristic(WIFI_DEVICE_ADDRESS_CHARACTERISTIC_UUID,
                                                                            BLECharacteristic::PROPERTY_READ
                                                                            | BLECharacteristic::PROPERTY_NOTIFY);
            deviceAddressCharacteristic->setCallbacks(this);

            deviceNetmaskCharacteristic = wifiService->createCharacteristic(WIFI_DEVICE_NETMASK_CHARACTERISTIC_UUID,
                                                                            BLECharacteristic::PROPERTY_READ
                                                                            | BLECharacteristic::PROPERTY_NOTIFY);
            deviceNetmaskCharacteristic->setCallbacks(this);

            deviceGatewayCharacteristic = wifiService->createCharacteristic(WIFI_DEVICE_GATEWAY_CHARACTERISTIC_UUID,
                                                                            BLECharacteristic::PROPERTY_READ
                                                                            | BLECharacteristic::PROPERTY_NOTIFY);
            deviceGatewayCharacteristic->setCallbacks(this);

            deviceMacCharacteristic = wifiService->createCharacteristic(WIFI_DEVICE_MAC_CHARACTERISTIC_UUID,
                                                                        BLECharacteristic::PROPERTY_READ
                                                                        | BLECharacteristic::PROPERTY_NOTIFY);
            deviceMacCharacteristic->setCallbacks(this);


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
        console->println("no bleServer in WifiService::begin");
    }

    console->println("WifiService setup complete");
}


void
WifiService::onWrite(BLECharacteristic *characteristic)
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
WifiService::onRead(BLECharacteristic *characteristic)
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
    if (characteristic->getUUID().equals(BLEUUID(WIFI_SSID_LIST_CHARACTERISTIC_UUID))) {
        //characteristic->setValue(ssidListString);
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(DEVICE_NAME_CHARACTERISTIC_UUID))) {
        characteristic->setValue(flashData.getDeviceName());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_DEVICE_ADDRESS_CHARACTERISTIC_UUID))) {
        characteristic->setValue(deviceAddress.toString().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_DEVICE_NETMASK_CHARACTERISTIC_UUID))) {
        characteristic->setValue(deviceNetmask.toString().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_DEVICE_GATEWAY_CHARACTERISTIC_UUID))) {
        characteristic->setValue(deviceGateway.toString().c_str());
    }
    else
    if (characteristic->getUUID().equals(BLEUUID(WIFI_DEVICE_MAC_CHARACTERISTIC_UUID))) {
        characteristic->setValue(deviceMac);
    }
}


void
WifiService::setConnectionState(std::string state)
{
    console->print("BLE: setConnectionState "); console->println(state.c_str());
    connectionState = state;
    if (statusCharacteristic) {
        statusCharacteristic->setValue(connectionState);
        statusCharacteristic->notify();
    }
}


void
WifiService::setDeviceAddress(IPAddress address)
{
    deviceAddress = address;

    std::string value = deviceAddress.toString().c_str();
    if (deviceAddressCharacteristic) {
        deviceAddressCharacteristic->setValue(value);
        deviceAddressCharacteristic->notify();
    }
}


void
WifiService::setDeviceNetmask(IPAddress netmask)
{
    deviceNetmask = netmask;

    std::string value = deviceNetmask.toString().c_str();
    if (deviceNetmaskCharacteristic) {
        deviceNetmaskCharacteristic->setValue(value);
        deviceNetmaskCharacteristic->notify();
    }
}


void
WifiService::setDeviceGateway(IPAddress gateway)
{
    deviceGateway = gateway;

    std::string value = deviceGateway.toString().c_str();
    if (deviceGatewayCharacteristic) {
        deviceGatewayCharacteristic->setValue(value);
        deviceGatewayCharacteristic->notify();
    }
}


void
WifiService::setDeviceMac(std::string mac)
{
    deviceMac = mac;
    if (deviceMacCharacteristic) {
        deviceMacCharacteristic->setValue(deviceMac);
        deviceMacCharacteristic->notify();
    }
}


void
WifiService::scanNetworks()
{
    auto start = millis();
    int n = WiFi.scanNetworks();

    if (n > 0) {
        std::ostringstream s;
        s << n;

        // print the list of networks seen:
        console->print("SSID List:");
        console->println(n);

        // print the network number and name for each network found:
        for (int thisNet = 0; thisNet < n; thisNet++) {
            std::string ssid = WiFi.SSID(thisNet).c_str();
            int32_t rssi = WiFi.RSSI(thisNet);
            std::string encryption = ((WiFi.encryptionType(thisNet) == WIFI_AUTH_OPEN) ? "OPEN " : "*");

            s << "|" << ssid << "," << rssi << "," << encryption;
        }

        const std::string wifiListString(s.str());
        console->printf("wifiListString is %s\n", wifiListString.c_str());

        ssidListCharacteristic->setValue(wifiListString);
        ssidListCharacteristic->notify();
    }
    auto end = millis();

    auto duration = end - start;
    console->printf("scanNetworks took %u millis\n", duration);
}
