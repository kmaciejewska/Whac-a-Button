#ifndef _ROTARYENCODER_H
#define _ROTARYENCODER_H

#define pinA              2   //used for generating interrupts using CLK - output A signal, must be 2 or 3
#define debounceDelay     50  //the debounce time
#define interruptInterval 5   //interval for isr in ms
#define longPressTimer    400 //detect long presses
#define pinB              16  //A2 used for reading DT signal - output B
#define pinSW             17  //A3 used for the push button switch         

class RotaryEncoder {
  private:
    //ISR and rotating:
    volatile int pulseCnt = 0;    //updated by the ISR; counts  the pulses sent by the encoder
    int lastCnt =  0;     //stores last position/count of the encoder
    unsigned long lastInterruptTime = 0;  //last time we interrupted on A

    //variables for the switch pressing and debouncing:
    volatile uint8_t swState = HIGH;       // the current reading from the switch, pressed or not?
    uint8_t lastSwState = HIGH;   // the previous reading from the switch - will change
    unsigned long lastDebounceTime = 0;  // the last time of pressing the switch
    unsigned long swTimer = 0; //measures the time switch is pressed down

  public:
    RotaryEncoder();
    void readMovementISR(); //will be invoked by an interrupt on CLK
    void updateCounter(); //update the position of the encoder
    void checkSwPresses();  //debounce the switch, update the state
    int getValue() { //get the current value
      return lastCnt;
    }
    bool isSwPressed() { //check the current state of the switch
      return !swState; //LOW means pressed
    }
    bool getLongPress() {
      if ((millis() - swTimer >= longPressTimer) && swState) {
        return true;
      }
      return false;
    }
    void setPulseCnt(uint8_t val) {
      pulseCnt = val;
    }
    void resetDefaults() { //for turning off the game
      pulseCnt = 0;
      lastCnt =  0;
      lastInterruptTime = 0;
      swState = HIGH;
      lastSwState = HIGH;
      lastDebounceTime = 0;
      swTimer = 0;
    }
};

RotaryEncoder::RotaryEncoder() {
    pinMode(pinA, INPUT); //set rotary pulses as input
    pinMode(pinB, INPUT);
    pinMode(pinSW, INPUT_PULLUP);   //switch is floating so use the in-built arduino PULLUP
}

void RotaryEncoder::readMovementISR() {
    if (millis() - lastInterruptTime > interruptInterval) {  //if interrupts come faster than the interval, assume it's a bounce and ignore
        if (digitalRead(pinB) == LOW) { //if B is low (the same as A), we have counter-clockwise movement of the encoder
            pulseCnt--;
        }
        else {
            pulseCnt++;
        }
    }
    lastInterruptTime = millis();    // update the interrupt timer - remember the last time it was triggered (no more than every 5ms)
}

void RotaryEncoder::updateCounter() { //must be run in the loop
    if (pulseCnt != lastCnt) {       //if the current rotary switch position has changed then update everything
        // Write out to serial monitor the value and direction
        //Serial.print(pulseCnt > lastCnt ? "Right: " : "Left: ");
        //Serial.println(pulseCnt);
        lastCnt = pulseCnt;           //remember the new value
    }
}

void RotaryEncoder::checkSwPresses() {  //must be run in the main loop
    uint8_t currentState = digitalRead(pinSW);   //read the state of the switch

    //If the switch changed, due to noise or pressing:
    if (currentState != lastSwState) {
        lastDebounceTime = millis();    //reset the debouncing timer
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {  //we have waited longer than the required debounce delay
        // if the button state has changed:
        if (currentState != swState) {
            swState = currentState; //update it
            if (!swState) { //LOW means pressed
                swTimer = millis(); //to detect a long press
            }
        }
    }
    // save the reading. Next time through the loop, it'll be the lastSwState:
    lastSwState = currentState;
}

#endif
