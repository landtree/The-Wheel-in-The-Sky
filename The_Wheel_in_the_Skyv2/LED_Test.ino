void ledTest() {
  //Red
  for (int i = 0; i < NUM_LEDS; i++)
  {

    leds[i] = CRGB(255, 0, 0);
    delay(100);
  }
  FastLED.show();

  //Green
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 255, 0);
    delay(100);
  }
  FastLED.show();

  //Blue
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 255);
    delay(100);
  }
  FastLED.show();

  //off
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 0);
    delay(100);
  }
  FastLED.show();
  if (debug)
  {
    Serial.println("LEDS Color Test Done");
  }

}
