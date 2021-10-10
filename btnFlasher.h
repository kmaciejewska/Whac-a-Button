#ifndef _BTNFLASHER_H
#define _BTNFLASHER_H

#define ledBlinkInterval 750   //interval for blinking the LED - time between on and off

class BtnFlasher {
  private:
    uint8_t LEDPin; //the LED pin
    uint8_t flashForever = 0;  //led may be set to flash continuously
    uint8_t flashCounter = 0;  //to count number of the LED flashes
    uint8_t LEDState = LOW; //current state of the LED
    unsigned long previousLEDMillis = 0; // will store last time LED was updated
  public:
    BtnFlasher(uint8_t pin);
    void updateFlasher(); //update the state of the LED, must be called in the main loop
    void startFlasher(uint8_t flashNumberOfTimes); //light up the LED a given number of times
    void setFlashForever(uint8_t oneOrZero); //set the LED to flash continuously OR turn off the LED already flashing
    void setFlasherState(uint8_t state) { //light up or turn off the LED
      LEDState = state;
      digitalWrite(LEDPin, LEDState);
    }
    void resetDefaults() {
      setFlashForever(0);
      previousLEDMillis = 0;
    }
};

BtnFlasher::BtnFlasher(uint8_t pin) : LEDPin(pin), LEDState(LOW), flashForever(0) {
    pinMode(LEDPin, OUTPUT);  //set the LED pin as output
};

void BtnFlasher::updateFlasher() {
    //check to see if it's time to change the state of the LED
    if ((millis() - previousLEDMillis >= ledBlinkInterval) && ((flashCounter > 0) || (flashForever == 1))) {
        previousLEDMillis = millis();        //remember the time.
        LEDState = !LEDState;
        digitalWrite(LEDPin, LEDState);        //update the state of the LED
        if (flashCounter > 0) {
            flashCounter--; //update the flash counter
        }
    }
}

void BtnFlasher::startFlasher(uint8_t flashNumberOfTimes) {  //flash the LED a given number of times
    flashCounter = flashNumberOfTimes * 2;  //we want to have a full flash - turn on and turn off
    flashForever = 0;
    LEDState = LOW;
    digitalWrite(LEDPin, LEDState); //write LOW to the LED pin
}

void BtnFlasher::setFlashForever(uint8_t oneOrZero) {
    flashForever = oneOrZero; //if 1 - start flashing continuously, if 0 - stop the LED already flashing
    if (!oneOrZero) {
        startFlasher(0);
    }
}

#endif
