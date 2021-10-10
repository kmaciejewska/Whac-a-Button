#ifndef _EEPROM_H
#define _EEPROM_H

#include <Wire.h>

#define chipAddress    0x50 //the addres of the memory chip is determined by the A2, A1, A0 pins (datasheet)
#define leadersNum     3
#define leader1Addr    0
#define leader2Addr    33
#define leader3Addr    65
#define highScore1Addr 32
#define highScore2Addr 64
#define highScore3Addr 96
#define MAX_WRITE      16
#define BYTES_PER_PAGE 64

class Eeprom {
  private:
    unsigned int leadersAddresses[leadersNum] = {leader1Addr, leader2Addr, leader3Addr};
    unsigned int scoreAddresses[leadersNum] = {highScore1Addr, highScore2Addr, highScore3Addr};
  public:
    Eeprom();
    void initChip(unsigned int eeAddress);
    uint8_t readChip(unsigned int eeAddress);
    void writeStringToChip(unsigned int eeAddress, char* data);
    void writeNumberToChip(unsigned int eeAddress, byte data);
    //void initWrite();
    void readLeaders();
    bool checkForHighScore(byte score, char* username);
    /*void readAddresses() {
      for (uint8_t i = 0; i < leadersNum; i++) {
        Serial.println();
        Serial.print(leadersAddresses[i], DEC); //0, 37, 69
        Serial.println();
        Serial.print(scoreAddresses[i], DEC);  //32, 64, 96
        delay(1000);
      }
    } */
};

Eeprom::Eeprom() {
    Wire.begin();
    Wire.setClock(400000);
}

void Eeprom::initChip(unsigned int eeAddress) {
    Wire.beginTransmission(chipAddress);
    Wire.write((int)(eeAddress >> 8)); //MSB
    Wire.write((int)(eeAddress & 0xFF)); //LSB
}

/*void Eeprom::initWrite() {
  unsigned int address = 0; //start at address zero
  char usernamePlace[10] = {'\n', 32, 32, 32, 32, 32, 32, 32, 32};  //write blank space for username
  char* writeOut[2] = {usernamePlace, "\nScore: "}; //add another string in the new line for the score
  for (int i = 0; i < leadersNum; i++) {
    leadersAddresses[i] = address;  //store the address of the first byte in the username
    for (int k = 0; k < 2; k++) {
      writeStringToChip(address, writeOut[k]);
    }
    scoreAddresses[i] = address;    //store the address of score
    writeNumberToChip(address, 0);  //write initially 0 - will be overwritten
    address += 1;
  }
} */

uint8_t Eeprom::readChip(unsigned int eeAddress) {
    uint8_t rdata = 0xFF;
    initChip(eeAddress);
    Wire.endTransmission();           //end transmission
    Wire.requestFrom(chipAddress, 1); //request one byte from the slave device
    if (Wire.available()) {           //slave may not send anything at all
        rdata = Wire.read();            //receive the byte
    }
    return rdata;
}

void Eeprom::writeStringToChip(unsigned int eeAddress, char* data) { //in initWrite was &eeAddress to get the continuity of the addresses
    /*
 * EEPROM_PAGE_WRITE
 *
 * Example program showing a method for writing large amounts of
 * data to an eeprom (24LC256/512 etc), whilst avoiding page write
 * boundaries
 *
 * copyright www.hobbytronics.co.uk 2012
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version. see <http://www.gnu.org/licenses/> */

    // Uses Page Write for 24LC256
    // Allows for 64 byte page boundary
    // Splits string into max 16 byte writes
    unsigned char i = 0, counter = 0;
    unsigned int  pageSpace;
    unsigned int  page = 0;
    unsigned int  numWrites;
    unsigned int  dataLen = 0;
    unsigned char firstWriteSize;
    unsigned char lastWriteSize;
    unsigned char writeSize;

    // Calculate length of data
    do {
        dataLen++;
    } while (data[dataLen]);

    // Calculate space available in first page
    pageSpace = int(((eeAddress / BYTES_PER_PAGE) + 1) * BYTES_PER_PAGE) - eeAddress;

    // Calculate first write size
    if (pageSpace > MAX_WRITE) {
        firstWriteSize = pageSpace - ((pageSpace / MAX_WRITE) * MAX_WRITE);
        if (firstWriteSize == 0) firstWriteSize = MAX_WRITE;
    }
    else
        firstWriteSize = pageSpace;

    // calculate size of last write
    if (dataLen > firstWriteSize)
        lastWriteSize = (dataLen - firstWriteSize) % MAX_WRITE;

    // Calculate how many writes we need
    if (dataLen > firstWriteSize)
        numWrites = ((dataLen - firstWriteSize) / MAX_WRITE) + 2;
    else
        numWrites = 1;

    i = 0;
    for (page = 0; page < numWrites; page++) {
        if (page == 0) writeSize = firstWriteSize;
        else if (page == (numWrites - 1)) writeSize = lastWriteSize;
        else writeSize = MAX_WRITE;

        initChip(eeAddress);
        counter = 0;
        do {
            Wire.write((byte) data[i]);
            i++;
            counter++;
        } while ((data[i]) && (counter < writeSize));
        Wire.endTransmission();

        eeAddress += writeSize; // Increment address for next write
        delay(6);  // needs 5ms for page write
    }

}

void Eeprom::writeNumberToChip(unsigned int eeAddress, byte data) {
    initChip(eeAddress);
    Wire.write(data);
    Wire.endTransmission();
    delay(5);     //needs a small delay for writing
}

void Eeprom::readLeaders() {
    Serial.println();
    for (unsigned i = 0; i < highScore3Addr + 5; i++) {
        byte rdata = readChip(i);
        if (i == scoreAddresses[0] || i == scoreAddresses[1] || i == scoreAddresses[2]) { //if we are reading from the scores addresses
            Serial.print(rdata);  //use print to visualize numbers instead of ASCII codes
            Serial.println();
        } else {
            Serial.write(rdata);
        }
    }
}

bool Eeprom::checkForHighScore(byte score, char* username) {
    for (int i = 0; i < leadersNum; i++) {    //we have 3 leaders
        if ((score > (readChip(scoreAddresses[i])))) { //if the score made the "leaderboard"
            if (i != leadersNum - 1) {  //if its not the last user on the list
                //copy the beaten score and move it to the next position:
                char previousUser[7] = {32, 32, 32, 32, 32, 32};
                for (unsigned j = 0; j < 6; j++) {
                    previousUser[j] = readChip(leadersAddresses[i] + j);
                }
                byte previousScore = readChip(scoreAddresses[i]);
                writeStringToChip(leadersAddresses[i + 1], previousUser); //move
                writeNumberToChip(scoreAddresses[i + 1], previousScore);
            }
            //add the new score to the list:
            writeStringToChip(leadersAddresses[i], username);
            writeNumberToChip(scoreAddresses[i], score);
            return true;
        }
    }
    return false;
}

#endif
