
#include "Arduino.h"

#include "WiThrottle.h"

class BKTThrottleDelegate : public WiThrottleDelegate
{
  public:
    void receivedVersion(String version);

    void receivedFastTime(uint32_t time);
    void receivedFastTimeRate(double rate);

    void receivedFunctionState(uint8_t func, bool state);

    void receivedSpeed(int speed);
    void receivedDirection(Direction dir);
    void receivedSpeedSteps(int steps);

    void receivedWebPort(int port);

    void receivedTrackPower(TrackPower state); // PPAn

};
