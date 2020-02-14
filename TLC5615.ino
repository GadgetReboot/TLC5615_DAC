/*
  A demo of the TLC5615 10-bit SPI DAC with Arduino Uno
  Generates a test sine wave, rising ramp wave, and
  some custom voltage outputs for hardware testing.

  Gadget Reboot

*/

#include <Wire.h>
#include <TLC5615.h>

// SPI chip select pin
#define dac_CS    10

// button inputs to toggle modes
#define button0   2  // voltage step-change mode
#define button1   3  // fixed reference voltage mode
#define button2   4  // sine wave
#define button3   5  // ramp wave

// current position in waveform/sample playback
int sinePos     = 0;
int rampPos     = 0;
int vStepPos    = 0;

// create DAC object using chosen chip select pin
TLC5615 dac(dac_CS);

// states for the DAC output generator to be operating in
enum runModes {SINEWAVE_FAST, SINEWAVE_SLOW, RAMPWAVE_FAST, RAMPWAVE_SLOW,
               VOLTAGE_STEPS, FIXED_V_LOW, FIXED_V_HIGH
              };
// default to a fast sine wave on reset
runModes runMode = SINEWAVE_FAST;

// various voltages to sequence through for one of the generator test patterns
// 0..1023 = ~0v..4v
int voltageSteps[12] = {350, 525, 400, 750, 900, 1000, 200, 800, 400, 600, 500, 1023};

// sine wave data table generated with https://www.daycounter.com/Calculators/Sine-Generator-Calculator.phtml
const PROGMEM uint16_t sineTable[256] =
{
  512, 524, 537, 549, 562, 574, 587, 599,
  611, 624, 636, 648, 660, 672, 684, 696,
  707, 719, 730, 741, 753, 764, 774, 785,
  796, 806, 816, 826, 836, 846, 855, 864,
  873, 882, 890, 899, 907, 915, 922, 930,
  937, 944, 950, 957, 963, 968, 974, 979,
  984, 989, 993, 997, 1001, 1004, 1008, 1011,
  1013, 1015, 1017, 1019, 1021, 1022, 1022, 1023,
  1023, 1023, 1022, 1022, 1021, 1019, 1017, 1015,
  1013, 1011, 1008, 1004, 1001, 997, 993, 989,
  984, 979, 974, 968, 963, 957, 950, 944,
  937, 930, 922, 915, 907, 899, 890, 882,
  873, 864, 855, 846, 836, 826, 816, 806,
  796, 785, 774, 764, 753, 741, 730, 719,
  707, 696, 684, 672, 660, 648, 636, 624,
  611, 599, 587, 574, 562, 549, 537, 524,
  512, 499, 486, 474, 461, 449, 436, 424,
  412, 399, 387, 375, 363, 351, 339, 327,
  316, 304, 293, 282, 270, 259, 249, 238,
  227, 217, 207, 197, 187, 177, 168, 159,
  150, 141, 133, 124, 116, 108, 101, 93,
  86, 79, 73, 66, 60, 55, 49, 44,
  39, 34, 30, 26, 22, 19, 15, 12,
  10, 8, 6, 4, 2, 1, 1, 0,
  0, 0, 1, 1, 2, 4, 6, 8,
  10, 12, 15, 19, 22, 26, 30, 34,
  39, 44, 49, 55, 60, 66, 73, 79,
  86, 93, 101, 108, 116, 124, 133, 141,
  150, 159, 168, 177, 187, 197, 207, 217,
  227, 238, 249, 259, 270, 282, 293, 304,
  316, 327, 339, 351, 363, 375, 387, 399,
  412, 424, 436, 449, 461, 474, 486, 499
};

void setup() {

  // configure button inputs
  pinMode(button0, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);

  // initialize DAC
  dac.begin();
}

void loop() {

  // update the run mode if a button is detected
  checkButtons();

  switch (runMode) {
    case SINEWAVE_FAST: {  // cycle through sinewave table as fast as possible
        // sine wave
        dac.analogWrite(pgm_read_word(&(sineTable[sinePos])));
        sinePos++;
        if (sinePos >= 256)
          sinePos = 0;
        break;
      }
    case SINEWAVE_SLOW: {  // cycle through sinewave table with a slow down delay
        // sine wave
        dac.analogWrite(pgm_read_word(&(sineTable[sinePos])));
        sinePos++;
        if (sinePos >= 256)
          sinePos = 0;
        delay(1);
        break;
      }
    case RAMPWAVE_FAST: {  // generate rising ramp wave as fast as possible
        // rising ramp wave
        dac.analogWrite(rampPos);
        rampPos++;
        if (rampPos >= 1024)
          rampPos = 0;
        break;
      }
    case RAMPWAVE_SLOW: {  // generate rising ramp wave with a slow down delay
        // rising ramp wave
        dac.analogWrite(rampPos);
        rampPos++;
        if (rampPos >= 1024)
          rampPos = 0;
        delay(1);
        break;
      }
    case VOLTAGE_STEPS: {  // cycle between an array of fixed output voltages
        // fixed voltage steps
        dac.analogWrite(voltageSteps[vStepPos]);
        vStepPos ++;
        if (vStepPos >= 12)
          vStepPos = 0;
        delay(250);
        break;
      }
    case FIXED_V_LOW: {  // set a fixed voltage out
        // fixed output voltage
        dac.analogWrite(600);
        break;
      }
    case FIXED_V_HIGH: {  // set a fixed voltage out
        // fixed output voltage
        dac.analogWrite(950);
        break;
      }
  }
}


void checkButtons() {

  // generate preset pattern of fixed voltages to change 555 frequency
  if (digitalRead(button0) == 0) {
    runMode = VOLTAGE_STEPS;
    vStepPos = 0;
    while (!digitalRead(button0)) {}
  }

  // set one of two steady voltages to use as a Vref generator
  if (digitalRead(button1) == 0) {
    if (runMode == FIXED_V_LOW) {
      runMode = FIXED_V_HIGH;
    }
    else if (runMode == FIXED_V_HIGH) {
      runMode = FIXED_V_LOW;
    }
    else {
      runMode = FIXED_V_LOW;
    }
    while (!digitalRead(button1)) {}
  }

  // toggle between a slower and faster sine wave
  if (digitalRead(button2) == 0) {
    if (runMode == SINEWAVE_SLOW) {
      runMode = SINEWAVE_FAST;
    }
    else if (runMode == SINEWAVE_FAST) {
      runMode = SINEWAVE_SLOW;
    }
    else {
      runMode = SINEWAVE_FAST;
    }
    while (!digitalRead(button2)) {}
  }

  // toggle between a slower and faster ramp wave
  if (digitalRead(button3) == 0) {
    if (runMode == RAMPWAVE_SLOW) {
      runMode = RAMPWAVE_FAST;
    }
    else if (runMode == RAMPWAVE_FAST) {
      runMode = RAMPWAVE_SLOW;
    }
    else {
      runMode = RAMPWAVE_FAST;
    }
    while (!digitalRead(button3)) {}
  }
}
