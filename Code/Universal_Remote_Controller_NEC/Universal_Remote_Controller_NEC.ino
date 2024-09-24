/**
 * @file Universal_Remote_Controller_NEC.ino
 * @brief This code implements a Universal Reprogrammable Remote Control using IR signals.
 * It can receive, store, and transmit IR signals, supporting multiple devices.
 * The user can switch between remotes and reprogram buttons using a keypad.
 * IR signals are stored in external EEPROM for persistence.
 *
 * Components:
 * - Arduino Board
 * - IR Receiver
 * - IR Transmitter
 * - 8x3 Keypad
 * - External EEPROM (24LC08 or similar)
 * - LEDs for feedback
 * 
 * Developed by: Youssif Hossam, Abdelrahman Khaled
 * Date: September 2024
 */

#include <extEEPROM.h>
#include <Wire.h>
#include <Arduino.h>
#include "PinDefinitionsAndMore.h"  // Define macros for input and output pin etc.

#define POWER_BUTTON_CODE 10
#define MENU_DOWN_REMOTE_BUTTON_CODE 11
#define ZERO_REMOTE_BUTTON_CODE 12
#define CANCEL_REMOTE_BUTTON_CODE 13
#define VOL_UP_REMOTE_BUTTON_CODE 14
#define UP_REMOTE_BUTTON_CODE 15
#define CH_UP_REMOTE_BUTTON_CODE 16
#define LEFT_REMOTE_BUTTON_CODE 17
#define OK_REMOTE_BUTTON_CODE 18
#define RIGHT_REMOTE_BUTTON_CODE 19
#define VOL_DOWN_REMOTE_BUTTON_CODE 20
#define DOWN_REMOTE_BUTTON_CODE 21
#define CH_DOWN_REMOTE_BUTTON_CODE 22

#define PROGRAMMING_BUTTON_CODE 23
#define CHANGE_REMOTE_BUTTON_CODE 24
#define NO_PRESSED_KEY_CODE 0

#define KEYPAD_NO_OF_ROWS 8
#define KEYPAD_NO_OF_COLUMNS 3

byte rowPins[KEYPAD_NO_OF_ROWS] = { 14, 15, 16, 17, 18, 19, 20, 21 };  //connect to the row pinouts of the keypad
byte colPins[KEYPAD_NO_OF_COLUMNS] = { 4, 5, 6 };                      //connect to the column pinouts of the keypad

char hexaKeys[KEYPAD_NO_OF_ROWS][KEYPAD_NO_OF_COLUMNS] = {
  { CHANGE_REMOTE_BUTTON_CODE, PROGRAMMING_BUTTON_CODE, POWER_BUTTON_CODE },
  { 1, 2, 3 },
  { 4, 5, 6 },
  { 7, 8, 9 },
  { MENU_DOWN_REMOTE_BUTTON_CODE, ZERO_REMOTE_BUTTON_CODE, CANCEL_REMOTE_BUTTON_CODE },
  { VOL_UP_REMOTE_BUTTON_CODE, UP_REMOTE_BUTTON_CODE, CH_UP_REMOTE_BUTTON_CODE },
  { LEFT_REMOTE_BUTTON_CODE, OK_REMOTE_BUTTON_CODE, RIGHT_REMOTE_BUTTON_CODE },
  { VOL_DOWN_REMOTE_BUTTON_CODE, DOWN_REMOTE_BUTTON_CODE, CH_DOWN_REMOTE_BUTTON_CODE },

};

#define IR_SEND_PIN 7
#define IR_RECEIVE_PIN 2

#define RG_LED_OFF 0
#define RG_LED_RED 1
#define RG_LED_GREEN 2

#define RG_LED_GREEN_PIN 25
#define RG_LED_RED_PIN 24

#define REMOTE0_LED_PIN 3
#define REMOTE1_LED_PIN 30
#define REMOTE2_LED_PIN 8
#define REMOTE3_LED_PIN 9

/*
  * Specify DistanceWidthProtocol for decoding. This must be done before the #include <IRremote.hpp>
  */
#define DECODE_NEC  // Includes Apple and Onkyo


#include <IRremote.hpp>

#define DEBOUNCE_DELAY 50  // Milliseconds for debounce

//define the cymbols on the buttons of the keypads

#define DELAY_BETWEEN_REPEATS_MILLIS 70

#define NO_OF_REMOTES_SUPPORTED 4
#define NO_OF_BUTTONS_PER_REMOTE 22
#define REMOTE_BUTTONS_CODE_SIZE NO_OF_BUTTONS_PER_REMOTE * sizeof(ButtonRawData)
#define EXT_EEPROM_SIZE_BYTE 1024
// Create an EEPROM object configured at address 0
extEEPROM ext_eeprom(kbits_8, 1, 16);  //device size, number of devices, page size

