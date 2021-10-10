#ifndef _BUZZER_H
#define _BUZZER_H

#include <toneAC.h>
#include "melody.h"

#define tempo        200 //change those to make the song slower or faster
#define melodyLength 12

class Buzzer {
    //this calculates the duration of a whole note in ms
    const int wholeNote = (60000 * 4) / tempo;
  public:
    //buzzer is connected to digital pins 9 and 10, however theres no need to use them in any way - toneAC makes use of timer1 PWM pins - 9 and 10
    Buzzer();
    void playMelody(uint8_t whichMelody);
    void play(int melody[]);
};

Buzzer::Buzzer() {};

void Buzzer::playMelody(uint8_t whichMelody) {
    int *melody;
    switch (whichMelody) {
        case 1:
            melody = melodyInit;
            break;
        case 2:
            melody = melodyGameOver;
            break;
    }
    play(melody);
}

void Buzzer::play(int melody[]) {
    uint8_t noteCounter = 0;
    int noteDuration = 0;
    unsigned long previousMillisPlaying = 0;
    do {
        if (millis() - previousMillisPlaying > (noteDuration + 0.1 * noteDuration)) {
            previousMillisPlaying = millis();
            int divider = melody[noteCounter + 1];
            if (divider > 0) {
                // regular note, just proceed
                noteDuration = (wholeNote) / divider;
            } else if (divider < 0) {
                noteDuration = (wholeNote) / abs(divider); //dotted notes are represented with negative durations
                noteDuration *= 1.5; // increases the duration in half for dotted notes
            }
            toneAC(melody[noteCounter], 10, noteDuration * 0.9); //play the note for 90% of the duration
            noteCounter += 2;
        }
    } while (noteCounter < melodyLength * 2);
}

#endif
