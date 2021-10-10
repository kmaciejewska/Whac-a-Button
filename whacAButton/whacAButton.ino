#include "btnFlasher.h"
#include "remoteController.h"
#include "arcadeButtons.h"
#include "reactionTimer.h"
#include "buzzer.h"
#include "lcd.h"
#include "eeprom.h"

#define btn1 0  // for using the btns array
#define btn2 1
#define btn3 2

bool systemInitialized = false; //can we play?

//game start sequence
#define intervalCountDown 1000
//

//game steps
unsigned long previousMillisGameStep = 0;
uint8_t gameStepBtn = 0; //the button that's need to be pushed
uint8_t currentScore = 0;
//

int whackAButtonState = 0; //current state of the game
#define STATE_UNKNOWN  0
#define WAIT_FOR_START 1
#define STARTING       2
#define RUNNING        3

//create instances of objects
IRrecv ir_receiver(A0); //pin of the receiver
RemoteController controller(ir_receiver);

Eeprom eeprom;
RotaryEncoder encoder; //DT and SW pins, CLK already in-class initialized
Adafruit_PCD8544 disp = Adafruit_PCD8544(5, 4, 3);
Lcd lcd(disp, encoder, eeprom);

ReactionTimer myTimer;  //instantiate reaction timer, uses timer0 and millis()

ArcadeButtons buttons(A1);  //btns

BtnFlasher flashers[] = {   //buttons LEDs
  BtnFlasher(7), BtnFlasher(6), BtnFlasher(8)
};

Buzzer buzzer;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0)); //for using random, initialized with a random input on an unconnected pin
  controller.init();
  attachInterrupt(digitalPinToInterrupt(pinA), encoderIsr, LOW); //attach the isr routine to service the interrupts for encoder
}

void loop() {
  for (int i = 0; i < buttons.getBtnsNum(); i++) {
    flashers[i].updateFlasher();  //keep updating the LEDs' state
  }

  runController();  //check for remote controller's incoming values

  if (systemInitialized) {  //is the system turned on by the remote controller?
    lcd.drawMenu();
    buttons.checkBtnsStates();  //check for changes in buttons' state
    checkNextGameStep();  //what is the next step in the game sequence?
  }
}

/////////////////////////////////

void encoderIsr() {
  encoder.readMovementISR();
}

/////////////////////////////////

void runController() {
  if (controller.receive()) {
    if (!systemInitialized) {
      Serial.println("ON");
      turonOnSystem();
    } else {
      Serial.println("OFF");
      //delay(10);
      turnOffandResSystem();
    }
  }
}

/////////////////////////////////

void turonOnSystem() { //add init code here - the screen etc
  lcd.init();
  for (int i = 0; i < buttons.getBtnsNum(); i++) {
    flashers[i].startFlasher(1);
  }
  systemInitialized = true;
  buzzer.playMelody(1);
  whackAButtonState = STATE_UNKNOWN;
}

/////////////////////////////////

void turnOffandResSystem() {
  lcd.resetDefaults();
  lcd.drawMenu();
  for(int i = 0; i < buttons.getBtnsNum(); i++) {
    flashers[i].resetDefaults();
  }
  buttons.resetDefaults();
  systemInitialized = false;
  previousMillisGameStep = 0;
  gameStepBtn = 0;
  currentScore = 0;
  whackAButtonState = 0;
}

/////////////////////////////////

void checkNextGameStep() {
  switch (whackAButtonState) {
    case STATE_UNKNOWN:
      if (lcd.isLevelSelected()) { //if the user selected any game level in the lcd menu
        whackAButtonState = WAIT_FOR_START;
      }
      break;
    case WAIT_FOR_START:
      if (buttons.isBtnPressed(btn1)) {    //if yellow pressed
        whackAButtonState = STARTING;      //start the countdown
      }
      flashers[btn1].setFlashForever(1); //blink yellow continously untill pressed
      break;
    case STARTING:
      gameCountDown();  //countdown
      break;
    case RUNNING:
      playGame(); //main game sequence
      break;
  }
}

/////////////////////////////////

void gameCountDown() { //count down 4 seconds untill the game starts
  flashers[btn1].setFlashForever(0);  //turn off the yellow button
  static uint8_t gameStartCountDown = 4;

  if (millis() - previousMillisGameStep >= intervalCountDown) {
    previousMillisGameStep = millis();
    if (gameStartCountDown > 0) {
      gameStartCountDown--;
      lcd.displayCountDownPage(gameStartCountDown);
      if (gameStartCountDown == 0) {
        whackAButtonState = RUNNING;
        gameStartCountDown = 4;
        previousMillisGameStep = 0;
      }
    }
  }
}

/////////////////////////////////

void playGame() {
  const int gameStepLength = 3500 / lcd.getGameLevel();  //set the time for button pressing

  uint8_t amountOfButtonPresses = 0;
  for (uint8_t btn = 0; btn < buttons.getBtnsNum(); btn++) {
    if (buttons.isBtnPressed(btn)) {
      amountOfButtonPresses++;    //used to determine if any button was pressed
    }
  }


  if (!previousMillisGameStep) {
    lightUpRandLED();
  } else {
    //if one does not press the button in the given time, or presses more than one button, or presses a button but the wrong one, the game is over
    if ((millis() - previousMillisGameStep >= gameStepLength) || (amountOfButtonPresses > 1) || (amountOfButtonPresses == 1) && (!buttons.isBtnPressed(gameStepBtn))) {
      gameOver(currentScore);
    } else if ((buttons.isBtnPressed(gameStepBtn)) && (amountOfButtonPresses == 1)) { //if correct button was pressed, continue the game
      myTimer.endTimer(); //reaction timer running, button released - end timer, calculate duration
      flashers[gameStepBtn].setFlasherState(LOW); //turn off led previous button
      do {
        buttons.checkBtnsStates();
      } while (buttons.isBtnPressed(gameStepBtn));  //wait for the release of the button
      lightUpRandLED();
      currentScore++; //increase the score
      lcd.displayCurrentScorePage(currentScore);
    }
  }
}

/////////////////////////////////

void lightUpRandLED() { //lights up a random LED light
  previousMillisGameStep = millis();
  uint8_t tempStepBtn = 0;
  do {
    tempStepBtn = random(0, buttons.getBtnsNum());
  } while (gameStepBtn == tempStepBtn);   //make sure the LED is not the same as the previous one
  gameStepBtn = tempStepBtn;
  flashers[gameStepBtn].setFlasherState(HIGH); //light up new button light
  myTimer.beginTimer(); //begin reaction timer
}

/////////////////////////////////

void gameOver(uint8_t& score) {
  buzzer.playMelody(2);
  flashers[gameStepBtn].setFlasherState(LOW); //turn off the previous LED button
  char* user = lcd.getUserName();
  bool highScore = false;
  if (eeprom.checkForHighScore(score, user)) {
    highScore = true;
  }
  displayResults(score, user, highScore);
  previousMillisGameStep = 0;
  score = 0;
  lcd.displayGameOverPage();
  whackAButtonState = STATE_UNKNOWN;
}

////////////////////////////////

void displayResults(uint8_t score, char* user, bool ifHighScore) {
  Serial.println();
  Serial.print(F("Game Over, "));
  Serial.println(user);
  Serial.print(F("Your Score: "));
  Serial.print(score);
  Serial.println();
  if(ifHighScore) {
   Serial.print(F("A new highscore!"));
   Serial.println(); 
  }
  int reactionTime = myTimer.calcAvgReact(score);
  Serial.print(F("Your average reaction time is: "));
  Serial.print(reactionTime);
  Serial.print(F(" ms"));
}