/* Function prototypes */
IRRawDataType readButtonCodeInEEPROM(uint8_t remoteIndex, uint8_t buttonIndex);
void storeButtonCodeInEEPROM(uint8_t remoteIndex, uint8_t buttonIndex, IRRawDataType data);
void Keypad_init();
uint8_t scanKeypad();

/* Global Variables */
uint8_t currentPressedButtonCode;
uint8_t previousPressedButtonCode;
uint8_t remoteIndex = 0;
IRRawDataType ButtonRawData;

void setup() {
  LEDs_init();
  Keypad_init();

  Serial.begin(115200);
  while (!Serial)
    ;  // Wait for Serial to become available. Is optimized away for some cores.

  uint8_t eepStatus = ext_eeprom.begin(ext_eeprom.twiClock400kHz);  //go fast!
  if (eepStatus) {
    Serial.print(F("extEEPROM.begin() failed, status = "));
    Serial.println(eepStatus);
    while (1)
      ;
  }

  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  IrReceiver.stop();
  IrSender.begin();  // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin

  lightUp_currentRemote_LED(remoteIndex);
  RG_LEG_lightUp(RG_LED_OFF);
  /*
    MyButtonIRData = {random(0xFFFF)};
    storeButtonCodeInEEPROM(0, 15, &MyButtonIRData);
    */
}

void loop() {

  currentPressedButtonCode = scanKeypad();
  if (currentPressedButtonCode) {
    Serial.print("Button ");
    Serial.print(currentPressedButtonCode);
    Serial.println(" is Pressed");
  }

  /*
      * Check for current button state
      */

  if (currentPressedButtonCode) {

    if (currentPressedButtonCode == PROGRAMMING_BUTTON_CODE) {

      Serial.println("Ready to Receive The Signal ...");
      RG_LEG_lightUp(RG_LED_RED);
      bool validSignal = false;
      IrReceiver.start();
      while (!validSignal) {
        // Check if received data is available and if yes, try to decode it.
        if (IrReceiver.decode()) {
          if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
            validSignal = true;  // Set flag when a valid signal is received
            Serial.println(F("Valid IR signal received"));
            IrReceiver.printIRResultShort(&Serial);  // Print the decoded result
          } else {
            Serial.println(F("Received noise or unknown protocol"));
          }
          IrReceiver.resume();  // Enable receiving of the next IR frame
        }
      }

      RG_LEG_lightUp(RG_LED_GREEN);
      Serial.println("Signal Received Successfully :D");
      Serial.println(F("Stop receiving"));
      IrReceiver.stop();

      Serial.println(F("Press the button you want to reprogram .."));
      do {
        currentPressedButtonCode = scanKeypad();
      } while ((currentPressedButtonCode == NO_PRESSED_KEY_CODE) || (currentPressedButtonCode == PROGRAMMING_BUTTON_CODE));

      ButtonRawData = IrReceiver.decodedIRData.decodedRawData;

      if (currentPressedButtonCode != ZERO_REMOTE_BUTTON_CODE) {
        storeButtonCodeInEEPROM(remoteIndex, currentPressedButtonCode, ButtonRawData);
      } else {
        storeButtonCodeInEEPROM(remoteIndex, 0x00, ButtonRawData);
      }
      RG_LEG_lightUp(RG_LED_OFF);

    } else if (currentPressedButtonCode == CHANGE_REMOTE_BUTTON_CODE) {
      remoteIndex++;
      if (remoteIndex == 4) {
        remoteIndex = 0;
      }
      lightUp_currentRemote_LED(remoteIndex);
    } else {

      if (currentPressedButtonCode != ZERO_REMOTE_BUTTON_CODE) {
        ButtonRawData = readButtonCodeInEEPROM(remoteIndex, currentPressedButtonCode);
      } else if (currentPressedButtonCode == ZERO_REMOTE_BUTTON_CODE) {
        ButtonRawData = readButtonCodeInEEPROM(remoteIndex, 0x00);
      } else {
        /* Do Nothing ..*/
      }
      Serial.print(F("Remote "));
      Serial.print(remoteIndex);
      Serial.print(F(" Button "));
      Serial.print(currentPressedButtonCode);
      Serial.print(F(" pressed, now sending: "));
      Serial.print(ButtonRawData, HEX);
      Serial.println();

      Serial.flush();  // To avoid disturbing the software PWM generation by serial output interrupts
      delay(10);
      IrSender.sendNECRaw(ButtonRawData);
      delay(DELAY_BETWEEN_REPEATS_MILLIS);  // Wait a bit between retransmissions
    }
  }
  previousPressedButtonCode = currentPressedButtonCode;
  delay(100);
}



