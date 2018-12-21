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
#define SERVER_FILE "/server"

ThrottleData::ThrottleData()
{
}


bool
ThrottleData::begin()
{
    bool rv = true;

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        rv = false;
    }
    else {
        size_t total = SPIFFS.totalBytes();
        size_t used  = SPIFFS.usedBytes();

        Serial.printf("FS %d/%d bytes\n", used, total);
    }

    return rv;
}


static bool
writeFile(std::string filename, std::string content)
{
    bool rv = false;

    File file = SPIFFS.open(filename.c_str(), FILE_WRITE);
    if (!file || file.isDirectory()) {
        rv = false;
        Serial.printf("unable to open file %s\n", filename.c_str());
    }
    else {
        rv = file.print(content.c_str());
        Serial.printf("write file %s with '%s': %d\n", filename.c_str(), content.c_str(), rv);
    }

    return rv;
}


static std::string
readFile(std::string filename, std::string defaultContent)
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

std::string ThrottleData::getWifiSSID()
{
    std::string ssid = readFile(SSID_FILE, "SSID");
    return ssid;
}

void ThrottleData::saveWifiSSID(std::string ssid)
{
    writeFile(SSID_FILE, ssid);
}

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

std::string
ThrottleData::getServerAddress()
{
    std::string server = readFile(SERVER_FILE, "Server");
    return server;
}

void
ThrottleData::saveServerAddress(std::string server)
{
    writeFile(SERVER_FILE, server);
}
