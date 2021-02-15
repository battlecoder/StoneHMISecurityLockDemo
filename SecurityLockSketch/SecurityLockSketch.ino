/************************************************/
 /* SecurityLockSketch.ino                      */
 /*****************************************************************************/
 /* Sketch for the full Security Lock Panel demo made for the Stone LCD HMI Displays.
  *  The repository for this project is https://github.com/battlecoder/StoneHMISecurityLockDemo
  *  From that repo you can obtain the LCD project (designed for a 800x480 module) and this Sketch.
  *  
  *  This Sketch requires my StoneLCDLib library:
  *     https://github.com/battlecoder/StoneLCDLib
  *
  *  And the MFRC522 library:
  *    https://github.com/miguelbalboa/rfid
  *  ** That can also be obtained from the Library manager
  * 
  * This project is configured to communicate with the Stone HMI using the native USART Port from
  * the Arduino board. For a Nano, Pro Mini and Uno, these are the connections:
  *  ARDUINO      MODULE                PIN
  *  -----------------------------------------------
  *  0 (RX)       TTL-RS232 Converter   RX <-
  *  1 (TX)       TTL-RS232 Converter   TX ->
 
  * The RFID reader on the other hand, should be connected following this mapping:
  * (taken from the RC-522 library example)
  * -----------------------------------------------------------------------------------------
  *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
  *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
  * Signal      Pin          Pin           Pin       Pin        Pin              Pin
  * -----------------------------------------------------------------------------------------
  * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
  * SPI SS      SDA(SS)      10            53        D10        10               10
  * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
  * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
  * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
  * 
 */

#include <SPI.h>
#include <MFRC522.h>
#include <StoneLCDLib.h>
/*############################################################################
 *##                                                                        ##
 *##                               D E F I N E S                            ##
 *##                                                                        ##
 *############################################################################*/
/* Pin assignment *******************************/
#define SS_PIN                10
#define RST_PIN               9

/* Limits ***************************************/
#define MAX_RECV_BUFFER       4
#define PINPAD_CODE_LEN       4
#define RFIDCARD_ID_LEN       4

/* LCD Constants ********************************/
#define LCD_PAGE_INPUTPANEL   0
#define LCD_PAGE_GREYEDPANEL  1

#define LCD_VAR_PIN_lENGTH    0x007B
#define LCD_VAR_PINPAD_KEY    0x007D
#define LCD_VAR_LOCK_STATUS   0x007C

#define LCD_LOCKSTAT_LOCKED   2   
#define LCD_LOCKSTAT_UNLOCKED 1

#define LCD_SOUND_TAP         0
#define LCD_SOUND_SUCCESS     1
#define LCD_SOUND_FAILURE     2

#define LCD_BACKSPACE_KEY     0x0A
#define LCD_CLEAR_KEY         0x0B

#define SFX_VOLUME            0x10

/* Login/Security Definitions *******************/
/* Change to EEPROM or User-DB settings (provided by the LCD) later */
const uint8_t CORRECT_CARD_ID[RFIDCARD_ID_LEN] = {0x1B, 0x90, 0x69, 0xC5};
const uint8_t CORRECT_PIN_NUM[PINPAD_CODE_LEN] = {2, 4, 8, 6};

/*############################################################################
 *##                                                                        ##
 *##                                G L O B A L                             ##
 *##                                                                        ##
 *############################################################################*/
MFRC522       rfid(SS_PIN, RST_PIN);
StoneLCD      myLCD (&Serial);
StoneLCDEvent evt;

uint16_t      recvBuffer[MAX_RECV_BUFFER];
uint8_t       nuidPICC[RFIDCARD_ID_LEN];
uint8_t       pinPadBuffer[PINPAD_CODE_LEN];
uint8_t       curPinPadLen;
boolean       lockActive;

/*############################################################################
 *##                                                                        ##
 *##                 P I N P A D    F U N C T I O N S                       ##
 *##                                                                        ##
 *############################################################################*/
void clearPinPadBuffer (){
  for (uint8_t i = 0; i < PINPAD_CODE_LEN; i++) {
    pinPadBuffer[i] = 0;
  }
  curPinPadLen = 0;
}

bool checkPinPadBufferMatch (){
  if (curPinPadLen != PINPAD_CODE_LEN) return false;

  for (uint8_t i = 0; i < PINPAD_CODE_LEN; i++) {
    if (pinPadBuffer[i] != CORRECT_PIN_NUM[i]) return false;
  }
  return true;
}

bool addPinPadCharToBuffer (uint8_t c){
  if (curPinPadLen >= PINPAD_CODE_LEN) return false;

  pinPadBuffer[curPinPadLen] = c;
  curPinPadLen++;
  return true;
}

bool truncatePinPadCharByOne (){
  if (curPinPadLen < 1) return false;

  pinPadBuffer[curPinPadLen] = 0;
  curPinPadLen--;
  return true;
}

