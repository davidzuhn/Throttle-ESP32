#pragma once

#include "Arduino.h"

#include <string>

class ThrottleDataDelegate
{
  public:
    virtual void throttleDataChanged() { }
};


class ThrottleData
{
  public:
    ThrottleData();
    bool begin(Stream *console);

    std::string getDeviceName();
    void saveDeviceName(std::string);

    std::string getSerialNumber();
    void saveSerialNumber(std::string);

    std::string getWifiSSID();
    void saveWifiSSID(std::string);

    std::string getWifiPassword();
    void saveWifiPassword(std::string);

    std::string getServerAddress();
    void saveServerAddress(std::string);

    std::string getServerPort();
    void saveServerPort(std::string);

  private:
    bool writeFile(std::string filename, std::string content);
    std::string readFile(std::string filename, std::string defaultContent);
    std::string getFileContent(std::string path);
    bool saveContentToFile(std::string data, std::string path);
    Stream *console;
};
