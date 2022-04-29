void Warmup()
{
  if (breathing)
  {

    float val = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0, 0, val);
    }


    FastLED.show();
  }
}

void resetPal()
{
  whichPalette = -1;
}

void twinkle()
{

  if (lightsOn)
  {
    if (humanTime == 5)
    {
      if (oneloop)
      {
        if (debug)
        {
          Serial.println("Human in front for 5 seconds");
        }
        chooseNextColorPalette( gTargetPalette );
        oneloop = false;
        drawTwinkles( leds);
        FastLED.show();
      }
    }
    if (humanTime == 30)
    {
      if (twoloop)
      {
        TWINKLE_SPEED = 4;
        twoloop = false;
        chooseNextColorPalette( gTargetPalette );
        drawTwinkles( leds);
        FastLED.show();
        Serial.println("30 Secs");
      }
    }
    if (humanTime == 60)
    {
      if (threeloop)
      {
        TWINKLE_SPEED = 5;
        threeloop = false;
        chooseNextColorPalette( gTargetPalette );
        drawTwinkles( leds);
        FastLED.show();
      }
    }

    if (humanTime == 100)
    {
      if (fourloop)
      {
        TWINKLE_SPEED = 6;
        fourloop = false;
        chooseNextColorPalette( gTargetPalette );
        drawTwinkles( leds);
        FastLED.show();
      }
    }

    if (humanTime == 130)
    {
      if (fiveloop)
      {
        TWINKLE_SPEED = 7;
        fiveloop = false;
        chooseNextColorPalette( gTargetPalette );
        drawTwinkles( leds);
        FastLED.show();
      }
    }

    if (humanTime == 160)
    {
      if (sixloop)
      {
        sixloop = false;
        TWINKLE_SPEED = 8;
        chooseNextColorPalette( gTargetPalette );
        drawTwinkles( leds);
        FastLED.show();
      }
    }
    drawTwinkles( leds);
    FastLED.show();
  }
}
