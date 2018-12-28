#include "ThrottleController.h"

#include <FunctionalInterrupt.h>

using namespace std::placeholders;   // for std::bind



ThrottleController::ThrottleController(ThrottleData& flashData):
    client(),
    hw(),
    wiThrottle(),
    port(12090),
    wifiInfo(flashData),
    bleServer(NULL),
    flashData(flashData),
    restartWifiOnNextCycle(false)
{
    Serial.print("ThrottleController constructed");
}


void
ThrottleController::setThrottleState(ThrottleState newState)
{
    if (newState == currentThrottleState) {
        // no change has occurred, do nothing at this point
        return;
    }

    // we have a new state
    switch (newState) {
        case TSTATE_UNKNOWN:
            Serial.println("now TSTATE_UNKNOWN");
            wifiInfo.setConnectionState("UNKNOWN");
            hw.setRGB(0, 0x00, 0x00, 0x00);
            break;
        case TSTATE_WIFI_DISCONNECTED:
            Serial.println("TSTATE_WIFI_DISCONNECTED");
            wifiInfo.setConnectionState("WIFI_DISCONNECTED");
            hw.setRGB(0, 0xFF, 0x00, 0x00);
            break;
        case TSTATE_WIFI_CONNECTED:
            Serial.println("TSTATE_WIFI_CONNECTED");
            wifiInfo.setConnectionState("WIFI_CONNECTED");
            hw.setRGB(0, 0x00, 0x80, 0x00);
            break;
        case TSTATE_WITHROTTLE_CONNECTED:
            Serial.println("TSTATE_WITHROTTLE_CONNECTED");
            wifiInfo.setConnectionState("WITHROTTLE_CONNECTED");
            hw.setRGB(0, 0x00, 0xFF, 0x00);
            break;
        case TSTATE_WITHROTTLE_ACTIVE:
            Serial.println("TSTATE_WITHROTTLE_ACTIVE");
            hw.setRGB(0, 0x80, 0x80, 0x80);
            wifiInfo.setConnectionState("WITHROTTLE_ACTIVE");
            break;
        default:
            Serial.println("change to ___UNDEFINED___ TSTATE value ");
            wifiInfo.setConnectionState("** UNDEFINED **");
            break;
    }

    currentThrottleState = newState;
}


#if 0
void
ThrottleController::readAccelerometer()
{
    const float ACCEL_DELTA = 0.75;
    static float last_x, last_y, last_z = 0;
    accel.read();

    sensors_event_t event;
    accel.getEvent(&event);

    float dx, dy, dz;
    dx = fabs(event.acceleration.x - last_x);
    dy = fabs(event.acceleration.y - last_y);
    dz = fabs(event.acceleration.z - last_z);

    if ((dx > ACCEL_DELTA) || (dy > ACCEL_DELTA) || (dz > ACCEL_DELTA)) {
        /* Display the results (acceleration is measured in m/s^2) */
        Serial.print("accel\tX: "); Serial.print(event.acceleration.x);
        Serial.print(" \tY: "); Serial.print(event.acceleration.y);
        Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
        Serial.println(" m/s^2 ");

        last_x = event.acceleration.x;
        last_y = event.acceleration.y;
        last_z = event.acceleration.z;
    }
}

#endif




bool
ThrottleController::begin()
{
    hw.begin();

    setThrottleState(TSTATE_WIFI_DISCONNECTED);

    bleServer = BLEDevice::createServer();

    wifiInfo.begin(bleServer);
    throttleInfo.begin(bleServer);

    wiThrottle.delegate = this;    // set up callbacks for various WiThrottle activities
    wifiInfo.delegate   = this;    // appropriate callbacks for BLE Wifi info
    hw.delegate         = this;    // and for hardware changes

    Serial.println("ThrottleController.begin complete");
}



