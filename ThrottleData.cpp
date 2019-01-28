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


#include "ThrottleData.h"

#include "FS.h"
#include "SPIFFS.h"

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

#define DEVICE_NAME_FILE "/deviceName"
#define SSID_FILE "/ssid"
#define PASSWORD_FILE "/password"
#define SERVER_ADDRESS_FILE "/server"
#define SERVER_PORT_FILE "/serverPort"
#define SERIAL_NUMBER_FILE "/serialNumber"


ThrottleData::ThrottleData()
{
}


bool
ThrottleData::begin(Stream *console)
{
    this->console = console;

    bool rv = true;

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        rv = false;
    }
    else {
        size_t total = SPIFFS.totalBytes();
        size_t used  = SPIFFS.usedBytes();

        console->printf("FS %d/%d bytes\n", used, total);
    }

    return rv;
}


bool
ThrottleData::writeFile(std::string filename, std::string content)
{
    bool rv = false;

    File file = SPIFFS.open(filename.c_str(), FILE_WRITE);
    if (!file || file.isDirectory()) {
        rv = false;
        console->printf("unable to open file %s\n", filename.c_str());
    }
    else {
        rv = file.print(content.c_str());
        console->printf("write file %s with '%s': %d\n", filename.c_str(), content.c_str(), rv);
    }

    return rv;
}


std::string
ThrottleData::readFile(std::string filename, std::string defaultContent)
{
    std::string content;

    File file = SPIFFS.open(filename.c_str());
    if (!file || file.isDirectory()) {
        return defaultContent;
    }
    else {
        while (file.available()) {
            char c = file.read();

            content += c;
        }
    }

    return content;
}



////////////////////////////////////////////////////////////////////////////////

std::string
ThrottleData::getDeviceName()
{
    std::string deviceName = readFile(DEVICE_NAME_FILE, "Device Name");
    return deviceName;
}

void
ThrottleData::saveDeviceName(std::string deviceName)
{
    writeFile(DEVICE_NAME_FILE, deviceName);
}

////////////////////////////////////////////////////////////////////////////////

std::string
ThrottleData::getSerialNumber()
{
    std::string serialNumber = readFile(SERIAL_NUMBER_FILE, "00000000");
    return serialNumber;
}

void
ThrottleData::saveSerialNumber(std::string serialNumber)
{
    writeFile(SERIAL_NUMBER_FILE, serialNumber);
}

////////////////////////////////////////////////////////////////////////////////

std::string
ThrottleData::getWifiSSID()
{
    std::string ssid = readFile(SSID_FILE, "SSID");
    return ssid;
}

void
ThrottleData::saveWifiSSID(std::string ssid)
{
    writeFile(SSID_FILE, ssid);
}

////////////////////////////////////////////////////////////////////////////////

std::string
ThrottleData::getWifiPassword()
{
    std::string password = readFile(PASSWORD_FILE, "Password");
    return password;
}

void
ThrottleData::saveWifiPassword(std::string password)
{
    writeFile(PASSWORD_FILE, password);
}

////////////////////////////////////////////////////////////////////////////////

std::string
ThrottleData::getServerAddress()
{
    std::string server = readFile(SERVER_ADDRESS_FILE, "Server");
    return server;
}

void
ThrottleData::saveServerAddress(std::string server)
{
    writeFile(SERVER_ADDRESS_FILE, server);
}

////////////////////////////////////////////////////////////////////////////////

std::string
ThrottleData::getServerPort()
{
    std::string server = readFile(SERVER_PORT_FILE, "12090");
    return server;
}

void
ThrottleData::saveServerPort(std::string serverPort)
{
    writeFile(SERVER_PORT_FILE, serverPort);
}
