/* KanaChordPlus.ino */
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

#include <Keyboard.h>
#include "display.h"
#include "kdict.h"
#include "kbd_mgt.h"


/**
 * Inter-core communications consists of unsigned, 32-bit values that are pushed onto a
 * fifo by one core and removed from the fifo by the other core.
 * 
 * In this application, data transfers from Core 0 to Core 1 have the bits partitioned
 * as follows:
 * 24-bit Unicode value: bits 0 - 23
 * 8-bit command value: bits 24 - 31
 * The following command values have been defined:
 * 0x00 - If Unicode queue is unmodified, append Unicode to end of queue and process 
 * with the Incremental Input Method Editor (IIME).  If the Unicode queue is modified 
 * (contains Kanji), send the queue content, clear the queue, add the Unicode to the
 * queue, and process with the IIME.
 * 0x01 - Add Unicode to queue, if it is not an Ideographic Space (0x3000), or if the 
 * queue is empty. Then send the queue content.
 * 
 * Data transfers from Core 1 to Core 0 currently utilize only lower 16 bits, which
 * are the Unicode value of the character sent to the computer as a macro sequence.
 */

// Supporting data and test sequences of Hirigana characters and command codes.
// These are to be removed when no longer needed.

/**
 * Kana, Kanji, and CJK symbol characters are converted into keyboard sequences
 * placed in a queue for eventual output to the computer via the USB HID.  As
 * a sequence characters is pulled from the queue and sent to the USB port, a 
 * timer may be set up to delay the transmission of the next sequence character.
 */
#define USB_QUEUE_SIZE 100
#define USB_WAIT_TIME 20
char usbQueue[USB_QUEUE_SIZE];
uint16_t inIdx, outIdx;
unsigned long lastSentTime;

/**
 * Function: insertUSBQueue - Insert USB sequence character into USB queue.
 * The input pointer is incremented, being wrapped back to zero the end
 * of the queue is reached.
 * Input: usbValue - USB sequence character.
 * Output: None.
 */
void insertUSBQueue(char usbValue) {
  usbQueue[inIdx++] = usbValue;
  if (inIdx >= USB_QUEUE_SIZE) {
    inIdx = 0;
  }
}

/**
 * Function: loadUSBQueue - Convert Unicode character value into a USB
 * character sequence and place them into the USB queue for eventual
 * transmission to the USB port.  The sequence formed depends upon the
 * Unicode Mode currently in effect.
 * Input: ucsValue - 16-bit Unicode value.
 * Output: None.
 */
void loadUSBQueue(uint16_t ucsValue) {
  // Convert Unicode value to ASCII by first isolating each nybble.
  // Add ACSII '0' (48) if nybble less than 10, otherwise add ten
  // less than ASCII 'a' (97).
  uint8_t n0 = uint8_t (ucsValue >> 12);
  n0 = n0 < 10 ? 48 + n0 : 87 + n0;
  uint8_t n1 = uint8_t ((ucsValue >> 8) & 0x000f);
  n1 = n1 < 10 ? 48 + n1 : 87 + n1;
  uint8_t n2 = uint8_t ((ucsValue >> 4) & 0x000f);
  n2 = n2 < 10 ? 48 + n2 : 87 + n2;
  uint8_t n3 = uint8_t (ucsValue & 0x000f);
  n3 = n3 < 10 ? 48 + n3 : 87 + n3;
  
  switch (unicodeMode) {
    case 0:
      // Macro sequence for MS Windows.
      // Insert Unicode characters in USB queue.
      insertUSBQueue((char) n0);
      insertUSBQueue((char) n1);
      insertUSBQueue((char) n2);
      insertUSBQueue((char) n3);
      // Insert character to request KEY_LEFT_ALT in USB queue.
      insertUSBQueue('A');
      // Insert character 'x' in USB queue.
      insertUSBQueue('x');
      // Insert character to RELEASE_ALL in USB queue.
      insertUSBQueue('R');
      break;
    case 1:
      // Macro sequence for Linux.
      // Insert character to request KEY_LEFT_SHIFT in USB queue.
      insertUSBQueue('S');
      // Insert character to request KEY_LEFT_CTRL in USB queue.
      insertUSBQueue('C');
      // Insert character 'u' in USB queue.
      insertUSBQueue('u');
      // Insert Unicode characters in USB queue.
      insertUSBQueue((char) n0);
      insertUSBQueue((char) n1);
      insertUSBQueue((char) n2);
      insertUSBQueue((char) n3);
      // Insert character to RELEASE_ALL in USB queue.
      insertUSBQueue('R');
      break;
    case 2:
      // Macro sequence for MacOS (TBD).
      // Insert character to request KEY_OPTION in USB queue.
      // The Option key the same as the Left ALT key.
      insertUSBQueue('A');
      // Insert Unicode characters in USB queue.
      insertUSBQueue((char) n0);
      insertUSBQueue((char) n1);
      insertUSBQueue((char) n2);
      insertUSBQueue((char) n3);
      // Insert character to RELEASE_ALL in USB queue.
      insertUSBQueue('R');
      break;
  }
}

