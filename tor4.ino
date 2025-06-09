#include <FastLED.h>
#include <avr/sleep.h>

#define NUM_LEDS 46 //67 //88
#define NUM_LEDS_TMP 23 //первая колонка

#define DATA_PIN 2
int battery_voltage;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() { 
	Serial.begin(57600);
	Serial.println("resetting");
	FastLED.addLeds<WS2812,DATA_PIN,GRB>(leds,NUM_LEDS);
	FastLED.setBrightness(255);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  #if defined(__LGT8FX8P__)
    analogReference(INTERNAL1V024);
  #else
    analogReference(INTERNAL1V25);
  #endif
    analogReadResolution(12);
}

void blink() { 
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CRGB::Red; 
    // Show the leds
    FastLED.show(); }
    delay(500);
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CRGB::Black;
    // Show the leds
    FastLED.show(); }
  delay(500);
  
  battery_voltage = readVcc();
  Serial.println(battery_voltage);
    if (battery_voltage< 2900){sleep_mode();}
  blink();
}

long readVcc() {
  uint32_t v = analogRead(VCCM);
    
    // convert to VCC (unit: mV);
  #if defined(__LGT8FX8P__)
    v = (v * 5 ) / 4 ;
  #else
    v = (v * 5000 ) / 4096 ;
  #endif
  return v;
}

void Fire2012()
{
// Array of temperature readings at each simulation cell

  static uint8_t heat[NUM_LEDS_TMP];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS_TMP; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS_TMP) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS_TMP - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS_TMP; j++) {
      CRGB color = HeatColor( heat[j]);
      leds[j] = color;
    }

    // Step 5. Map from heat cells to LED colors for the inverted second row
    for (int j = 0; j < NUM_LEDS_TMP; j++) {
      CRGB color = HeatColor(heat[j]);
      int pixelnumber = NUM_LEDS-1-j; // Direct mapping for the second row
      leds[pixelnumber] = color; // Assuming leds2 is defined and represents the second row
    }
}

void loop() { 

  Fire2012(); // run simulation frame
  FastLED.show(); // display this frame
  FastLED.delay(20);
  
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		// Wait a little bit before we loop around and do it again
	//Serial.print("x");

  
    battery_voltage = readVcc();
    Serial.println(battery_voltage);
    if (battery_voltage< 3000){
      battery_voltage = readVcc(); 
      if (battery_voltage< 3000){ //нужна перепроверка потому что иногда (редко) может поймать рандомное значение
        blink();}}
      
    
	}