void
ThrottleController::test_loop()
{
    hw.setRGB(0, 0x00, 0x00, 0x00);
    delay(500);

    hw.setRGB(0, 0xFF, 0x00, 0x00);
    delay(500);

    hw.setRGB(0, 0x00, 0xFF, 0x00);
    delay(500);

    hw.setRGB(0, 0x00, 0x00, 0xFF);
    delay(500);

    for (int i = 0x00; i < 0x100; i += 0x8) {
        hw.setRGB(0, i, i, i);
        delay(20);
    }


}




// This loop waits for the WiFi connection, and the WiThrottle connection
// once that's lost, it exits and restarts

// unlike the usual Arduino loop() which is called many many times a second
// this one contains its own tight repeat loop for action checks

void
ThrottleController::loop()
{
    bool addressSelected = false;
    bool nameSent = false;
    String selectedAddress = "S23";


    delay(10000);

//    clockDisplay.clear();
//    clockDisplay.writeDisplay();

    // BLE is not connected at this time, nor is WiFI
    setThrottleState(TSTATE_WIFI_DISCONNECTED);

    Serial.println("wifi is disconnected");

    WiFi.disconnect();
    WiFi.onEvent(std::bind(&ThrottleController::wifiEvent, this, _1));
    WiFi.mode(WIFI_MODE_STA);

    std::string ssid = flashData.getWifiSSID();
    std::string password = flashData.getWifiPassword();

    Serial.printf("Connecting to Wifi SSID:'%s' Password:'%s'\n", ssid.c_str(), password.c_str());

    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        static int retryCount = 0;
        delay(500);
        Serial.println("retry wifi connection");
        if ((retryCount++ % 65) == 0) {
            Serial.println("");
        }
    }

    // light blue when connected to WiThrottle server
    setThrottleState(TSTATE_WIFI_CONNECTED);

    Serial.println("wifi connected");

    while (! client.connected()) {
        hw.check();
        std::string host = flashData.getServerAddress();
        if (!client.connect(host.c_str(), port)) {
            Serial.printf("connection to %s:%d failed\n", host.c_str(), port);
        }
        else {
            Serial.println("connection succeeded");
            client.setNoDelay(true); // disable Nagle & packet coalescing
            wiThrottle.connect(&client);
        }
        delay(1000);
        Serial.print(":");
    }

    // bright blue when connected to WiThrottle server
    setThrottleState(TSTATE_WITHROTTLE_CONNECTED);

    while (true) {
        hw.check();

        if (wiThrottle.check()) {
            if (wiThrottle.clockChanged) {
                updateFastTimeDisplay();
            }
            if (wiThrottle.heartbeatChanged) {
                wiThrottle.requireHeartbeat();
            }
            if (! client.connected()) {
                setThrottleState(TSTATE_WIFI_DISCONNECTED);
                wiThrottle.disconnect();
                return;
            }

            if (!nameSent) {
                wiThrottle.setDeviceName(flashData.getDeviceName().c_str());
                nameSent = true;
            }

#if 1
            if (!addressSelected) {
                addressSelected = wiThrottle.addLocomotive(selectedAddress);
            }
#endif
        }

        if (restartWifiOnNextCycle) {
            break;
        }
    }


    restartWifiOnNextCycle = false;

}



void
ThrottleController::receivedVersion(String version)
{
    Serial.print("received protocol version string ");
    Serial.println(version);
    setThrottleState(TSTATE_WITHROTTLE_ACTIVE);
}


// this is called by the WiThrottle controller when the notification of a
// function state change is received.  By changing any externally visble
// values at this point, we complete a feedback loop (at the expense of a
// slightly increased latency on the indication change).
void
ThrottleController::receivedFunctionState(uint8_t func, bool state)
{
    Serial.printf("display function state F%d: %d\n", func, state);

    hw.setLight(func, state == 0 ? 0 : 255);

    // do something with hw.<xyz?> to indicate the function state
    return;
}


void
ThrottleController::receivedSpeed(int speed)
{
    Serial.print("speed value "); Serial.println(speed);
}


void
ThrottleController::receivedDirection(Direction dir)
{
    Serial.print("direction is ");
    switch(dir) {
        case Forward: Serial.println("FWD"); break;
        case Reverse: Serial.println("REV"); break;
        default:      Serial.println("UNKNOWN"); break;
    }
}