IRRawDataType readButtonCodeInEEPROM(uint8_t remoteIndex, uint8_t buttonIndex) {
  int address = remoteIndex * REMOTE_BUTTONS_CODE_SIZE + buttonIndex * sizeof(IRRawDataType);
  IRRawDataType data = 0;

  // Read byte by byte and reconstruct the IRRawDataType
  for (int i = 0; i < sizeof(IRRawDataType); i++) {
    ((uint8_t*)&data)[i] = ext_eeprom.read(address + i);
  }

  return data;
}

// Function to store button code in EEPROM and verify
void storeButtonCodeInEEPROM(uint8_t remoteIndex, uint8_t buttonIndex, IRRawDataType data) {
  int address = remoteIndex * REMOTE_BUTTONS_CODE_SIZE + buttonIndex * sizeof(IRRawDataType);

  // Write the data byte by byte to EEPROM
  for (int i = 0; i < sizeof(IRRawDataType); i++) {
    ext_eeprom.write(address + i, ((uint8_t*)&data)[i]);
  }

  // Verify by reading back the data
  IRRawDataType readBackData = readButtonCodeInEEPROM(remoteIndex, buttonIndex);

  // Check if the data stored matches the read data
  if (readBackData == data) {
    Serial.println("Data stored successfully.");
  } else {
    Serial.println("Data storage failed.");
  }
}

void lightUp_currentRemote_LED(uint8_t ledIndex) {
  switch (ledIndex) {
    case 0:
      digitalWrite(REMOTE0_LED_PIN, HIGH);
      digitalWrite(REMOTE1_LED_PIN, LOW);
      digitalWrite(REMOTE2_LED_PIN, LOW);
      digitalWrite(REMOTE3_LED_PIN, LOW);
      break;
    case 1:
      digitalWrite(REMOTE0_LED_PIN, LOW);
      digitalWrite(REMOTE1_LED_PIN, HIGH);
      digitalWrite(REMOTE2_LED_PIN, LOW);
      digitalWrite(REMOTE3_LED_PIN, LOW);
      break;
    case 2:
      digitalWrite(REMOTE0_LED_PIN, LOW);
      digitalWrite(REMOTE1_LED_PIN, LOW);
      digitalWrite(REMOTE2_LED_PIN, HIGH);
      digitalWrite(REMOTE3_LED_PIN, LOW);
      break;
    case 3:
      digitalWrite(REMOTE0_LED_PIN, LOW);
      digitalWrite(REMOTE1_LED_PIN, LOW);
      digitalWrite(REMOTE2_LED_PIN, LOW);
      digitalWrite(REMOTE3_LED_PIN, HIGH);
      break;
  }
}
void RG_LEG_lightUp(uint8_t state) {
  switch (state) {
    case RG_LED_OFF:
      digitalWrite(RG_LED_GREEN_PIN, HIGH);
      digitalWrite(RG_LED_RED_PIN, HIGH);
      break;

    case RG_LED_GREEN:
      digitalWrite(RG_LED_GREEN_PIN, LOW);
      digitalWrite(RG_LED_RED_PIN, HIGH);
      break;

    case RG_LED_RED:
      digitalWrite(RG_LED_GREEN_PIN, HIGH);
      digitalWrite(RG_LED_RED_PIN, LOW);
      break;
  }
}

void LEDs_init() {
  pinMode(REMOTE0_LED_PIN, OUTPUT);
  pinMode(REMOTE1_LED_PIN, OUTPUT);
  pinMode(REMOTE2_LED_PIN, OUTPUT);
  pinMode(REMOTE3_LED_PIN, OUTPUT);
  pinMode(RG_LED_GREEN_PIN, OUTPUT);
  pinMode(RG_LED_RED_PIN, OUTPUT);
}

uint8_t scanKeypad() {
    for (int row = 0; row < KEYPAD_NO_OF_ROWS; row++) {
        digitalWrite(rowPins[row], LOW);  // Activate the current row
        delayMicroseconds(10);  // Brief delay for stabilization
        for (int col = 0; col < KEYPAD_NO_OF_COLUMNS; col++) {
            if (digitalRead(colPins[col]) == LOW) {  // Check if the key is pressed
                delay(DEBOUNCE_DELAY);  // Wait for debounce
                // Check again to confirm the key is still pressed
                if (digitalRead(colPins[col]) == LOW) {
                    digitalWrite(rowPins[row], HIGH);  // Deactivate the row
                    return hexaKeys[row][col];  // Return the key value
                }
            }
        }
        digitalWrite(rowPins[row], HIGH);  // Deactivate the current row
    }
    return NO_PRESSED_KEY_CODE;  // No key pressed
}

void Keypad_init(){
  // Initialize row pins as outputs
  for (int i = 0; i < KEYPAD_NO_OF_ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);  // Set them high (not selected)
  }

  // Initialize column pins as inputs
  for (int i = 0; i < KEYPAD_NO_OF_COLUMNS; i++) {
    pinMode(colPins[i], INPUT_PULLUP); // Enable internal pull-up resistors
  }
}