/*
   Throttle

   This sketch implements a Model Railroad Throttle using the WiThrottle protocl.

   The hardware is use is an Adafruit Huzzah32 (ESP32 based), with additional
   supporting hardware, including
     * SX1509 GPIO expander (up to 16 I/O lines)
     * RGB LED
     * 10K potentiometer (for Analog reads)
     * SPDT center off toggle switch
     * LIS3DH accelerometer
     * DRV2605 haptic feedback controller
     * 4 digit display

   Huzzah32 - https://www.adafruit.com/product/3405
   SX1509   - https://www.sparkfun.com/products/13601
   LIS3DH   - https://www.adafruit.com/product/2809
   DRV2605  - https://www.adafruit.com/product/2305
   display  - https://www.adafruit.com/product/3106

   Copyright 2018 by david d zuhn <zoo@blueknobby.com>

   This work is licensed under the Creative Commons Attribution-ShareAlike
   4.0 International License. To view a copy of this license, visit
   http://creativecommons.org/licenses/by-sa/4.0/deed.en_US.

   You may use this work for any purposes, provided that you make your
   version available to anyone else.

   If you make a commercial product out of this software, please consider
   sending a sample to the author of this code.
*/

// You need to change these values to suit your network...

#define WIFI_SSID     "myownlittleidaho"
#define WIFI_PASSWORD "goslowdown"

#define JMRI_SERVER_ADDRESS "192.168.22.20"



#include <string>

#include <Chrono.h>

#include <Time.h>
#include <TimeLib.h>

#include <WiFi.h>
#include <WiThrottle.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <SparkFunSX1509.h> // Include SX1509 library

#include "RGBLED.h"
#include "BKTThrottleDelegate.h"

#include "hardware.h"

typedef enum TogglePosition {
  Left = 0,
  Right = 1,
  CenterOff = 2,
  Unknown = 3
} TogglePosition;



BKTThrottleDelegate throttleDelegate;


// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   (0x70)

// I2C address of the SX1509 GPIO expander.
#define SX1509_ADDRESS  (0x3E)

// the INTR output pin of the SX1509 is attached to this pin on the ESP32
#define SX1509_INTR_PIN (A10)


#define RGB_RED (15)
#define RGB_GREEN (14)
#define RGB_BLUE (13)
#define BUTTON1 (9)
#define BUTTON2 (10)
#define BUTTON3 (11)
#define BUTTON4 (12)
#define BUTTON5 (7)
#define BRAKE (8)

#define LED1 (0)
#define LED2 (1)
#define LED3 (2)
#define LED4 (3)
#define LED5 (4)
#define LED6 (5)
#define LED7 (6)

#define SPEED_KNOB (A4)
#define DIR_LEFT (15)
#define DIR_RIGHT (33)

const std::string ssid = WIFI_SSID;
const std::string password = WIFI_PASSWORD;

const std::string host = JMRI_SERVER_ADDRESS;
const int port = 12090;


static volatile bool wifi_connected = false;

Chrono speedPotRead;
Chrono speedCheck;

WiFiClient client;
WiThrottle wiThrottle;
SX1509 sx1509;
SX1509RGBLED statusLED(sx1509, RGB_RED, RGB_GREEN, RGB_BLUE);

Adafruit_7segment clockDisplay = Adafruit_7segment();

void wifiOnConnect() {
  Serial.println("STA Connected");
  Serial.print("STA IPv4: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to ");
  Serial.println(host.c_str());
}

void wifiOnDisconnect() {
  client.stop();
  wiThrottle.disconnect();
  statusLED.setWifiDisconnected();


  Serial.println("STA Disconnected");
}

void WiFiEvent(WiFiEvent_t event) {
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
      wifi_connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      wifi_connected = false;
      wifiOnDisconnect();
      break;
    default:
      break;
  }
}


bool handleSX1509Interrupt = false;



void setupButtonPin(SX1509& sx1509, int pin)
{
    sx1509.pinMode(pin, INPUT_PULLUP);
    sx1509.enableInterrupt(pin, CHANGE);
    sx1509.debouncePin(pin);
}


void setup() {
  Serial.begin(115200);

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

  setupButtonPin(sx1509, BRAKE);
  setupButtonPin(sx1509, BUTTON1);
  setupButtonPin(sx1509, BUTTON2);
  setupButtonPin(sx1509, BUTTON3);
  setupButtonPin(sx1509, BUTTON4);
  setupButtonPin(sx1509, BUTTON5);

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
  attachInterrupt(digitalPinToInterrupt(SX1509_INTR_PIN),
                  sx1509_interrupt, FALLING);

  pinMode(DIR_LEFT, INPUT_PULLUP);
  pinMode(DIR_RIGHT, INPUT_PULLUP);

  analogReadResolution(12);   // 0-4095, no matter what the hardware support

  wiThrottle.delegate = &throttleDelegate;
}


void sx1509_interrupt()
{
  handleSX1509Interrupt = true;
}


void updateFastTimeDisplay()
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


void
readButton(int intrStatus, int buttonPin, int funcNum, const char *name)
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
void readButtons()
{
  unsigned int intrStatus = sx1509.interruptSource();
  // For debugging handiness, print the intStatus variable.

  // Each bit in intStatus represents a single SX1509 I/O.
  readButton(intrStatus, BRAKE,   9, "BRAKE");

  readButton(intrStatus, BUTTON1, 0, "BUTTON1");
  readButton(intrStatus, BUTTON2, 1, "BUTTON2");
  readButton(intrStatus, BUTTON3, 2, "BUTTON3");
  readButton(intrStatus, BUTTON4, 3, "BUTTON4");
  readButton(intrStatus, BUTTON5, 4, "BUTTON5");
}


int previousSpeedValue = -1;
int penultimateSpeedValue = -1;

TogglePosition previousTogglePosition = Unknown;



Direction directionFromTogglePosition(TogglePosition position)
{
  Direction value = Forward;

  if (position == Left) {
    value = Reverse;
  }

  Serial.print("TogglePosition "); Serial.print(position); Serial.print(" => Direction "); Serial.println(value);
  return value;
}


TogglePosition readTogglePosition()
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



int speedAccumulator;

#define SPEED_WEIGHT (0.80)

void readSpeedPot()
{
    int rawSpeedValue = analogRead(SPEED_KNOB);
    int reading = map(rawSpeedValue, 0, 4095, 0, 126);

    speedAccumulator = SPEED_WEIGHT * speedAccumulator + reading;
}


void readSpeed()
{
  bool speedChanged = false;
  bool togglePositionChanged = false;

  int speedValue = (((speedAccumulator/5) / (1.0 - SPEED_WEIGHT))/5);

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


#if 1
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

}





// This loop waits for the WiFi connection, and the WiThrottle connection
// once that's lost, it exits and restarts

// unlike the usual Arduino loop() which is called many many times a second
// this one contains its own repeat loop for action checks
void loop()
{
  bool addressSelected = false;
  bool nameSent = false;
  String selectedAddress = "S21";

  clockDisplay.clear();

  // BLE is not connected at this time, nor is WiFI
  statusLED.setWifiDisconnected();

  WiFi.disconnect();
  WiFi.onEvent(WiFiEvent);
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
      Serial.println("connection failed");
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
