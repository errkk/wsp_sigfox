#include <ArduinoLowPower.h>
#include <SigFox.h>
#include "conversions.h"

// Set oneshot to false to trigger continuous mode when you finisched setting up the whole flow
int oneshot = false;

#define STATUS_OK     0

int temperature;
uint16_t ph;
uint16_t chlorine;
uint16_t temp;

#define ADC_PH A2
#define ADC_CHLORINE A2

uint8_t data[7];

void setup() {

  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  if (oneshot == true) {
    // Wait for the serial
    Serial.begin(96000);
    while (!Serial) {}
  }

  if (!SigFox.begin()) {
    // Something is really wrong, try rebooting
    // Reboot is useful if we are powering the board using an unreliable power source
    // (eg. solar panels or other energy harvesting methods)
    reboot();
  }

  //Send module to standby until we need to send a message
  SigFox.end();

  if (oneshot == true) {
    // Enable debug prints and LED indication if we are testing
    SigFox.debug();
  }
}

void loop() {

  // Start the module
  SigFox.begin();
  
  // Wait at least 30ms after first configuration (100ms before)
  delay(1000);

  float temperature = SigFox.internalTemperature();
  temp = temperature * 100;

  chlorine = analogRead(ADC_CHLORINE);
  ph = analogRead(ADC_PH);


  uint8_t templow = temp & 0xff;
  uint8_t temphigh = (temp >> 8);

  uint8_t chlorinelow = chlorine & 0xff;
  uint8_t chlorinehigh = (chlorine >> 8);

  uint8_t phlow = ph & 0xff;
  uint8_t phhigh = (ph >> 8);
  

  data[0] = 10;
  data[1] = templow;
  data[2] = temphigh;

  data[3] = chlorinelow;
  data[4] = chlorinehigh;

  data[5] = phlow;
  data[6] = phhigh;

  uint8_t size = 7;
  
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.write(data, size); // number of bytes?

  uint8_t last = SigFox.endPacket();

  if (oneshot == true) {

    Serial.print("Chlorine: ");
    Serial.println(chlorine);

    Serial.print("PH: ");
    Serial.println(ph);
    
    Serial.print("Temp: ");
    Serial.println(temp);
    
    Serial.println("Status: " + String(last));
  }

  SigFox.end();

  
  if (oneshot == true) {
    Serial.println("Waiting");
    
    // spin forever, so we can test that the backend is behaving correctly
  //  while (1) {}
  delay(15 * 60 * 1000);
  }
  

  

  //Sleep for 15 minutes
  LowPower.sleep(15 * 60 * 1000);
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}
