/* settings.cpp */
/**
This file is part of KanaChordPlus Keyboard.
 Copyright (C) 2024 Mac A. Cody

 KanaChordPlus Keyboard is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

// EPROM support for RP2040 ARM M0 architecture.
// The Philhower RP2040 Arduino implementation support up to 4096 bytes of simulated
// EEPROM at the upper end of the flash memory space.  It appears that the smallest
// size is 256 bytes.  CAUTION! This is actually flash memory, not real EEPROM, so it
// doesn't support +100K writes of real EEPROM.  Frequent updates are not encouraged.
#define EEPROM_EMULATION_SIZE 256
#include <EEPROM.h>
#include "settings.h"

// Signature used at start of a flash memory block to mark the block as
// containing valid data written by the application.
const int WRITTEN_SIGNATURE = 0xDEADAAAA;

struct nonvolatileSettings {
  uint16_t tsCalData[5];
  uint8_t unicodeMode;
} nonvolatileSettings;

/**
 * Function: initNonvolatileStorage - Initial non-volatile storage, if necessary.
 * Input: None.
 * Output: 
 *   Returns true if it was necessary to initialize the storage.
 *   Returns false if already initialized, so settings are present.
 */
bool initNonvolatileStorage(void) {
  int signature;

  EEPROM.begin(EEPROM_EMULATION_SIZE);
  EEPROM.get(0, signature);
  if (signature != WRITTEN_SIGNATURE) {
    EEPROM.put(0, WRITTEN_SIGNATURE);
    return(true);
  } else {
    return(false);
  }
}

/**
 * Function: readNonvolatileSettings - Read the settings from non-volatile storage.
 * Input:
 *   *tsCalData - Pointer to array to hold touch screen calibration data.
 *   *unicodeMode - Pointer to variable to hold Unicode macro mode.
 * Output:
 *   None.
 */
void readNonvolatileSettings(uint16_t *tsCalData, uint8_t *unicodeMode) {
  // Read settings data from flash-based EEPROM.
  EEPROM.get(sizeof(int), nonvolatileSettings);
  tsCalData[0] = nonvolatileSettings.tsCalData[0];
  tsCalData[1] = nonvolatileSettings.tsCalData[1];
  tsCalData[2] = nonvolatileSettings.tsCalData[2];
  tsCalData[3] = nonvolatileSettings.tsCalData[3];
  tsCalData[4] = nonvolatileSettings.tsCalData[4];
  *unicodeMode = nonvolatileSettings.unicodeMode;
}

/**
 * Function: writeNonvolatileSettings - Write the settings to non-volatile storage.
 * Input:
 *   *tsCalData - Pointer to array containing touch screen calibration data.
 *   *unicodeMode - Pointer to variable containing Unicode macro mode.
 * Output:
 *   None.
 */
void writeNonvolatileSettings(uint16_t *tsCalData, uint8_t *unicodeMode) {
  // Read settings data from flash-based EEPROM.
  nonvolatileSettings.tsCalData[0] = tsCalData[0];
  nonvolatileSettings.tsCalData[1] = tsCalData[1];
  nonvolatileSettings.tsCalData[2] = tsCalData[2];
  nonvolatileSettings.tsCalData[3] = tsCalData[3];
  nonvolatileSettings.tsCalData[4] = tsCalData[4];
  nonvolatileSettings.unicodeMode = *unicodeMode;
  EEPROM.put(sizeof(int), nonvolatileSettings);
  EEPROM.commit();
}
