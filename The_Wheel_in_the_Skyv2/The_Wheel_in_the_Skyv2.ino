/* Pinout for Circuit*/
//PINOUT
//DIN = D9
//Photocell = A3
//INT = D4
//SCL = D3
//SDA = D2



//libraries
#include <SparkFun_AK975X_Arduino_Library.h>
#include <Wire.h>
#include <FastLED.h>
#include <arduino-timer.h>

/******* Varaible for Sensors ********/
//Constants
const byte sensorPin = A3;    // pin that the sensor is attached to

//LED Settings
const byte LEDData = 9;
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    10
CRGBArray<NUM_LEDS> leds;

#define BRIGHTNESS          125
#define FRAMES_PER_SECOND  60

/****************This is for the Fire Pallete routine***********************/
//bool gReverseDirection = false;
//CRGBPalette16 gPal;

/****************This is for the breathing attractor******************/
byte val = 0;
bool dir;

/****************This is for the twinkle mode******************/

// Overall twinkle speed.
// 0 (VERY slow) to 8 (VERY fast).
// 4, 5, and 6 are recommended, default is 4.
byte TWINKLE_SPEED = 3;

// Overall twinkle density.
// 0 (NONE lit) to 8 (ALL lit at once).
// Default is 5.
#define TWINKLE_DENSITY 5

// How often to change color palettes.
#define SECONDS_PER_PALETTE  30

// Background color for 'unlit' pixels
// Can be set to CRGB::Black if desired.
CRGB gBackgroundColor = CRGB::Black;
#define AUTO_SELECT_BACKGROUND_COLOR 1
#define COOL_LIKE_INCANDESCENT 1

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;
static uint8_t whichPalette = -1;

//IR Sensor Settings
AK975X movementSensor; //Hook object to the library
int ir1, ir2, ir3, ir4;
int ir1dif, ir2dif, ir3dif, ir4dif;
int ir1Last, ir2Last, ir3Last, ir4Last;

// Changing Variables:
int PhotoValue = 0;         // the sensor value
int sensorMin = 0;         // Absolute minimum sensor value
int sensorMax = 1024;     // Absolute maximum sensor value
byte humanCount = 0;     //counts how long person is front of sensor
int humanTime = -1;

//this is what sets the trigger for the light routine to turn on
int IRMin = -9500; // Max human threshold (farther away from sensor
int IRMax = 2000; // Min human threshold (closer to sensor)

//Photocell trip limit
byte dark = 25;

/******* Timers **********/
unsigned long previousCalibrateMillis = 0;
const long CalibrateInterval = 10000;
#define PCtimer 10000
#define IRtimer 5000
Timer <1, millis, const char *> pctimer ; // create a timer with default settings
Timer <1, millis, const char *> irtimer;

//Global Flags
boolean lightsOn, allowIR, Human, runOnce, breathing;

//Timer Flags
boolean oneloop = true, twoloop = true, threeloop = true, fourloop = true, fiveloop = true, sixloop = true;

//debug Flag
boolean debug = true;



void setup() {
  //Sanity Delay
  delay(3000); // 3 second delay for recovery

  //Setup LEDs for use
  FastLED.addLeds<LED_TYPE, LEDData, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalPixelString);
  FastLED.setBrightness(50);

  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  //gPal = CRGBPalette16( CRGB::Black, CRGB::DarkBlue, CRGB::Green, CRGB::Pink);

  //Turn on Serial for output
  if (debug)
  {
    Serial.begin(115200);
  }

  //Check if IR is Connected
  if (!debug)
  {
    Serial.println("checking if device is connected");
  }
  Wire.begin();
  if (movementSensor.begin(Wire, I2C_SPEED_FAST) == false)
  {
    while (1)
    {
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB(255, 0, 0);
        delay(100);
      }
      FastLED.show();
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB(0, 0, 0);
        delay(100);
      }
      FastLED.show();
    }
  }

  //small delay to let Serial connection catchup
  delay(100);
  //Check to see if RBG works on all LEDS
  ledTest();
  //Calibrates the photocell to match room brightness
  Calibration();


  //set flags for start
  lightsOn = false;
  allowIR = false;
  Human = false;

  //Set to full brightness & darken
  FastLED.setBrightness(BRIGHTNESS);

  // signal the end of the calibration period
  if (debug)
  {
    Serial.println("Calibration over....Results: ");
    Serial.print("IR Max= ");
    Serial.print(IRMax);
    Serial.print(" | IR Min= ");
    Serial.println(IRMin);
    Serial.print("PC Max= ");
    Serial.print(sensorMax);
    Serial.print(" | PC Min= ");
    Serial.println(sensorMin);
    Serial.print("5 Sec loop: ");
    Serial.print(oneloop);
    Serial.print(" | 30 Sec loop: ");
    Serial.print(twoloop);
    Serial.print("| 60 Sec loop: ");
    Serial.print(threeloop);
    Serial.print("| 100 Sec loop: ");
    Serial.print(fourloop);
    Serial.print("| 130 Sec loop: ");
    Serial.print(fiveloop);
    Serial.print("| 160 Sec loop: ");
    Serial.println(sixloop);
  }

  //start timers for repeated checks
  pctimer.every(PCtimer, readPhoto, (void *)1);
  irtimer.every(IRtimer, allowCheck, (void *)1);
  //blendTimer.every(10, Blend);

  if (debug)
  {
    Serial.println("starting...");
  }

  chooseNextColorPalette(gTargetPalette);

}

