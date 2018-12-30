#include "ThrottleController.h"

#include <FunctionalInterrupt.h>

using namespace std::placeholders;   // for std::bind


// if no Wifi connection within this amount of time, stop
// the connection attempt and just scan for networks
#define WIFI_CONNECTION_TIMEOUT (15000)  // ms

// rescan for new networks every this often
#define WIFI_RETRY_DELAY_TIME  (15000) // ms


ThrottleController::ThrottleController():
    client(),
    hw(),
    wiThrottle(),
    port(12090),
    wifiService(flashData),
    bleServer(NULL),
    flashData(),
    restartWifiOnNextCycle(false),
    wifiRetryCheck()
{
    // hw.console->println("ThrottleController constructed");
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
            hw.console->println("now TSTATE_UNKNOWN");
            wifiService.setConnectionState("UNKNOWN");
            hw.setRGB(0, 0x00, 0x00, 0x00);
            break;
        case TSTATE_WIFI_DISCONNECTED:
            hw.console->println("TSTATE_WIFI_DISCONNECTED");
            wifiService.setConnectionState("WIFI_DISCONNECTED");
            hw.setRGB(0, 0xFF, 0x00, 0x00);
            break;
        case TSTATE_WIFI_CONNECTED:
            hw.console->println("TSTATE_WIFI_CONNECTED");
            wifiService.setConnectionState("WIFI_CONNECTED");
            hw.setRGB(0, 0xFF, 0x00, 0xFF);
            break;
        case TSTATE_WITHROTTLE_CONNECTED:
            hw.console->println("TSTATE_WITHROTTLE_CONNECTED");
            wifiService.setConnectionState("WITHROTTLE_CONNECTED");
            hw.setRGB(0, 0x00, 0xFF, 0x00);
            break;
        case TSTATE_WITHROTTLE_ACTIVE:
            hw.console->println("TSTATE_WITHROTTLE_ACTIVE");
            hw.setRGB(0, 0x00, 0x00, 0x80);
            wifiService.setConnectionState("WITHROTTLE_ACTIVE");
            break;
        default:
            hw.console->println("change to ___UNDEFINED___ TSTATE value ");
            wifiService.setConnectionState("** UNDEFINED **");
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
        hw.console->print("accel\tX: "); hw.console->print(event.acceleration.x);
        hw.console->print(" \tY: "); hw.console->print(event.acceleration.y);
        hw.console->print(" \tZ: "); hw.console->print(event.acceleration.z);
        hw.console->println(" m/s^2 ");

        last_x = event.acceleration.x;
        last_y = event.acceleration.y;
        last_z = event.acceleration.z;
    }
}

#endif


void
ThrottleController::setupBLE()
{
    BLEDevice::init(flashData.getDeviceName());
    BLEAddress addr = BLEDevice::getAddress();

    hw.console->print("BLE Address is ");
    hw.console->println(addr.toString().c_str());

    // set up the BLE services
    bleServer = BLEDevice::createServer();

    wifiService.begin(bleServer, hw.console);
    throttleService.begin(bleServer, hw.console);
}

bool
ThrottleController::begin()
{
    hw.begin();

    wiThrottle.begin(hw.console);
    flashData.begin(hw.console);

    setupBLE();

    wiThrottle.delegate      = this;    // set up callbacks for various WiThrottle activities
    wifiService.delegate     = this;    // appropriate callbacks for BLE Wifi service
    throttleService.delegate = this;  // callbacks for the throttleService
    hw.delegate              = this;    // and for hardware changes

    hw.console->println("ThrottleController.begin complete");
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

    // BLE is not connected at this time, nor is WiFI
    setThrottleState(TSTATE_WIFI_DISCONNECTED);

    hw.console->println("wifi is disconnected");

    WiFi.mode(WIFI_MODE_STA);
    wifiService.setDeviceMac(WiFi.macAddress().c_str());

    WiFi.disconnect();
    WiFi.onEvent(std::bind(&ThrottleController::wifiEvent, this, _1));
    delay(100);

    std::string ssid = flashData.getWifiSSID();
    std::string password = flashData.getWifiPassword();

    hw.console->printf("Connecting to Wifi SSID:'%s' Password:'%s'\n", ssid.c_str(), password.c_str());

    WiFi.begin(ssid.c_str(), password.c_str());
    bool connectionBegun = true;

    while (WiFi.status() != WL_CONNECTED) {
        hw.check();
        if (restartWifiOnNextCycle) {
            goto end;
        }

        if (connectionBegun) {
            if (wifiRetryCheck.hasPassed(WIFI_CONNECTION_TIMEOUT)) {
                wifiRetryCheck.restart();
                WiFi.disconnect();
                delay(100);
                connectionBegun = false;
            }
        }
        else if (wifiRetryCheck.hasPassed(WIFI_RETRY_DELAY_TIME)) {
            wifiRetryCheck.restart();
            wifiService.scanNetworks();
        }
    }

    // light blue when connected to WiThrottle server
    setThrottleState(TSTATE_WIFI_CONNECTED);

    hw.console->println("wifi connected");

    while (! client.connected()) {
        hw.check();
        std::string host = flashData.getServerAddress();
        if (!client.connect(host.c_str(), port)) {
            hw.console->printf("connection to %s:%d failed\n", host.c_str(), port);
        }
        else {
            hw.console->println("connection succeeded");
            client.setNoDelay(true); // disable Nagle & packet coalescing
            wiThrottle.connect(&client);
        }
    }

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
            if (!addressSelected && selectedAddress != "") {
                addressSelected = wiThrottle.addLocomotive(selectedAddress);

                std::string sa = selectedAddress.c_str();
                throttleService.setSelectedAddress(sa);
            }
#endif
        }

        if (restartWifiOnNextCycle) {
            break;
        }
    }


  end:
    restartWifiOnNextCycle = false;
    delay(3000);
}



