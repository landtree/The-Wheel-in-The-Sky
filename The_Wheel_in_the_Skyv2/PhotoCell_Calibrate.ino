//Sets the Photocell to match the light brightness when turned on

void Calibration()
{
  //temp variables for calibration
  byte lightcount = 0;
  boolean on = 1;
  if (debug)
  {
    Serial.println("Starting Calibration");
  }
  //run for 10 interations
  while (on == 1)
  {
    // calibrate during the first 10 seconds
    unsigned long currentMillis = millis();

    PhotoValue = analogRead(sensorPin);

    if (debug)
    {
      Serial.print("Raw Sensor: ");
      Serial.println(PhotoValue);
    }

    // record the maximum sensor value
    if (PhotoValue > sensorMax) {
      sensorMax = PhotoValue;
    }

    // record the minimum sensor value
    if (PhotoValue < sensorMin) {
      sensorMin = PhotoValue;
    }

    if (currentMillis - previousCalibrateMillis >= 1000)
    {
      // save the last time you blinked the LED
      previousCalibrateMillis = currentMillis;
      lightcount++;

      if (debug)
      {
        Serial.println(lightcount);
      }

      if (lightcount == 1)
      {
        leds[9] = CRGB(0, 100, 100);
        FastLED.show();
      }
      else if (lightcount == 5)
      {
        leds[9] = CRGB(255, 247, 0);
        FastLED.show();
      }
      else if (lightcount == 9)
      {
        leds[9] = CRGB(0, 0, 255);
        FastLED.show();
      }
      else if (lightcount == 10)
      {
        leds[9] = CRGB(0, 0, 0);
        FastLED.show();
        on = 0;
      }
    }
  }
}