void loop()
{
  pctimer.tick(); // tick the timer
  irtimer.tick(); // tick the timer
  //blendTimer.tick();

  nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 50);
  FastLED.show();


  if (allowIR)
  {
    pctimer.cancel();
    if (!runOnce)
    {
      runOnce = 1;
    }
  }


  if (!lightsOn)
  {
    //fades all Leds to black
    if (!breathing)
    {
      fadeToBlackBy(leds, NUM_LEDS, 255);
      FastLED.show();
      breathing = true;
    }
    else
    {
      TWINKLE_SPEED = 3;
      resetPal();
      Warmup();


    }
  }
  else
  {
    drawTwinkles( leds);
    FastLED.show();
  }
}


bool allowCheck(void *)
{
  if (debug)
  {
    Serial.println("Timer: allow IR check");
  }
  Human = false;
  ReadSensor();
  return true;
}



bool inRange(int val1, int val2, int val3, int val4, int minn, int maxx)
{
  if (debug)
  {
    Serial.print("ir1:");
    Serial.print((minn <= val1) && (val1 <= maxx));
    Serial.print("| ir2:");
    Serial.print((minn <= val2) && (val2 <= maxx));
    Serial.print("| ir3:");
    Serial.print((minn <= val3) && (val3 <= maxx));
    Serial.print("| ir4:");
    Serial.println((minn <= val4) && (val4 <= maxx));
  }


  return ((minn <= val1) && (val1 <= maxx) || (minn <= val2) && (val2 <= maxx) || (minn <= val3) && (val3 <= maxx) || (minn <= val4) && (val4 <= maxx) );
}


void drawTwinkles( CRGBSet& L)
{

  if (debug)
  {
    //Serial.println("Drawing Twinkles....");

  }



  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;

  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if ( (AUTO_SELECT_BACKGROUND_COLOR == 1) &&
       (gCurrentPalette[0] == gCurrentPalette[1] )) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if ( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if ( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();

  for ( CRGB& pixel : L) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16 = PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF) >> 4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if ( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color,
      // use the new color.
      pixel = c;
    } else if ( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      pixel = blend( bg, c, deltabright * 8);
    } else {
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      pixel = bg;
    }
  }
}


