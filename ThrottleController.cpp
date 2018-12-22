#include "ThrottleController.h"

#include <FunctionalInterrupt.h>

#include "BSP.h"




using namespace std::placeholders;

ThrottleController::ThrottleController(ThrottleData& flashData):
    client(),
    clockDisplay(),
    accel(),
    sx1509(),
    pilotLight(),
    statusLED(sx1509, RGB_RED, RGB_GREEN, RGB_BLUE),
    wiThrottle(),
    previousTogglePosition(Unknown),
    speedAccumulator(0),
    speedCount(0),
    previousSpeedValue(-1),
    penultimateSpeedValue(-1),
    handleSX1509Interrupt(false),
    handleAccelInterrupt(false),
    port(12090),
    wifiInfo(flashData),
    bleServer(NULL),
    flashData(flashData),
    restartWifiOnNextCycle(false)
{
    Serial.print("ThrottleController constructed");
}


void
ThrottleController::accel_isr()
{
    handleAccelInterrupt = true;
    accelIntrValue = digitalRead(ACCEL_INTR_PIN);
}

void
ThrottleController::setupAccelerometer()
{
    attachInterrupt(ACCEL_INTR_PIN, std::bind(&ThrottleController::accel_isr, this), CHANGE);

    Serial.println("accelerometer initialized");
}


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
        Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
        Serial.print(" \tY: "); Serial.print(event.acceleration.y);
        Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
        Serial.println(" m/s^2 ");

        last_x = event.acceleration.x;
        last_y = event.acceleration.y;
        last_z = event.acceleration.z;
    }
}



void
ThrottleController::sx1509_isr()
{
    handleSX1509Interrupt = true;
}


void
ThrottleController::setupSX1509()
{
    pilotLight.begin(21);
    statusLED.begin();

    // The SX1509 has built-in debounce features, so a single button-press
    // doesn't accidentally create multiple ints.  Use
    // .debounceTime(<time_ms>) to set the GLOBAL SX1509 debounce time.
    //
    // <time_ms> can be either 0, 1, 2, 4, 8, 16, 32, or 64 ms.
    sx1509.debounceTime(16);

#ifdef BRAKE
    setupButtonPin(BRAKE);
#endif
#ifdef BUTTON1
    setupButtonPin(BUTTON1);
#endif
#ifdef BUTTON2
    setupButtonPin(BUTTON2);
#endif
#ifdef BUTTON3
    setupButtonPin(BUTTON3);
#endif
#ifdef BUTTON4
    setupButtonPin(BUTTON4);
#endif
#ifdef BUTTON5
    setupButtonPin(BUTTON5);
#endif
#ifdef BUTTON6
    setupButtonPin(BUTTON6);
#endif
#ifdef BUTTON7
    setupButtonPin(BUTTON7);
#endif
#ifdef BUTTON8
    setupButtonPin(BUTTON8);
#endif

#ifdef LED1
    setupLEDPin(LED1);
    Serial.println("LED1 setup");
#endif
#ifdef LED2
    setupLEDPin(LED2);
#endif
#ifdef LED3
    setupLEDPin(LED3);
#endif
#ifdef LED4
    setupLEDPin(LED4);
#endif
#ifdef LED5
    setupLEDPin(LED5);
#endif
#ifdef LED6
    setupLEDPin(LED6);
#endif
#ifdef LED7
    setupLEDPin(LED7);
#endif

    // Attach an Arduino interrupt to the interrupt pin. Call the ISR
    // function, whenever the pin goes from HIGH to LOW.
    attachInterrupt(SX1509_INTR_PIN, std::bind(&ThrottleController::sx1509_isr, this), FALLING);

    Serial.println("SX1509 initialized");
}


