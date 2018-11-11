#include "ThrottleController.h"

#include <FunctionalInterrupt.h>

#define WIFI_SSID     "myownlittleidaho"
#define WIFI_PASSWORD "goslowdown"

#define JMRI_SERVER_ADDRESS "192.168.22.20"

ThrottleController::ThrottleController():
    client(),
    clockDisplay(),
    sx1509(),
    statusLED(sx1509, RGB_RED, RGB_GREEN, RGB_BLUE),
    wiThrottle(),
    previousTogglePosition(Unknown),
    speedAccumulator(0),
    speedCount(0),
    previousSpeedValue(-1),
    penultimateSpeedValue(-1),
    handleSX1509Interrupt(false),
    ssid(WIFI_SSID),
    password(WIFI_PASSWORD),
    host(JMRI_SERVER_ADDRESS),
    port(12090)
{
}


void
ThrottleController::sx1509_isr()
{
    handleSX1509Interrupt = true;
}


bool
ThrottleController::begin()
{

  clockDisplay.begin(DISPLAY_ADDRESS);
  clockDisplay.clear();

  // Call .begin(<address>) to initialize the SX1509. If it successfully
  // communicates, it'll return true.
  if (!sx1509.begin(SX1509_ADDRESS))
  {
    while (1) ; // If we fail to communicate, loop forever.
    // ideally this should do some sort of external notification
  }

  Serial.println("SX1509 initialized");

  statusLED.begin();

  // The SX1509 has built-in debounce features, so a single button-press
  // doesn't accidentally create multiple ints.  Use
  // .debounceTime(<time_ms>) to set the GLOBAL SX1509 debounce time.
  //
  // <time_ms> can be either 0, 1, 2, 4, 8, 16, 32, or 64 ms.
  sx1509.debounceTime(16);

  setupButtonPin(BRAKE);
  setupButtonPin(BUTTON1);
  setupButtonPin(BUTTON2);
  setupButtonPin(BUTTON3);
  setupButtonPin(BUTTON4);
  setupButtonPin(BUTTON5);

  sx1509.pinMode(LED1, OUTPUT);
  sx1509.pinMode(LED2, OUTPUT);
  sx1509.pinMode(LED3, OUTPUT);
  sx1509.pinMode(LED4, OUTPUT);
  sx1509.pinMode(LED5, OUTPUT);
  sx1509.pinMode(LED6, OUTPUT);
  sx1509.pinMode(LED7, OUTPUT);

  // Attach an Arduino interrupt to the interrupt pin. Call
  // the button function, whenever the pin goes from HIGH to
  // LOW.
  attachInterrupt(SX1509_INTR_PIN, std::bind(&ThrottleController::sx1509_isr, this), FALLING);

  pinMode(DIR_LEFT, INPUT_PULLUP);
  pinMode(DIR_RIGHT, INPUT_PULLUP);

  analogReadResolution(12);   // 0-4095, no matter what the hardware support

  wiThrottle.delegate = this;
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
        lastDirection = d;
      }

    }
    if (speedChanged) {
      wiThrottle.setSpeed(speedValue);
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
    String selectedAddress = "S21";

    clockDisplay.clear();

    // BLE is not connected at this time, nor is WiFI
    statusLED.setWifiDisconnected();

    WiFi.disconnect();
    WiFi.onEvent(std::bind(&ThrottleController::wifiEvent, this, std::placeholders::_1));
    WiFi.mode(WIFI_MODE_STA);

    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // light blue when connected to WiThrottle server
    statusLED.setWifiConnected();

    while (! client.connected()) {
        if (!client.connect(host.c_str(), port)) {
            Serial.printf("connection to %s:%d failed", host.c_str(), port);
        }
        else {
            Serial.println("connected succeeded");
            client.setNoDelay(true); // disable Nagle & packet coalescing
            wiThrottle.connect(&client);
        }
        delay(1000);
        Serial.print(":");
    }

    // bright blue when connected to WiThrottle server
    statusLED.setThrottleConnected();

    while (true) {
        if (wiThrottle.check()) {
            if (wiThrottle.clockChanged) {
                updateFastTimeDisplay();
            }
            if (wiThrottle.heartbeatChanged) {
                wiThrottle.requireHeartbeat();
            }
            if (! client.connected()) {
                statusLED.setWifiDisconnected();
                wiThrottle.disconnect();
                Serial.println("Network Disconnected");
                return;
            }

            if (handleSX1509Interrupt) {
                readButtons();
                handleSX1509Interrupt = false;
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
    }




}


void
ThrottleController::setupButtonPin(int pin)
{
    sx1509.pinMode(pin, INPUT_PULLUP);
    sx1509.enableInterrupt(pin, CHANGE);
    sx1509.debouncePin(pin);
}



void
ThrottleController::receivedVersion(String version)
{
    Serial.print("received version string ");
    Serial.println(version);
}


void
ThrottleController::receivedFunctionState(uint8_t func, bool state)
{
    state = !state;;   // inverted outputs
    switch(func) {
        case 0:
            sx1509.digitalWrite(5, state);
            break;
        case 1:
            sx1509.digitalWrite(4, state);
            break;
        case 2:
            sx1509.digitalWrite(3, state);
            break;
        case 3:
            sx1509.digitalWrite(2, state);
            break;
        case 4:
            sx1509.digitalWrite(1, state);
            break;
        case 9:
            sx1509.digitalWrite(6, state);
            break;
        default:
            break;
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

  // Each bit in intStatus represents a single SX1509 I/O.  Check all thet
  // we know about each time we get notified of a change.

  // TODO: put the buttons into a map (name:pin)
  readButton(intrStatus, BRAKE,   9, "BRAKE");

  readButton(intrStatus, BUTTON1, 0, "BUTTON1");
  readButton(intrStatus, BUTTON2, 1, "BUTTON2");
  readButton(intrStatus, BUTTON3, 2, "BUTTON3");
  readButton(intrStatus, BUTTON4, 3, "BUTTON4");
  readButton(intrStatus, BUTTON5, 4, "BUTTON5");
}




// call this every time that fast time value has changed (should be every
// real second or so), even if the displayed fast time value doesn't change
// (we want to keep the colon blinking once per second)

void ThrottleController::updateFastTimeDisplay()
{
    static bool blinkColon = true;
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

    // Now print the time value to the display.
    clockDisplay.print(displayValue, DEC);

    // Blink the colon by flipping its value every loop iteration
    // (which happens every second).
    blinkColon = !blinkColon;
    clockDisplay.drawColon(blinkColon);
    sx1509.digitalWrite(LED1, blinkColon);

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
  Serial.println(host.c_str());
}

void
ThrottleController::wifiOnDisconnect() {
  client.stop();
  wiThrottle.disconnect();
  statusLED.setWifiDisconnected();
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
