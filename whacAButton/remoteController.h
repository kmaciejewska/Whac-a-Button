#ifndef _REMOTECONTROLLER_H
#define _REMOTECONTROLLER_H

#include <IRremote.h>   //the library uses timer2 interrupts every 50uS

#define remoteCode     0xE0E040BF //the hexadecimal value of the on/off button on our controller; read from the serial monitor - must be corrected if different remote controller is used
#define intervalCheck  800        //the interval required for moving the finger away from the 'on/off' button 

class RemoteController {
  public:
    RemoteController(IRrecv &irrecv);
    void init() {
      _irrecv.enableIRIn();   //start the receiver
    }
    bool receive();
    bool isIRCorrect(long receivedIR) { //check if the received value matches the remote controller code
      return (receivedIR == remoteCode);
    }
  private:
    unsigned long previousMillisCheck = 0;
    IRrecv& _irrecv;
    decode_results results;
};

RemoteController::RemoteController(IRrecv &irrecv) : _irrecv(irrecv) {};  //initialize the reference to IRremote object

bool RemoteController::receive() {  //must run in the main loop
    if (_irrecv.decode(&results))
    {
        long receivedVal = 0;
        if (millis() - previousMillisCheck >= intervalCheck) {  //check if the time required to move the finger away from the button has passed
            previousMillisCheck = millis();
            receivedVal = results.value; //read the remote controller value
            //Serial.println(receivedVal, HEX);
        }
        _irrecv.resume(); //resume receiving the values
        return isIRCorrect(receivedVal);
    }
    return false;
}


#endif
