#ifndef _ARCADEBUTTONS_H
#define _ARCADEBUTTONS_H

#define btnsNum      3 //the number of buttons-leds used
#define TOL          20 //we need some kind of a tolerance, when reading a given button value, since sometimes the signal flickers
#define debounceTime 10 //debounce time in milliseconds 

class ArcadeButtons {
  private:
    //buttons:
    const int btnsValues[btnsNum] = {895, 395, 205};  //analog value of the button (range 0-1023), read by means of serial monitor and analogRead function; must be changed if different values are detected
    uint8_t btnsPin;    //since we use a voltage divider circuit - saves us 2 PINs, we connected all the buttons to one analog pin
    uint8_t btnState[btnsNum] {}; //for storing states of buttons - HIGH/LOW
    //debouncing:
    unsigned long lastDebounceTime = 0;    //last time a button was pressed
    //

  public:
    ArcadeButtons(uint8_t btnsPin);    //initialize buttons
    const static uint8_t getBtnsNum() { //how many buttons?
      return btnsNum;
    }
    void readBtnValue();
    bool getBtnState(uint8_t btnIndex);   //read given btn value and determine - is button pressed?
    void checkBtnsStates();     //check for buttons state changes; debounce and update state
    bool isBtnPressed(uint8_t btnIndex) { //for managing the btnState array
      return btnState[btnIndex];
    }
    void resetDefaults() {
      for (int i = 0; i < btnsNum; i++) {
        btnState[i] = LOW;
      }
      lastDebounceTime = 0;
    }
};

ArcadeButtons::ArcadeButtons(uint8_t btnsPin) : btnsPin(btnsPin) {};

void ArcadeButtons::readBtnValue() {    //for reading the analog value of a button; must be put in the main loop
    Serial.println(analogRead(btnsPin));  //print the value of a pressed button
}

//read current buttons value, if the value is in the tolerable range, the button is pressed - state HIGH;
bool ArcadeButtons::getBtnState(uint8_t atIndex) {
    return ((analogRead(btnsPin) <= (btnsValues[atIndex] + TOL)) && (analogRead(btnsPin) >= (btnsValues[atIndex] - TOL)));  //buttons pin is connected to ground (through the pull-down resistor)
}                                                                                                                          //and we read a LOW whenever the button is not pressed

void ArcadeButtons::checkBtnsStates() { //check for button changes
    uint8_t currentBtnState; //declared static to prevent redefinitions

    //debounce
    if ((lastDebounceTime + debounceTime) > millis()) {
        return; //not enough time has passed to debounce, so wait longer
    }
    //we have waited DEBOUNCE milliseconds, so reset the timer
    lastDebounceTime = millis();
    //

    for (int i = 0; i < btnsNum; i++) {
        currentBtnState = getBtnState(i);       //read the button
        if (currentBtnState != btnState[i]) {   //if the previous button state has changed
            btnState[i] = currentBtnState;        //set it as the current value
        }
    }
}

#endif