void
ThrottleController::receivedSpeedSteps(int steps)
{
    Serial.print("speed steps: "); Serial.println(steps);
}


void
ThrottleController::receivedWebPort(int port)
{
    Serial.print("web port: "); Serial.println(port);
}


void
ThrottleController::receivedTrackPower(TrackPower state)
{
    Serial.print("track power: ");
    switch (state) {
        case PowerOff: Serial.println("OFF"); break;
        case PowerOn:  Serial.println("ON"); break;
        default:       Serial.println("UNKNOWN"); break;
    }
}



// call this every time that fast time value has changed (should be every
// real second or so), even if the displayed fast time value doesn't change
// (we want to keep the colon blinking once per second)

void ThrottleController::updateFastTimeDisplay()
{
    int hour = wiThrottle.fastTimeHours();
    int minutes = wiThrottle.fastTimeMinutes();

    hw.setTimeDisplay(hour, minutes);

    float rate = wiThrottle.fastTimeRate();
    TimeStatus state = (rate == 0.0f) ? Paused : Running;
    hw.setTimeStatus(state);
}



Direction
ThrottleController::directionFromTogglePosition(TogglePosition position)
{
  Direction value = Forward;

  if (position == Left) {
    value = Reverse;
  }

  return value;
}






void
ThrottleController::wifiOnConnect() {
  Serial.println("STA Connected");
  Serial.print("STA IPv4: ");
  Serial.println(WiFi.localIP());

  Serial.printf("connecting to %s:%s\n",
                flashData.getServerAddress().c_str(),
                flashData.getServerPort().c_str());
}

void
ThrottleController::wifiOnDisconnect() {
  client.stop();
  wiThrottle.disconnect();
  setThrottleState(TSTATE_WIFI_DISCONNECTED);
}

void
ThrottleController::wifiEvent(WiFiEvent_t event) {
  switch (event) {

    case SYSTEM_EVENT_STA_START:
      //set sta hostname here
      WiFi.setHostname("mylittlethrottle");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      wifiOnConnect();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      wifiOnDisconnect();
      break;
    default:
      break;
  }
}


void
ThrottleController::wifiCommandReceived(std::string command)
{
    Serial.print("wifi command received "); Serial.print(command.c_str()); Serial.println("");

    Serial.printf("  ssid: '%s'\n", flashData.getWifiSSID().c_str());
    Serial.printf("  password: '%s'\n", flashData.getWifiPassword().c_str());
    Serial.printf("  server: '%s:%s'\n", flashData.getServerAddress().c_str(), flashData.getServerPort().c_str());

    restartWifiOnNextCycle = true;
}


void
ThrottleController::updateDirection(TogglePosition togglePosition)
{
    // Do not change direction when the toggle is CENTER OFF
    //
    if (togglePosition == Left || togglePosition == Right) {
        Direction dir = directionFromTogglePosition(togglePosition);
        if (dir != wiThrottle.getDirection()) {
            wiThrottle.setDirection(dir);
        }
    }

}

void
ThrottleController::speedChanged(int newSpeed, TogglePosition togglePosition)
{
    updateDirection(togglePosition);

    wiThrottle.setSpeed(newSpeed);
}


void
ThrottleController::togglePositionChanged(TogglePosition newPosition)
{
    updateDirection(newPosition);
}


void
ThrottleController::throttleMoved()
{
}


void
ThrottleController::throttleFell()
{
}


void
ThrottleController::batteryLevelChanged(int batteryLevel)
{
    // by calling the delegate, the HW controller module has determined
    // that this batteryLevel is "of interest" and should be reported
    // to all interested parties
    Serial.printf(">>> Battery Level: %d\n", batteryLevel);
}


// this is called by the HW module when the function button itself changes state
void
ThrottleController::functionButtonChanged(int func, bool pressed)
{
    Serial.printf("** button F%d changed to %s\n", func, pressed ? "PRESSED" : "RELEASED");

    wiThrottle.setFunction(func, pressed);
}