//  This function takes a time in pseudo-milliseconds,
//  figures out brightness = f( time ), and also hue = f( time )
//  The 'low digits' of the millisecond time are used as
//  input to the brightness wave function.
//  The 'high digits' are used to select a color, so that the color
//  does not change over the course of the fade-in, fade-out
//  of one cycle of the brightness wave function.
//  The 'high digits' are also used to determine whether this pixel
//  should light at all during this cycle, based on the TWINKLE_DENSITY.
CRGB computeOneTwinkle( uint32_t ms, uint8_t salt)
{
  uint16_t ticks = ms >> (8 - TWINKLE_SPEED);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  uint8_t bright = 0;
  if ( ((slowcycle8 & 0x0E) / 2) < TWINKLE_DENSITY) {
    bright = attackDecayWave8( fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if ( bright > 0) {
    c = ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
    if ( COOL_LIKE_INCANDESCENT == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}


// This function is like 'triwave8', which produces a
// symmetrical up-and-down triangle sawtooth waveform, except that this
// function produces a triangle wave with a faster attack and a slower decay:
//
//     / \ 
//    /     \ 
//   /         \ 
//  /             \ 
//

uint8_t attackDecayWave8( uint8_t i)
{
  if ( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i / 2));
  }
}

// This function takes a pixel, and if its in the 'fading down'
// part of the cycle, it adjusts the color a little bit like the
// way that incandescent bulbs fade toward 'red' as they dim.
void coolLikeIncandescent( CRGB& c, uint8_t phase)
{
  if ( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}


















const TProgmemRGBPalette16 Blue_p FL_PROGMEM =
{ CRGB::DodgerBlue, CRGB::DarkBlue, CRGB::DodgerBlue, CRGB::DarkBlue,
  CRGB::DodgerBlue, CRGB::DarkBlue, CRGB::DodgerBlue, CRGB::DarkBlue,
  CRGB::DodgerBlue, CRGB::DarkBlue, CRGB::DodgerBlue, CRGB::DarkBlue,
  CRGB::DodgerBlue, CRGB::DarkBlue, CRGB::DodgerBlue, CRGB::DarkBlue
};

const TProgmemRGBPalette16 Red_p FL_PROGMEM =
{ CRGB::MediumVioletRed, CRGB::DodgerBlue, CRGB::MediumVioletRed, CRGB::DodgerBlue,
  CRGB::MediumVioletRed, CRGB::DodgerBlue, CRGB::MediumVioletRed, CRGB::DodgerBlue,
  CRGB::MediumVioletRed, CRGB::DodgerBlue, CRGB::MediumVioletRed, CRGB::DodgerBlue,
  CRGB::MediumVioletRed, CRGB::DodgerBlue, CRGB::MediumVioletRed, CRGB::DodgerBlue
};


const TProgmemRGBPalette16 Green_p FL_PROGMEM =
{ CRGB::MediumVioletRed, CRGB::PaleGreen, CRGB::MediumVioletRed, CRGB::PaleGreen,
  CRGB::MediumVioletRed, CRGB::PaleGreen, CRGB::MediumVioletRed, CRGB::PaleGreen,
  CRGB::MediumVioletRed, CRGB::PaleGreen, CRGB::MediumVioletRed, CRGB::PaleGreen,
  CRGB::MediumVioletRed, CRGB::PaleGreen, CRGB::MediumVioletRed, CRGB::PaleGreen
};

const TProgmemRGBPalette16 Yellow_p FL_PROGMEM =
{ CRGB::PaleGreen, CRGB::SpringGreen, CRGB::PaleGreen, CRGB::SpringGreen,
  CRGB::PaleGreen, CRGB::SpringGreen, CRGB::PaleGreen, CRGB::SpringGreen,
  CRGB::PaleGreen, CRGB::SpringGreen, CRGB::PaleGreen, CRGB::SpringGreen,
  CRGB::PaleGreen, CRGB::SpringGreen, CRGB::PaleGreen, CRGB::SpringGreen
};

const TProgmemRGBPalette16 Pink_p FL_PROGMEM =
{ CRGB::SpringGreen, CRGB::YellowGreen, CRGB::SpringGreen, CRGB::YellowGreen,
  CRGB::SpringGreen, CRGB::YellowGreen, CRGB::SpringGreen, CRGB::YellowGreen,
  CRGB::SpringGreen, CRGB::YellowGreen, CRGB::SpringGreen, CRGB::YellowGreen,
  CRGB::SpringGreen, CRGB::YellowGreen, CRGB::SpringGreen, CRGB::YellowGreen
};

const TProgmemRGBPalette16 White_p FL_PROGMEM =
{ CRGB::YellowGreen, CRGB::Aquamarine, CRGB::YellowGreen, CRGB::Aquamarine,
  CRGB::YellowGreen, CRGB::Aquamarine, CRGB::YellowGreen, CRGB::Aquamarine,
  CRGB::YellowGreen, CRGB::Aquamarine, CRGB::YellowGreen, CRGB::Aquamarine,
  CRGB::YellowGreen, CRGB::Aquamarine, CRGB::YellowGreen, CRGB::Aquamarine
};



const TProgmemRGBPalette16* ActivePaletteList[] =
{
  &Blue_p, //Start
  &Red_p, //30 secs
  &Green_p, //60 secs
  &Yellow_p, //100 sec
  &Pink_p, //130 secs
  &White_p //160 secs
};

// Add or remove palette names from this list to control which color
// palettes are used, and in what order.

// Advance to the next color palette in the list (above).
void chooseNextColorPalette( CRGBPalette16& pal)
{
  Serial.print("Start color pallete switch: ");
  Serial.println(whichPalette);

  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);
  whichPalette = addmod8( whichPalette, 1, numberOfPalettes);

  Serial.print("End color pallete switch: ");
  Serial.println(whichPalette);

  pal = *(ActivePaletteList[whichPalette]);

}
