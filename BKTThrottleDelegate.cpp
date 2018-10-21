
#include "BKTThrottleDelegate.h"

#include <SparkFunSX1509.h>
extern SX1509 sx1509;


void
BKTThrottleDelegate::receivedVersion(String version)
{
    Serial.print("received version string ");
    Serial.println(version);
}


void
BKTThrottleDelegate::receivedFunctionState(uint8_t func, bool state)
{
    switch(func) {
        case 0:
            sx1509.digitalWrite(0, state);
            break;
        case 1:
            sx1509.digitalWrite(1, state);
            break;
        case 2:
            sx1509.digitalWrite(2, state);
            break;
        case 3:
            sx1509.digitalWrite(3, state);
            break;
        case 4:
            sx1509.digitalWrite(4, state);
            break;
        case 9:
            sx1509.digitalWrite(9, state);
            break;
        default:
            break;
    }
}


void
BKTThrottleDelegate::receivedSpeed(int speed)
{
    Serial.print("speed value "); Serial.println(speed);
}


void
BKTThrottleDelegate::receivedDirection(Direction dir)
{
    Serial.print("direction is ");
    switch(dir) {
        case Forward: Serial.println("FWD"); break;
        case Reverse: Serial.println("REV"); break;
        default:      Serial.println("UNKNOWN"); break;
    }
}


void
BKTThrottleDelegate::receivedSpeedSteps(int steps)
{
    Serial.print("speed steps: "); Serial.println(steps);
}

void
BKTThrottleDelegate::receivedWebPort(int port)
{
    Serial.print("web port: "); Serial.println(port);
}


void
BKTThrottleDelegate::receivedTrackPower(TrackPower state)
{
    Serial.print("track power: ");
    switch (state) {
        case PowerOff: Serial.println("OFF"); break;
        case PowerOn:  Serial.println("ON"); break;
        default:       Serial.println("UNKNOWN"); break;
    }
}
