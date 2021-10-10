#ifndef _REACTIONTIMER_H
#define _REACTIONTIMER_H

class ReactionTimer {
  private:
    unsigned long startTime;  //when the timer began measuring time?
    unsigned long duration;   //the time between a light being light up and the button being pressed
  public:
    ReactionTimer();
    void beginTimer();
    void endTimer();
    int calcAvgReact(int correctButtonPresses);  //calculates average reaction time
};

ReactionTimer::ReactionTimer() : startTime(0), duration(0) {};

void ReactionTimer::beginTimer() {
    startTime = millis(); //record the timer start time
}

void ReactionTimer::endTimer() {
    duration += (millis() - startTime);    //sum up every durations in order to calculate average reaction time in ms
}

int ReactionTimer::calcAvgReact(int correctButtonPresses) {
    int avgReactTime = 0.00;
    if (correctButtonPresses) {
        avgReactTime = (duration / correctButtonPresses);    //divide by the amount of button presses - current score/amount of correct btn presses
    }
    duration = 0; //reset for the next time playing
    return avgReactTime;
}


#endif
