/*
   Fast Clock

   This sketch implements a Digital Fast Time clock.

   The hardware is use is an Adafruit Huzzah32 (ESP32 based),
   with a 7-segment display feather wing.

   Huzzah32 - https://www.adafruit.com/product/3405
   Display  - https://www.adafruit.com/product/3108

   This combination makes a battery powered portable fast clock for your
   layout.  Or power it with 5V and you have a permanent installation.

   Copyright 2018 by david d zuhn <zoo@blueknobby.com>

   This work is licensed under the Creative Commons Attribution-ShareAlike
   4.0 International License. To view a copy of this license, visit
   http://creativecommons.org/licenses/by-sa/4.0/deed.en_US.

   You may use this work for any purposes, provided that you make your
   version available to anyone else.
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


// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70

// I2C address of the SX1509 GPIO expander.
#define SX1509_ADDRESS  0x3E

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

Chrono speedCheck;

WiFiClient client;
WiThrottle wiThrottle;
SX1509 sx1509;

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
  sx1509.analogWrite(RGB_BLUE, 0);
  sx1509.analogWrite(RGB_GREEN, 0x40);

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


void setup() {
  analogReadResolution(12);   // 0-4095, no matter what the hardware support

  clockDisplay.begin(DISPLAY_ADDRESS);
  clockDisplay.clear();

  Serial.begin(115200);




  // Call io.begin(<address>) to initialize the SX1509. If it
  // successfully communicates, it'll return 1.
  if (!sx1509.begin(SX1509_ADDRESS))
  {
    while (1) ; // If we fail to communicate, loop forever.
  }

  Serial.println("SX1509 initialized");

  // The SX1509 has built-in debounce features, so a single
  // button-press doesn't accidentally create multiple ints.
  // Use io.debounceTime(<time_ms>) to set the GLOBAL SX1509
  // debounce time.
  // <time_ms> can be either 0, 1, 2, 4, 8, 16, 32, or 64 ms.
  sx1509.debounceTime(16); // Set debounce time to 32 ms.


  sx1509.pinMode(RGB_RED, ANALOG_OUTPUT);
  sx1509.pinMode(RGB_GREEN, ANALOG_OUTPUT);
  sx1509.pinMode(RGB_BLUE, ANALOG_OUTPUT);
  sx1509.analogWrite(RGB_RED, 0);
  sx1509.analogWrite(RGB_GREEN, 0);
  sx1509.analogWrite(RGB_BLUE, 0);

  sx1509.pinMode(BRAKE,   INPUT_PULLUP);
  sx1509.enableInterrupt(BRAKE, CHANGE);
  sx1509.debouncePin(BRAKE);

  sx1509.pinMode(BUTTON1, INPUT_PULLUP);
  sx1509.enableInterrupt(BUTTON1, CHANGE);
  sx1509.debouncePin(BUTTON1);

  sx1509.pinMode(BUTTON2, INPUT_PULLUP);
  sx1509.enableInterrupt(BUTTON2, CHANGE);
  sx1509.debouncePin(BUTTON2);

  sx1509.pinMode(BUTTON3, INPUT_PULLUP);
  sx1509.enableInterrupt(BUTTON3, CHANGE);
  sx1509.debouncePin(BUTTON3);

  sx1509.pinMode(BUTTON4, INPUT_PULLUP);
  sx1509.enableInterrupt(BUTTON4, CHANGE);
  sx1509.debouncePin(BUTTON4);

  sx1509.pinMode(BUTTON5, INPUT_PULLUP);
  sx1509.enableInterrupt(BUTTON5, CHANGE);
  sx1509.debouncePin(BUTTON5);

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


int readButton(int intrStatus, int buttonPin, const char *name)
{
  int state = 0;

  if (intrStatus & (1 << buttonPin)) {
    int state = sx1509.digitalRead(buttonPin);
    Serial.print(name); Serial.print(" BUTTON: ");
    Serial.println(state ? "RELEASED" : "PRESSED");
  }
  return state;

}


void readButtons()
{
  unsigned int intStatus = sx1509.interruptSource();
  // For debugging handiness, print the intStatus variable.
  // Each bit in intStatus represents a single SX1509 IO.
  readButton(intStatus, BRAKE, "BRAKE");
  readButton(intStatus, BUTTON1, "BUTTON1");
  readButton(intStatus, BUTTON2, "BUTTON2");
  readButton(intStatus, BUTTON3, "BUTTON3");
  readButton(intStatus, BUTTON4, "BUTTON4");
  readButton(intStatus, BUTTON5, "BUTTON5");



}


int previousSpeedValue = -1;
int actualSpeedValue = -1;
int previousDirection = -1;

void readSpeed()
{
  bool speedChanged = false;
  bool directionChanged = false;

  int rawSpeedValue = analogRead(SPEED_KNOB);
  int speedValue = map(rawSpeedValue, 0, 4095, 0, 126);

  int d1 = !digitalRead(DIR_LEFT);
  int d2 = !digitalRead(DIR_RIGHT);
  int direction;

  if (d1 && d2) {
    direction = 3;  // SHOULD NEVER HAPPEN
  }
  else if (d1) {
    direction = 1;
  }
  else if (d2) {
    direction = 2;
  }
  else {
    // center off position
    speedValue = 0;
    direction = 0;
  }

  if (direction != previousDirection) {
    previousDirection = direction;
    directionChanged = true;
  }

  if (speedValue != actualSpeedValue && speedValue != previousSpeedValue) {
    previousSpeedValue = actualSpeedValue;
    actualSpeedValue = speedValue;
    speedChanged = true;
  }

  if (speedChanged || directionChanged) {
    Serial.print("SPEED = (raw)"); Serial.print(rawSpeedValue);
    Serial.print(" (mapped)"); Serial.print(speedValue);
    switch (direction) {
      case 0:
        Serial.println(" CENTER OFF");
        break;
      case 1:
        Serial.println(" LEFT");
        break;
      case 2:
        Serial.println(" RIGHT");
        break;
      default:
        Serial.println(" SHOUDLN'T HAPPEN");
        break;
    }

    if (directionChanged) {
      wiThrottle.setDirection(direction);
    }
    if (speedChanged) {
      wiThrottle.setSpeed(speedValue);
    }

  }

}






void loop()
{
  bool addressSelected = false;
  String selectedAddress = "S21";

  clockDisplay.clear();


  sx1509.analogWrite(RGB_RED, 0x0);
  sx1509.analogWrite(RGB_BLUE, 0x0);
  sx1509.analogWrite(RGB_GREEN, 0x40);


  WiFi.disconnect();
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_STA);

  WiFi.begin(ssid.c_str(), password.c_str());




  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  while (! client.connected()) {
    if (!client.connect(host.c_str(), port)) {
      Serial.println("connection failed");
    }
    else {
      Serial.println("connected succeeded");
      wiThrottle.connect(&client);
    }
    delay(1000);
    Serial.print(":");
  }

  sx1509.analogWrite(RGB_GREEN, 0);
  sx1509.analogWrite(RGB_BLUE, 0x20);


  while (true) {

    if (wiThrottle.check()) {
      if (wiThrottle.clockChanged) {
        updateFastTimeDisplay();
      }
      if (wiThrottle.protocolVersionChanged) {
        wiThrottle.setDeviceName("mylittlethrottle");
      }
      if (wiThrottle.heartbeatChanged) {
        wiThrottle.requireHeartbeat(true);
      }
      if (! client.connected()) {
        wiThrottle.disconnect();
        Serial.println("Network Disconnected");
        break;
      }
      else {
        sx1509.analogWrite(RGB_BLUE, 0x80);
      }
      if (handleSX1509Interrupt) {
        readButtons();
        handleSX1509Interrupt = false;
      }

#if 1
      if (!addressSelected) {
        addressSelected = wiThrottle.addLocomotive(selectedAddress);
      }
#endif

      if (speedCheck.hasPassed(1000 / 15)) {
        speedCheck.restart();
        readSpeed();
      }
    }
  }
}
