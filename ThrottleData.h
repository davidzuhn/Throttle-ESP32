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
