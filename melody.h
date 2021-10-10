#ifndef _MELODY_H
#define _MELODY_H

#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define REST     0

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth, etc
// negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth
int melodyInit[] = {
  NOTE_E5, 8, NOTE_E5, 8, REST, 8, NOTE_E5, 8, REST, 8, NOTE_C5, 8, NOTE_E5, 8,
  NOTE_G5, 4, REST, 4, NOTE_G4, 8, REST, 4, REST, 4
};

int melodyGameOver[] = {
  NOTE_C5, -4, NOTE_G4, -4, NOTE_E4, 4,
  NOTE_A4, -8, NOTE_B4, -8, NOTE_A4, -8, NOTE_GS4, -8, NOTE_AS4, -8, NOTE_GS4, -8,
  NOTE_G4, 8, NOTE_D4, 8, NOTE_E4, -2
};

#endif