void
ThrottleController::receivedVersion(String version)
{
    hw.console->print("received protocol version string ");
    hw.console->println(version);
    setThrottleState(TSTATE_WITHROTTLE_CONNECTED);
}


// this is called by the WiThrottle controller when the notification of a
// function state change is received.  By changing any externally visble
// values at this point, we complete a feedback loop (at the expense of a
// slightly increased latency on the indication change).
void
ThrottleController::receivedFunctionState(uint8_t func, bool state)
{
    hw.console->printf("display function state F%d: %d\n", func, state);

    hw.setLight(func, state == 0 ? 0 : 255);

    // do something with hw.<xyz?> to indicate the function state
    return;
}


void
ThrottleController::receivedSpeed(int speed)
{
    hw.console->print("speed value "); hw.console->println(speed);
}


void
ThrottleController::receivedDirection(Direction dir)
{
    hw.console->print("direction is ");
    switch(dir) {
        case Forward: hw.console->println("FWD"); break;
        case Reverse: hw.console->println("REV"); break;
        default:      hw.console->println("UNKNOWN"); break;
    }
}


void
ThrottleController::receivedSpeedSteps(int steps)
{
    hw.console->print("speed steps: "); hw.console->println(steps);
}


void
ThrottleController::receivedWebPort(int port)
{
    hw.console->print("web port: "); hw.console->println(port);
}


void
ThrottleController::receivedTrackPower(TrackPower state)
{
    hw.console->print("track power: ");
    switch (state) {
        case PowerOff: hw.console->println("OFF"); break;
        case PowerOn:  hw.console->println("ON"); break;
        default:       hw.console->println("UNKNOWN"); break;
    }
}



// call this every time that fast time value has changed (should be every
// real second or so), even if the displayed fast time value doesn't change
// (we want to keep the colon blinking once per second)

void
ThrottleController::updateFastTimeDisplay()
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
  hw.console->println("STA Connected");
  hw.console->print("STA IPv4: ");
  hw.console->println(WiFi.localIP());

  wifiService.setDeviceAddress(WiFi.localIP());
  wifiService.setDeviceNetmask(WiFi.subnetMask());
  wifiService.setDeviceGateway(WiFi.gatewayIP());

  hw.console->printf("connecting to %s:%s\n",
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
      WiFi.setHostname(flashData.getDeviceName().c_str());

      hw.console->print("MAC: "); hw.console->println( WiFi.macAddress().c_str() );
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
    hw.console->print("wifi command received "); hw.console->print(command.c_str()); hw.console->println("");

    hw.console->printf("  ssid: '%s'\n", flashData.getWifiSSID().c_str());
    hw.console->printf("  password: '%s'\n", flashData.getWifiPassword().c_str());
    hw.console->printf("  server: '%s:%s'\n", flashData.getServerAddress().c_str(), flashData.getServerPort().c_str());

    restartWifiOnNextCycle = true;
}


void
ThrottleController::updateDirection(TogglePosition togglePosition)
{
    throttleService.setTogglePosition(togglePosition);

    // Do not change direction when the toggle is CENTER OFF
    //
    if (togglePosition == Left || togglePosition == Right) {
        Direction dir = directionFromTogglePosition(togglePosition);
        if (dir != wiThrottle.getDirection()) {
            wiThrottle.setDirection(dir);
            throttleService.setDirection(dir);
        }
    }

}

void
ThrottleController::speedChanged(int newSpeed, TogglePosition togglePosition)
{
    updateDirection(togglePosition);

    wiThrottle.setSpeed(newSpeed);
    throttleService.setSpeed(newSpeed);
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
    hw.console->printf(">>> Battery Level: %d\n", batteryLevel);
}


// this is called by the HW module when the function button itself changes state
void
ThrottleController::functionButtonChanged(int func, bool pressed)
{
    hw.console->printf("** button F%d changed to %s\n", func, pressed ? "PRESSED" : "RELEASED");

    wiThrottle.setFunction(func, pressed);
}


// this is called by the ThrottleService when the address is set
void
ThrottleController::throttleAddressChanged(std::string address)
{
    hw.console->printf("** address should be changed to %s\n", address.c_str());
    selectedAddress = String(address.c_str());
}
