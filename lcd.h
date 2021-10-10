#ifndef _LCD_H
#define _LCD_H

#include <Adafruit_PCD8544.h>
#include <Adafruit_GFX.h>
#include "rotaryEncoder.h"
#include "eeprom.h"

#define OFF         0
#define UNKNOWN     1
#define MENU        2
#define SELECT_USER 3
#define OPTIONS     4
#define CONTRAST    5
#define GAME_LEVEL  6
#define GAME_OVER   7

class Lcd {
  public:
    Lcd(Adafruit_PCD8544 &disp, RotaryEncoder& enc, Eeprom& eeprom);
    void init();
    void drawMenu();
    void switchPage();
    void displayEncVal();
    void displayInitSet(char* textToDisp);
    void displayMenuItem(char* item, int posY, boolean selected, int posX = 5);
    void displayUserNamePage();
    void displayLevelPage(int value);
    void restrictUserName(int encVal);
    void restrictGameLevel(int encVal);
    void displayCountDownPage(uint8_t countDownVal);
    void displayCurrentScorePage(int score);
    void displayGameOverPage();
    void displayMainMenuPage();
    bool isLevelSelected() {
      return levelFlag;
    }
    uint8_t getGameLevel() {
      return gameLevel;
    }
    char* getUserName() {
      return userName;
    }
    void resetDefaults() {  //for turning off the game
      _display.clearDisplay();
      encoder.resetDefaults();
      currentMenuItem = 0;
      for (int i = 0; i < 8; i++) { //requires to be null terminated
        userName[i] = 32;
      }
      currentLetter = 0;
      gameLevel = 0;
      pos = 10;
      page = OFF;
      levelFlag = false;
    }
  private:
    Adafruit_PCD8544& _display;
    RotaryEncoder& encoder;
    Eeprom& eeprom;
    uint8_t currentMenuItem = 0;
    char userName[9] = {32, 32, 32, 32, 32, 32, 32, 32}; //requires to be null terminated
    uint8_t currentLetter = 0;        // so 1st letter in the username
    uint8_t gameLevel = 0;
    uint8_t pos = 10;
    uint8_t page = 0;
    bool levelFlag = false;
};

Lcd::Lcd(Adafruit_PCD8544 &disp, RotaryEncoder& enc, Eeprom& eeprom) : _display(disp), encoder(enc), eeprom(eeprom) {};

void Lcd::init() {
    _display.begin();
    _display.clearDisplay();
    page = MENU;
}

void Lcd::drawMenu() {
    encoder.checkSwPresses(); //check for encoder's switch input
    encoder.updateCounter();  //update encoder's position
    displayEncVal();
    switchPage();

    switch (page) {
        case OFF:
            _display.clearDisplay();
            break;
        case MENU:
            displayMainMenuPage();
            break;
        case SELECT_USER:
            displayUserNamePage();
            break;
        case GAME_LEVEL:
            displayLevelPage(gameLevel);
            break;
        case GAME_OVER:
            displayGameOverPage();
            break;
    }

    _display.display();

}

void Lcd::switchPage() {
    if (encoder.isSwPressed()) {   //if we press the rotary switch
        encoder.setPulseCnt(0); //reset the encoder's counter
        while (encoder.isSwPressed()) { //wait for the release of the switch
            encoder.checkSwPresses();
        }
        switch (page) {
            case MENU:
                if (currentMenuItem == 1) { //PLAY
                    page = SELECT_USER;
                } else if (currentMenuItem == 2) { //LEADERBOARD
                    eeprom.readLeaders();    //view it in the Serial port
                }
                break;
            case SELECT_USER:
                currentLetter++;
                pos += 6;
                if (currentLetter > 7 || encoder.getLongPress()) {
                    page = GAME_LEVEL;
                    currentLetter = 0;  //reset back to default for the next time
                    pos = 10;
                }
                break;
            case GAME_LEVEL:
                levelFlag = true;
                page = UNKNOWN;
                _display.clearDisplay();
                break;
            case GAME_OVER:
                if (currentMenuItem == 1) { //YES
                    page = GAME_LEVEL;
                } else if (currentMenuItem == 2) { //NO
                    for (int i = 0; i < 8; i++) { //reset the name
                        userName[i] = 32;
                    }
                    page = MENU;
                }
                break;
        }
    }
}

