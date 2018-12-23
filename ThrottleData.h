#pragma once

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
    bool begin();

    std::string getDeviceName();
    void saveDeviceName(std::string);

    std::string getWifiSSID();
    void saveWifiSSID(std::string);

    std::string getWifiPassword();
    void saveWifiPassword(std::string);

    std::string getServerAddress();
    void saveServerAddress(std::string);

    std::string getServerPort();
    void saveServerPort(std::string);

  private:
    std::string getFileContent(std::string path);
    bool saveContentToFile(std::string data, std::string path);
};
