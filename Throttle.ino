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

#include "ThrottleController.h"
#include "ThrottleData.h"

ThrottleData flashData;
ThrottleController controller(flashData);


void setup() {
    Serial.begin(115200);

    if (!flashData.begin()) {
        Serial.println("SPIFFS Mount Failed");
        while (true) ; //
    }

    BLEDevice::init(flashData.getDeviceName());

    BLEAddress addr = BLEDevice::getAddress();
    Serial.print("BLE Address is ");
    Serial.println(addr.toString().c_str());

    controller.begin();
}


void loop()
{
    controller.loop();
}