void Lcd::displayEncVal() {
    int encVal = encoder.getValue();
    if (page == MENU || page == GAME_OVER) {
        //we have 2 items on the page
        currentMenuItem = abs(encVal % 2) + 1;    //highlight the current position of the encoder
    } else if (page == SELECT_USER) {
        userName[currentLetter] = encVal + 65;
        restrictUserName(encVal);
    } else if (page == GAME_LEVEL) {
        gameLevel = encVal + 1;
        restrictGameLevel(encVal);
    }
}

void Lcd::restrictUserName(int encVal) {
    //in the counter we start from zero, but the letter was set to 'A', so
    if (encVal > 25) {    //if we (rotating clockwise) get to the counter value bigger than ASCII code 'Z'
        userName[currentLetter] = 90;   //set the letter as 'Z"
        encoder.setPulseCnt(0);         //reset the encoder's counter
    } else if (encVal < 0) {          //if we rotate counterclockwise
        userName[currentLetter] = encVal + 91;  //encVal is negative, so add ASCII code 'Z" + 1 =
        // (-1 + 91 = 90 = 'Z', -2 + 91 = 89 = 'Y' etc.
        if (userName[currentLetter] < 65) {     //if we get 'lower' than 'A'
            userName[currentLetter] = 65;         //loop again
            encoder.setPulseCnt(0);
        }
    }
}

void Lcd::restrictGameLevel(int encVal) {
    if (encVal < 0) {
        gameLevel = encVal + 11;
        if (gameLevel < 1) {
            gameLevel = 1;
            encoder.setPulseCnt(0);
        }
    } else if (encVal > 9) {
        gameLevel = 10;
        encoder.setPulseCnt(0);
    }
}

void Lcd::displayInitSet(char* textToDisp) {
    _display.setTextSize(1);
    _display.clearDisplay();
    _display.setTextColor(BLACK, WHITE);
    _display.setCursor(10, 0);
    _display.print(textToDisp);
    _display.drawFastHLine(0, 10, 83, BLACK);
}

void Lcd::displayMenuItem(char* item, int posY, boolean selected, int posX = 5) {
    if (selected == 1) {
        _display.setTextColor(WHITE, BLACK);
    } else {
        _display.setTextColor(BLACK, WHITE);
    }
    _display.setCursor(posX, posY);
    _display.print(F(">"));
    _display.print(item);
}

void Lcd::displayMainMenuPage() {
    displayInitSet("MENU");
    if (currentMenuItem == 1) {
        displayMenuItem("PLAY", 15, true);
        displayMenuItem("LEADERBOARD", 25, false);
    } else if (currentMenuItem == 2) {
        displayMenuItem("PLAY", 15, false);
        displayMenuItem("LEADERBOARD", 25, true);
    }
}

void Lcd::displayUserNamePage() {
    displayInitSet("SELECT NAME");
    _display.setCursor(10, 25);
    _display.print(userName);
    _display.setCursor(pos, 35);
    _display.print(F("^"));
}

void Lcd::displayLevelPage(int value) {
    displayInitSet("GAME LEVEL");
    _display.setTextSize(2);
    _display.setCursor(10, 20);
    _display.print(value);
}

void Lcd::displayCountDownPage(uint8_t countDownVal) {
    _display.clearDisplay();
    _display.setTextSize(2);
    _display.setCursor(30, 20);
    _display.print(countDownVal);
    if (!countDownVal) {
        _display.setCursor(20, 20);
        _display.print(F("GO!!"));
    }
}

void Lcd::displayCurrentScorePage(int score) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.print(F("Score:"));
    _display.setTextSize(2);
    _display.setCursor(30, 20);
    _display.print(score);
}

void Lcd::displayGameOverPage() {
    page = GAME_OVER;
    levelFlag = false;
    displayInitSet("GAME OVER");
    _display.setCursor(0, 15);
    _display.print(F("Play again?"));
    if (currentMenuItem == 1) {
        displayMenuItem("YES", 35, true, 10);
        displayMenuItem("NO", 35, false, 40);
    } else if (currentMenuItem == 2) {
        displayMenuItem("YES", 35, false, 10);
        displayMenuItem("NO", 35, true, 40);
    }
}

#endif