bool
ThrottleController::begin()
{
    if (! accel.begin(ACCEL_I2C_ADDRESS)) {
        Serial.printf("Unable to initialize accelerometer at 0x18\n");

        accel.setRange(LIS3DH_RANGE_4_G);
    }


  bleServer = BLEDevice::createServer();
#if ADAFRUIT_ALPHANUM
  clockDisplay.begin(ALPHANUM_DISPLAY_ADDRESS);
  clockDisplay.clear();
  clockDisplay.writeDigitAscii(0, '0');
  clockDisplay.writeDigitAscii(1, '0');
  clockDisplay.writeDigitAscii(2, '0');
  clockDisplay.writeDigitAscii(3, '0');
#else
  clockDisplay.begin(DISPLAY_ADDRESS);#
  clockDisplay.clear();
#endif
  clockDisplay.writeDisplay();

  // Call .begin(<address>) to initialize the SX1509. If it successfully
  // communicates, it'll return true.
  if (!sx1509.begin(SX1509_I2C_ADDRESS))
  {
      Serial.println("no sx1509 found");
      while (1) {
	  // If we fail to communicate, loop forever.
      }
    // ideally this should do some sort of external notification
    // this might be a good reason to put the status LED on GPIO's on the ESP32
  }

  setupSX1509();

  setupAccelerometer();

  pinMode(DIR_LEFT, INPUT_PULLUP);
  pinMode(DIR_RIGHT, INPUT_PULLUP);

  analogReadResolution(12);   // 0-4095, no matter what the hardware support

  wifiInfo.begin(bleServer);
  throttleInfo.begin(bleServer);

  wiThrottle.delegate = this;    // set up callbacks for various WiThrottle activities
  wifiInfo.delegate = this;      // appropriate callbacks for BLE Wifi info

  Serial.println("ThrottleController.begin complete");
}




void
ThrottleController::readSpeedPot()
{
    int rawSpeedValue = analogRead(SPEED_KNOB);
    int reading = map(rawSpeedValue, 0, 4095, 0, 126);

    speedAccumulator += reading;
    speedCount += 1;

}



