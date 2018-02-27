#define SKETCH_NAME "Window Sensor"
#define SKETCH_VERSION "1.1"

// Enable debug prints to serial monitor
//#define MY_DEBUGS

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_DATARATE RF24_250KBPS
#define MY_RF24_PA_LEVEL RF24_PA_LOW //Options: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define MY_RF24_CHANNEL  1
#define MY_NODE_ID 1  //Manually set the node ID here. Comment out to auto assign
#define MY_TRANSPORT_WAIT_READY_MS 5000 //This will allow the device to function without draining the battery if it can't find the gateway when it first starts up
#define MY_BAUD_RATE 9600 //Serial monitor will need to be set to 9600 Baud

#include <MySensors.h>

#include <Vcc.h>

#define CONTACT_CHILD_ID 0
#define CONTACT_PIN  3  // Arduino Digital I/O pin for button/reed switch
#define CONTACT_CHILD_NAME "Window Sensor 1" //The name of this specific child device will be sent to the controller

#define DEBOUNCE_DELAY 1200 //DO NOT SET BELOW 1000! Amount of time to sleep between reading the contact sensor (used for debouncing)
#define BATTERY_DELAY 14400000 //(4 hours) Amount of time in milliseconds to sleep between battery messages (as long as no interrupts occur)
//#define BATTERY_DELAY 60000

uint8_t oldContactVal = 2; //Used to track last contact value sent.  Starting out of bounds value to force an update when the sensor is first powered on
uint8_t contactVal[2]; //Used for storing contact debounce values
uint8_t contactTracker = 0; //Used as a sort of debounce to stop frequent updates when shifting in chair
int8_t wakeVal = 0; //Used to determine if wake was from timer or interrupt

const float VccMin   = 1.9;           // Minimum expected Vcc level, in Volts. (NRF can only go to 1.9V)
const float VccMax   = 3.3;           // Maximum expected Vcc level, in Volts.
const float VccCorrection = 1.0 / 1.0; // Measured Vcc by multimeter divided by reported Vcc

Vcc vcc(VccCorrection);

MyMessage msgContact(CONTACT_CHILD_ID, V_TRIPPED);

void presentation() {
  // Present sketch name & version
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);

  // Register binary input sensor to gw (they will be created as child devices)
  // You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage.
  // If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
  present(CONTACT_CHILD_ID, S_MOTION, CONTACT_CHILD_NAME);
}
void setup()
{
  disableUnusedStuff(0x3);
  //Set unused pins low to save a little power (http://gammon.com.au/forum/?id=11497)
  for (uint8_t i = 4; i <= 8; i++)
  {
    pinMode (i, INPUT);
    digitalWrite (i, LOW);
  }
  // Setup the button and activate internal pullup resistor
  //pinMode(CONTACT_PIN, INPUT_PULLUP); //(Uses 100uA in sleep)
  pinMode(CONTACT_PIN, INPUT);  //Use this with an external pullup resistor (uses 10uA in sleep)

  float p = vcc.Read_Perc(VccMin, VccMax);
#ifdef MY_DEBUGS
  Serial.print("Batt Level: ");
  Serial.println(p);
#endif
  sendBatteryLevel((uint8_t)p); //Send battery level to gateway
}


void loop()
{
  //Read the value returned by sleep.  If it was a timer send battery info otherwise
  if (wakeVal < 0) {
    //Woke by timer, send battery level and current state of contact sensor
    float p = vcc.Read_Perc(VccMin, VccMax);

#ifdef MY_DEBUGS
    float v = vcc.Read_Volts(); //Only using this for debugging so we don't need it taking up resources normally
    Serial.println("Woke by timer.");
    Serial.print("VCC = ");
    Serial.print(v);
    Serial.println(" Volts");
    Serial.print("VCC = ");
    Serial.print(p);
    Serial.println(" %");
#endif
    sendBatteryLevel((uint8_t)p); //Send battery level to gateway
    send(msgContact.set(oldContactVal ? 0 : 1)); //Send current sensor value
    wakeVal = sleep(digitalPinToInterrupt(CONTACT_PIN), CHANGE, BATTERY_DELAY);  //Go back to sleep
  }
  else {
    //Woke by interrupt, send contact value
    if (contactTracker < 2) {
      contactVal[contactTracker] = digitalRead(CONTACT_PIN);
#ifdef MY_DEBUGS
      Serial.print("contactVal: ");
      Serial.println(contactVal[contactTracker]);
      Serial.print("contactTracker: ");
      Serial.println(contactTracker);
      Serial.println("Sleeping");
#endif
      contactTracker++; //increment contact tracker
      sleep(DEBOUNCE_DELAY); //sleep until next read
      //wait(DEBOUNCE_DELAY);
    }
    else {
      contactTracker = 0; //Reset contact tracker
      uint8_t readValue = digitalRead(CONTACT_PIN);
#ifdef MY_DEBUG
      Serial.print("contactVal: ");
      Serial.println(readValue);
#endif
      if (readValue == contactVal[0] && readValue == contactVal[1]) {
        //All values are the same, send an update
#ifdef MY_DEBUGS
        Serial.println("Values the same");
#endif
        if (oldContactVal != readValue) {
#ifdef MY_DEBUGS
          Serial.println("Contact changed. Sending to gateway");
#endif
          send(msgContact.set(readValue == LOW ? 1 : 0));
          oldContactVal = readValue;
        }
      }
      wakeVal = sleep(digitalPinToInterrupt(CONTACT_PIN), CHANGE, BATTERY_DELAY);  //Go back to sleep
    }
  }

}
inline void disableUnusedStuff(uint8_t clockPrescaler) {

  // Disable the ADC
  //ADCSRA = ADCSRA & B01111111;

  // Disable the analog comparator
  //ACSR = B10000000;

  // Disable digital input buffers on all analog input pins
  //DIDR0 = DIDR0 | B00111111;

  if (clockPrescaler <= 0x8) {
    // Disable interrupts.
    uint8_t oldSREG = SREG;
    cli();
    // Enable change.
    CLKPR = _BV(CLKPCE); // write the CLKPCE bit to one and all the other to zero
    // Change clock division.
    CLKPR = clockPrescaler; // write the CLKPS0..3 bits while writing the CLKPE bit to zero
    // Copy for fast access.
    // Recopy interrupt register.
    SREG = oldSREG;
  }
}
