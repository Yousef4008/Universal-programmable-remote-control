# Universal Reprogrammable Remote Control

This project implements a **Universal Reprogrammable Remote Control** system using Arduino, allowing users to control multiple devices through IR signals. It features the ability to store and reprogram buttons for up to four remotes and persistently save these configurations to external EEPROM memory.

## Table of Contents
- [Features](#features)
- [Components](#components)
- [Software Overview](#software-overview)
- [How It Works](#how-it-works)
- [Usage](#usage)
- [Future Improvements](#future-improvements)

## Features
- **Multi-Remote Support:** Supports up to 4 different remote configurations, each with 22 buttons.
- **Reprogrammable Buttons:** Users can reprogram buttons by storing new IR signals for each button via a keypad interface.
- **EEPROM Storage:** Button configurations are saved in external EEPROM, ensuring persistence even after a power cycle.
- **IR Transmission and Reception:** Receive and send IR signals using an IR receiver and transmitter module.
- **LED Feedback:** RGB and single-color LEDs provide visual feedback during programming and remote switching.

## Components
- **ATmega32**
- **IR Receiver** (e.g., TSOP1738)
- **IR Transmitter** (IR LED)
- **8x3 Keypad** (for selecting buttons and reprogramming)
- **External EEPROM** (24C08 or similar)
- **LEDs** (for feedback)
- **Resistors** (appropriate values for LEDs and keypad)

## Software Overview

The code is written in Arduino's `.ino` format and uses the following libraries:
- **IRremote:** For handling IR transmission and reception.
- **Wire:** For I2C communication with the EEPROM.
- **extEEPROM:** To interface with external EEPROM (24LC08 or similar).

### Key Sections of the Code
- **Keypad Initialization and Scanning:** 
    - The keypad is scanned to detect button presses, which are mapped to corresponding IR codes or actions.
- **EEPROM Read/Write Functions:**
    - `storeButtonCodeInEEPROM`: Stores IR data for a specific button.
    - `readButtonCodeInEEPROM`: Retrieves stored IR data for a specific button.
- **IR Signal Handling:**
    - Receives IR signals for reprogramming and sends stored IR signals for button presses.

### EEPROM Configuration
- EEPROM is partitioned into sections, each corresponding to one remote and its 22 buttons. Each button's data is stored as raw IR data.

## How It Works

1. **Remote Switching:**
    - The user can switch between four remotes using the keypad.
    - The active remote's indicator LED lights up.
  
2. **Programming Mode:**
    - Press the "Programming" button to enter programming mode. The remote is now ready to receive a new IR signal.
    - After receiving the IR signal, select the button to associate with this signal. The system will store it in EEPROM for later use.

3. **Normal Operation:**
    - Press any button to send the associated IR signal, which is retrieved from EEPROM and transmitted via the IR LED.

## Usage

1. **Upload the Code:**
    - Open the `.ino` file in the Arduino IDE.
    - Ensure all required libraries (`IRremote`, `extEEPROM`, etc.) are installed.
    - Upload the code to your Arduino board.

2. **Operating the Remote:**
    - Use the keypad to select a button and send the associated IR signal.
    - Enter programming mode by pressing the programming button, then point an existing remote at the IR receiver to capture a signal.

## Future Improvements

- **Add More Remote Support:** Expand the number of remotes by using a larger EEPROM.
- **Different Protocols:** Add support for additional IR protocols beyond NEC.
- **Improved UI:** Use an LCD screen to provide better feedback for button reprogramming.

## Credits
Developed by:
- Youssif Hossam
- Abdelrahman Khaled

Date: September 2024