/*############################################################################
 *##                                                                        ##
 *##                      U I    F U N C T I O N S                          ##
 *##                                                                        ##
 *############################################################################*/
void refreshPinPadLengthOnUI(){
  myLCD.writeVariableWord (LCD_VAR_PIN_lENGTH, curPinPadLen);
}

void refreshLockIndicatorOnUI(){
  myLCD.writeVariableWord (LCD_VAR_LOCK_STATUS, lockActive ? LCD_LOCKSTAT_LOCKED : LCD_LOCKSTAT_UNLOCKED);
}

void showDisabledUI(int interval){
  myLCD.setCurrentPage(LCD_PAGE_GREYEDPANEL);
  delay (interval);
  myLCD.setCurrentPage(LCD_PAGE_INPUTPANEL);
}

/*############################################################################
 *##                                                                        ##
 *##                    L O C K    F U N C T I O N S                        ##
 *##                                                                        ##
 *############################################################################*/
void updateLock (){
  // TO-DO: Operate physical lock based on lockActiveValue
}

void toggleSecurityLock() {
  lockActive = !lockActive;
  updateLock();
}

/*############################################################################
 *##                                                                        ##
 *##                    M I S C    F U N C T I O N S                        ##
 *##                                                                        ##
 *############################################################################*/
void doAccessError() {
  refreshPinPadLengthOnUI();
  refreshLockIndicatorOnUI();
  clearPinPadBuffer();
  myLCD.playSound(LCD_SOUND_FAILURE, SFX_VOLUME);
  delay(200);
  showDisabledUI(800);
}

void doAccessSuccess(){
  toggleSecurityLock();
  refreshPinPadLengthOnUI();
  refreshLockIndicatorOnUI();
  clearPinPadBuffer();
  myLCD.playSound(LCD_SOUND_SUCCESS, SFX_VOLUME);
  delay(1000);
}
/*############################################################################
 *##                                                                        ##
 *##                    R F I D    F U N C T I O N S                        ##
 *##                                                                        ##
 *############################################################################*/
bool readCardID(){
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent()) return false;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial()) return false;

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    return false;
  }

  // Store NUID into nuidPICC array
  for (byte i = 0; i < RFIDCARD_ID_LEN; i++) nuidPICC[i] = rfid.uid.uidByte[i];

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  return true;
}

bool checkRFIDmatch(){
  for (uint8_t i = 0; i < RFIDCARD_ID_LEN; i++) {
    if (nuidPICC[i] != CORRECT_CARD_ID[i]) return false;
  }
  return true;
}

/*############################################################################
 *##                                                                        ##
 *##                                 S E T U P                              ##
 *##                                                                        ##
 *############################################################################*/
void setup() { 
  Serial.begin(19200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  delay(1000);
  myLCD.clearInputStream();

  // Initial application status
  lockActive = true;
  clearPinPadBuffer();
  updateLock();
  // UI
  refreshPinPadLengthOnUI();
  refreshLockIndicatorOnUI();
}

/*############################################################################
 *##                                                                        ##
 *##                                  L O O P                               ##
 *##                                                                        ##
 *############################################################################*/
void loop() {
  boolean recvEvent;
  byte valAsByte;

  // Continuosly try to read the card
  if (readCardID()){
    if (checkRFIDmatch()){
      doAccessSuccess();
    } else {
      doAccessError();
    }
    // Update UI
    refreshPinPadLengthOnUI();
    refreshLockIndicatorOnUI();
  }
  
  recvEvent = myLCD.checkForIOEvent(&evt, recvBuffer, MAX_RECV_BUFFER);
  if (recvEvent){
    valAsByte = recvBuffer[0];

    switch (evt.address){
      case LCD_VAR_PINPAD_KEY:
        if (valAsByte == LCD_BACKSPACE_KEY) {
          truncatePinPadCharByOne();
        }else if (valAsByte == LCD_CLEAR_KEY) {
          clearPinPadBuffer();
        }else {
          addPinPadCharToBuffer (valAsByte);
        }

        if (curPinPadLen < PINPAD_CODE_LEN) {
          // Since we receive the keypad events when the user lifts their finger
          // from the buttons, the timing for a "keypad press" SFX is weird. I'm
          // leaving here this for testing, and as a reminder to explore that topic
          // again in future revisions of their framework, if they change this.
          // myLCD.playSound(LCD_SOUND_TAP, SFX_VOLUME);
        } else {
          if (checkPinPadBufferMatch()){
            doAccessSuccess();
          } else {
            doAccessError();
          }
        }
      break;
    }
    // Let's clear pending messages, if any.
    myLCD.clearInputStream();

    // Update UI
    refreshPinPadLengthOnUI();
    refreshLockIndicatorOnUI();
  }
}