void
ThrottleController::readSpeed()
{
  bool speedChanged = false;
  bool togglePositionChanged = false;

  if (speedCount == 0) {
      // no data has been accumulated, so don't do anything...
      return;
  }

  int speedValue = speedAccumulator / speedCount;

  TogglePosition togglePosition = readTogglePosition();

  if (togglePosition == CenterOff) {
      // center off position
      speedValue = 0;
      if (previousSpeedValue != 0) {
          previousSpeedValue = penultimateSpeedValue = 0;
          speedChanged = true;
      }
  }

  if (togglePosition != previousTogglePosition) {
    togglePositionChanged = true;
    previousTogglePosition = togglePosition;
  }


  if (speedValue != previousSpeedValue && speedValue != penultimateSpeedValue) {
    penultimateSpeedValue = previousSpeedValue;
    previousSpeedValue = speedValue;
    speedChanged = true;
  }

#if 0
  Serial.print("speedAccumulator:"); Serial.print(speedAccumulator);
  Serial.print("  speedValue:"); Serial.print(speedValue);
  Serial.print("  previous:"); Serial.print(previousSpeedValue);
  Serial.print("  penultimate:"); Serial.print(penultimateSpeedValue);
  Serial.print("  speedChanged:"); Serial.print(speedChanged ? "true" : "false");
  Serial.print("  toggleChanged:"); Serial.println(togglePositionChanged ? "true" : "false");
#endif


  if (speedChanged || togglePositionChanged) {
#if 0
    Serial.print("SPEED = (raw)"); Serial.print(rawSpeedValue);
    Serial.print(" (mapped)"); Serial.print(speedValue);
    switch (togglePosition) {
      case CenterOff:
        Serial.println(" CENTER OFF");
        break;
      case Left:
        Serial.println(" LEFT");
        break;
      case Right:
        Serial.println(" RIGHT");
        break;
      default:
        Serial.println(" SHOULDN'T HAPPEN");
        break;
    }
#endif

    if (togglePositionChanged && (togglePosition==Left || togglePosition==Right)) {
      // the WiThrottle protocol only does Fwd & Reverse, so any other position
      // just gets ignored at this time.
      static Direction lastDirection = Forward;
      Direction d = directionFromTogglePosition(togglePosition);

      if (d != lastDirection) {
        wiThrottle.setDirection(d);
        throttleInfo.setDirection(d);
        lastDirection = d;
      }

    }
    if (speedChanged) {
      wiThrottle.setSpeed(speedValue);
      throttleInfo.setSpeed(speedValue);
    }
  }


  speedAccumulator = speedCount = 0;
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
    statusLED.setWifiDisconnected();
    wifiInfo.setConnectionState("WIFI_DISCONNECTED");

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
    statusLED.setWifiConnected();
    wifiInfo.setConnectionState("WIFI_CONNECTED");

    Serial.println("wifi connected");

    while (! client.connected()) {
	pilotLight.check();
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
    statusLED.setThrottleConnected();
    wifiInfo.setConnectionState("WITHROTTLE_CONNECTED");

    while (true) {
        pilotLight.check();

        if (wiThrottle.check()) {
            if (wiThrottle.clockChanged) {
                updateFastTimeDisplay();
            }
            if (wiThrottle.heartbeatChanged) {
                wiThrottle.requireHeartbeat();
            }
            if (! client.connected()) {
                statusLED.setWifiDisconnected();
                wifiInfo.setConnectionState("WIFI_DISCONNECTED");
                wiThrottle.disconnect();
                Serial.println("Network Disconnected");
                return;
            }

            if (handleSX1509Interrupt) {
                readButtons();
                handleSX1509Interrupt = false;
            }

            if (handleAccelInterrupt) {
                Serial.printf("ACCEL CHANGE: now %d\n", accelIntrValue);
            }

            if (accelerometerCheck.hasPassed(1000/5)) {
                readAccelerometer();
            }

#if 1
            if (!nameSent) {
                wiThrottle.setDeviceName("mylittlethrottle");
                nameSent = true;
            }
#endif


#if 1
            if (!addressSelected) {
                addressSelected = wiThrottle.addLocomotive(selectedAddress);
            }
#endif

            if (speedPotRead.hasPassed(1000 / (15 * 4))) {
                speedPotRead.restart();
                readSpeedPot();
            }

            if (speedCheck.hasPassed(1000 / 15)) {
                speedCheck.restart();
                readSpeed();
            }
        }

        if (restartWifiOnNextCycle) {
            break;
        }
    }


    restartWifiOnNextCycle = false;

}


void
ThrottleController::setupButtonPin(int pin)
{
    Serial.print("setup BUTTON on pin "); Serial.println(pin);
    sx1509.pinMode(pin, INPUT_PULLUP);
    sx1509.enableInterrupt(pin, CHANGE);
    sx1509.debouncePin(pin);
}


void
ThrottleController::setupLEDPin(int pin)
{
    sx1509.pinMode(pin, OUTPUT);
    sx1509.digitalWrite(pin, SX1509_OFF);
}



void
ThrottleController::receivedVersion(String version)
{
    Serial.print("received protocol version string ");
    Serial.println(version);
}


void
ThrottleController::receivedFunctionState(uint8_t func, bool state)
{
    int pin = -1;
    state = !state;;   // inverted outputs

    // TODO: these mappings should be part of user-controllable configuration
    switch(func) {
        case 0:
            pin = 5;
            break;
        case 1:
            pin = 4;
            break;
        case 2:
            pin = 3;
            break;
        case 3:
            pin = 2;
            break;
        case 4:
            pin = 1;
            break;
        case 9:
            pin = 6;
            break;
        default:
            break;
    }

    if (pin != -1) {
        Serial.printf("setting LED %d to %d\n", pin, state);
        sx1509.digitalWrite(pin, state);
    }
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



void
ThrottleController::readButton(int intrStatus, int buttonPin, int funcNum, const char *name)
{
  if (intrStatus & (1 << buttonPin)) {
    int pressed = !sx1509.digitalRead(buttonPin);
    Serial.print(name); Serial.print(" BUTTON: ");
    Serial.println(pressed ? "PRESSED" : "RELEASED");

    wiThrottle.setFunction(funcNum, pressed ? true : false);
  }
}


/* This function gets called whenever the interrupt handler indicates that
 * we need to process a button change event from the SX1509.  Only those
 * buttons that have changed will be indicated in the interrupt source,
 * so we read the new value and send the function state accordingly.
 */
void
ThrottleController::readButtons()
{
  unsigned int intrStatus = sx1509.interruptSource();
  // For debugging handiness, print the intStatus variable.
  Serial.print("button interrupt: "); Serial.print(intrStatus, BIN); Serial.println("");

  // Each bit in intStatus represents a single SX1509 I/O.  Check all thet
  // we know about each time we get notified of a change.

  // TODO: put the buttons into a map (name:pin)
  readButton(intrStatus, BRAKE,   9, "BRAKE");

#ifdef BUTTON1
  readButton(intrStatus, BUTTON1, 0, "BUTTON1");
#endif
#ifdef BUTTON2
  readButton(intrStatus, BUTTON2, 1, "BUTTON2");
#endif
#ifdef BUTTON3
  readButton(intrStatus, BUTTON3, 2, "BUTTON3");
#endif
#ifdef BUTTON4
  readButton(intrStatus, BUTTON4, 3, "BUTTON4");
#endif
#ifdef BUTTON5
  readButton(intrStatus, BUTTON5, 4, "BUTTON5");
#endif
#ifdef BUTTON6
  readButton(intrStatus, BUTTON6, 5, "BUTTON6");
#endif
#ifdef BUTTON7
  readButton(intrStatus, BUTTON7, 6, "BUTTON7");
#endif
#ifdef BUTTON8
  readButton(intrStatus, BUTTON8, 7, "BUTTON8");
#endif
}




// call this every time that fast time value has changed (should be every
// real second or so), even if the displayed fast time value doesn't change
// (we want to keep the colon blinking once per second)

void ThrottleController::updateFastTimeDisplay()
{
    static int blinkColon = SX1509_OFF;
    int hour = wiThrottle.fastTimeHours();
    int minutes = wiThrottle.fastTimeMinutes();

    // Show the time on the display by turning it into a numeric
    // value, like 3:30 turns into 330, by multiplying the hour by
    // 100 and then adding the minutes.
    int displayValue = hour * 100 + minutes;


    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (hour > 12) {
        displayValue -= 1200;
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (hour == 0) {
        displayValue += 1200;
    }


    blinkColon = !blinkColon;

#if ! ADAFRUIT_ALPHANUM
    // Now print the time value to the display.
    clockDisplay.print(displayValue, DEC);

    // Blink the colon by flipping its value every loop iteration
    // (which happens every second).
    clockDisplay.drawColon(blinkColon);
#endif
#ifdef LED1
    sx1509.digitalWrite(LED1, blinkColon ? SX1509_ON : SX1509_OFF);
#endif

    clockDisplay.writeDisplay();
}



Direction
ThrottleController::directionFromTogglePosition(TogglePosition position)
{
  Direction value = Forward;

  if (position == Left) {
    value = Reverse;
  }

  Serial.print("TogglePosition "); Serial.print(position); Serial.print(" => Direction "); Serial.println(value);
  return value;
}


TogglePosition
ThrottleController::readTogglePosition()
{
    TogglePosition togglePosition = Unknown; // This should never happen

    int d1 = !digitalRead(DIR_LEFT);
    int d2 = !digitalRead(DIR_RIGHT);

    if (!d1 && !d2) {
        togglePosition = CenterOff;
    }
    else if (d1) {
        togglePosition = Left;
    }
    else if (d2) {
        togglePosition = Right;
    }

    return togglePosition;
}




void
ThrottleController::wifiOnConnect() {
  Serial.println("STA Connected");
  Serial.print("STA IPv4: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to ");
  Serial.println(flashData.getServerAddress().c_str());
}

void
ThrottleController::wifiOnDisconnect() {
  client.stop();
  wiThrottle.disconnect();
  statusLED.setWifiDisconnected();
  wifiInfo.setConnectionState("WIFI_DISCONNECTED");
  Serial.println("STA Disconnected");
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
    Serial.printf("  server: '%s'\n", flashData.getServerAddress().c_str());

    restartWifiOnNextCycle = true;
}