/**
 * Function: sendUSBValue - Send USB character from USB queue
 * to the computer via the USB HID device.
 * Input: None.
 * Output: None.
 */
void sendUSBValue(void) {
  char usbValue;

  // If the previous USB value sent requires a wait period,
  // do not do anything until the wait period has exceeded.
  if (millis() - lastSentTime > USB_WAIT_TIME) {
    // If there are USB values in the queue, take one out
    // of the queue and process it.
    if (outIdx != inIdx) {
      usbValue = usbQueue[outIdx++];
      if (outIdx >= USB_QUEUE_SIZE) {
        outIdx = 0;
      }

      switch (usbValue) {
        case 'A':
          Keyboard.press(KEY_LEFT_ALT);
          // Introduce wait for 20 milliseconds.
          lastSentTime = millis();
          break;
        case 'C':
          Keyboard.press(KEY_LEFT_CTRL);
          // Introduce wait for 20 milliseconds.
          lastSentTime = millis();
          break;
        case 'R':
          Keyboard.releaseAll();
          // Introduce wait for 20 milliseconds.
          lastSentTime = millis();
          break;
        case 'S':
          Keyboard.press(KEY_LEFT_SHIFT);
          // Introduce wait for 20 milliseconds.
          lastSentTime = millis();
          break;
        case 'x':
          Keyboard.press('x');
          // Introduce wait for 20 milliseconds.
          lastSentTime = millis();
          break;
        default:
          Keyboard.write(usbValue);
          break;
      }
    }
  }
}

// Core 0 setup and loop functions.
void setup() {
  rp2040.idleOtherCore();
  // Initialize the USB keyboard emulation.
  Keyboard.begin();
  kbd_setup();
  inIdx = 0;
  outIdx = 0;
  lastSentTime = 0;
  rp2040.resumeOtherCore();
}

void loop() {
  uint16_t ucsValue;

  // Obtain key data from keypads.
  uint32_t kanaValue = kbd_process();
  // This FIFO call WILL be used to send character data to Core 1.
  if (kanaValue != 0x00000000) {
    rp2040.fifo.push((uint32_t) kanaValue);
  }
  
  if (rp2040.fifo.available() > 0) {
    ucsValue = (uint16_t) (rp2040.fifo.pop() & 0x0000ffff);
    loadUSBQueue(ucsValue);
  }

  sendUSBValue();
}

uint16_t ucsQueue[10];
uint16_t queueIdx;
bool queueModified;

/**
 * Function: clear_input_queue() - Clear Unicode character queue.
 * Input: None.
 * Output: None.
 */
void clear_input_queue(void) {
  queueIdx = 0;
  queueModified = false;
}

/**
 * Function: decrement_input_queue - Reduce Unicode character queue by
 * one character, if there are characters in the queue.
 */
void decrement_input_queue(void) {
  if (queueIdx > 0) {
    queueIdx--;
    clear_kanji_list();
    build_kanji_list_data(ucsQueue, queueIdx);
    // Set external variable indicating beginning viewpoint of the displayed Kanji list.
    listStart = 0;
    build_kanji_list();
  }
}

/**
 * Function: kana_to_kanji() - Replaces Kana Unicode values with
 * a selected Kanji Unicode value.
 * Input: Kanji Unicode character value.
 * Output: None.
 */
void kana_to_kanji(uint16_t *ucsList, uint16_t len) {
  uint16_t i;
  for (i = 0; i < len; i++) {
    ucsQueue[i] = ucsList[i];
  }
  queueIdx = i;
  queueModified = true;
}

/**
 * Fuction: send_queue() - Send content of Unicode queue to Core 0.
 * Input: None.
 * Output: None.
 */
void send_queue(void) {
  // Transfer content of ucsQueue to RP2040 FIFO to send to Core 0.
  for (int i = 0; i < queueIdx; i++) {
    rp2040.fifo.push((uint32_t) ucsQueue[i]);
  }
  clear_input_queue();
  clear_kanji_list();
  clear_text();
}

// Core 1 setup and loop functions.
void setup1() {
  delay(10);
  queueIdx = 0;
  queueModified = false;
  display_setup();
  // Construct the main components of the interface.
  make_interface();
}

void loop1() {
  uint32_t ucsValue;
  uint8_t cmdValue;

  if (rp2040.fifo.available() > 0) {
    ucsValue = rp2040.fifo.pop();
    cmdValue = (uint8_t) (ucsValue >> 24);
    ucsValue = (ucsValue & 0x0000ffff);

    if (cmdValue) {
       if ((ucsValue != 0x3000) || (queueIdx == 0)) {
         ucsQueue[queueIdx++] = (uint16_t) ucsValue;
       }

       send_queue();
    } else {
      if (queueModified) {
        send_queue();
      }

      ucsQueue[queueIdx++] = (uint16_t) ucsValue;
      // Insert the kana character into the display after the last character
      // currently displayed in the textarea widget.
      insert_kana_char((uint16_t) ucsValue);
      build_kanji_list_data(ucsQueue, queueIdx);
      // Set external variable indicating beginning viewpoint of the displayed Kanji list.
      listStart = 0;
      build_kanji_list();
    }
  }

  display_handler();
}
