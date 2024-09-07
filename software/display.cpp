/* display.cpp */
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


/* Using LVGL with Arduino requires some extra steps:
  Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html */
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "display.h"
#include "settings.h"
#include "kdict.h"
#include "loop1.h"
#define MAX_REAL_LIST 10

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

// Global pointers for LVGL widgets.
static lv_obj_t *kanji_text, *kanji_list, *okuri_list, *settings;
// Global variables for display colors.
lv_color_t wht_color, red_color, orn_color, ylw_color, grn_color, blu_color;
LV_FONT_DECLARE(KanaKanjiFont32);

// Global storage for display calibration data.
uint16_t tsCalData[5] = {0, 0, 0, 0, 0};
// Index for beginning index of listbox 'window' into klist_data, the Kanji
// list generated from the dictionary.
uint16_t listStart = 0;
// Absolute index into klist_data, the Kanji list generated from the dictionary.
uint16_t klistIdx = 0;

// Unicode output mode: 0 - MS Windows, 1 - Linux, 3 - MacOS
uint8_t unicodeMode = 1;

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  tft.startWrite();
  tft.setAddrWindow( area->x1, area->y1, w, h );
  tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.endWrite();

  lv_disp_flush_ready( disp_drv );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch( &touchX, &touchY, 600 );

  if ( !touched ) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

/**
 * Function: display_setup() - Initize the LVGL system, associated
 * drivers, and data.  Also retrieve keyboard settings, initializing
 * if they don't already exist.
 * Input: None.
 * Output: None.
 */
void display_setup(void) {
  lv_init();
  tft.begin();          /* TFT init */
  tft.setRotation( 3 ); /* Landscape orientation, flipped */

  // At this point, check EEPROM for existing calibration data.
  /* Set the touchscreen calibration data,
     the actual data for your display can be acquired using
     the Generic -> Touch_calibrate example from the TFT_eSPI library
  */
  if (initNonvolatileStorage()) {
    // Creating non-volatile storage and calibrating...
    tft.calibrateTouch(tsCalData, 0xffff, 0x0000, 15);
    writeNonvolatileSettings(tsCalData, &unicodeMode);
  } else {
    // Retrieving settings from non-volatile storage.
    readNonvolatileSettings(tsCalData, &unicodeMode);
  }
  tft.setTouch(tsCalData);

  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );

  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

  // Define colors used by the Kanji selection list.
  wht_color = lv_color_make(0xFF, 0xFF, 0xFF);
  red_color = lv_color_make(0xED, 0x75, 0x75);
  orn_color = lv_color_make(0xFF, 0x99, 0x33);
  ylw_color = lv_color_make(0xB3, 0xB3, 0x23);
  grn_color = lv_color_make(0x45, 0xD1, 0x45);
  blu_color = lv_color_make(0x7E, 0x7E, 0xED);
}

/**
 * Function: hexUCS2utf8str - Convert hexidecimal Unicode value to UTF8 string.
 * Note that a properly formatted string with Null terminator is generated.
 * Input: utf8chars - Pointer to char array that will hold UTF* characters.
 *        hexUCS - Unicode number.
 * Output: None.
 */
void hexUCS2utf8str(char *utf8chars, uint16_t hexUCS) {
  utf8chars[0] = 0xe0 | ((hexUCS >> 12) & 0x000f);
  utf8chars[1] = 0x80 | ((hexUCS >> 6) & 0x003f);
  utf8chars[2] = 0x80 | (hexUCS & 0x003f);
  utf8chars[3] = 0x00;
}

/**
 * Function: utf8str2hexUCS
 * NOTE: Code points greater than 0xffff are not supported.
 */
enum scanState_enum {
  utf8idle, // Ready for first byte of UTF-8 string.
  utf8cont, // Ready for UTF-8 continuation byte.
  utf8done  // Abort further scanning of string.
};
int utf8str2hexUCS(uint16_t *hexUCSlist, char *utf8chars) {
  scanState_enum scanState = utf8idle;
  char *utf8ptr;
  utf8ptr = utf8chars; // Is this really necessary?
  int byteCount, hexCount = 0;
  uint16_t hexValue = 0;
  do {
    //Serial.println("Top of loop.");
    switch (scanState) {
      case utf8idle:
        if (*utf8ptr == 0x00) {
          // Null character encountered, so done scanning.
          scanState = utf8done;
        } else if ((*utf8ptr & 0x80) == 0x00) {
          // Code points 0x01 - 0x7f, so output directly.
          // Single-byte Unicode value.
          hexUCSlist[hexCount++] = (uint16_t) *utf8ptr++;
        } else if ((*utf8ptr & 0xe0) == 0xc0) {
          // Code points 0x80 - 0x7ff.  Has top five bits.
          // Two-byte Unicode value.
          hexValue = (uint16_t) (*utf8ptr++ & 0x1f);
          byteCount = 1; // One more byte is anticipated.
          scanState = utf8cont; // Continuation state.
        } else if ((*utf8ptr & 0xf0) == 0xe0) {
          // Three-byte Unicode value.
          // Code points 0x0800 - 0xffff.  Has top four bits.
          hexValue = (uint16_t) (*utf8ptr++ & 0x0f);
          byteCount = 2; // Two more bytes are anticipated.
          scanState = utf8cont; // Continuation state.
        } else {
          // Error! Abort further scanning of string.
          // "Error! Unexpected character.
          scanState = utf8done;
        }
        break;
      case utf8cont:
        if ((*utf8ptr & 0xc0) == 0x80) {
          // Continuation character.  Isolate and shift in six bits.
          hexValue = (hexValue << 6) | (uint16_t) (*utf8ptr++ & 0x3f);
          //byteCount--;
          if (--byteCount == 0) {
            // All needed continuation bytes have been scanned in.
            hexUCSlist[hexCount++] = hexValue;
            hexValue = 0; // This may not be necessary.
            scanState = utf8idle; // Return to idle state for next UTF-8 string.
          }
        } else {
          // Error! Abort further processing of string.
          scanState = utf8done;
        }
        break;
    }
  } while(scanState != utf8done);
  return(hexCount);
}

/**
 * Function: set_kana_text - Convert the 16-bit Unicode characters in the
 * queue to UTF8 strings and write them into the LVGL textarea widget.
 * Input:
 *   queueData - Pointer to array of 16-bit Unicode characters.
 *   queueLen - Number of valid Unicode characters in the array.
 * Output: None.
 */
void set_kana_text(uint16_t *queueData, uint16_t queueLen) {
  char utf8str[30];

  for (int i = 0; i < queueLen; i++) {
    hexUCS2utf8str(utf8str + 3*i, queueData[i]);
  }

  //lv_textarea_set_text(kanji_text, charstr);
  lv_textarea_set_text(kanji_text, utf8str);
}

/**
 * Function: insert_kana_char - Convert a 16-bit Unicode character into a
 * UTF8 string and insert it at the right end of the LVGL textarea widget.
 * Input: ucsData - 16-bit Unicode character.
 * Output: None.
 */
void insert_kana_char(uint16_t ucsData) {
  char utf8str[4];
  // Convert 16-bit Unicode to UTF8 and insert into display.
  hexUCS2utf8str(utf8str, ucsData);
  lv_textarea_add_text(kanji_text, utf8str);
}

/**
 * Function: clear_text - Clear content of LVGL textarea widget.
 * Input: None.
 * Output: None.
 */
void clear_text(void) {
  lv_textarea_set_text(kanji_text, "");
}

/**
 * Function: close_okuri_window_cb
 */
static void close_okuri_window_cb(lv_event_t * e) {
  // Retrieve the Cancel button object.
  lv_obj_t * obj = lv_event_get_target(e);
  // Retrieve the Window banner object.
  obj = lv_obj_get_parent(obj);
  // Retrieve the Window object.
  obj = lv_obj_get_parent(obj);
  // Delete the window object.
  lv_obj_del(obj);
}

/**
 * Function: kanji_only_cb
 */
static void kanji_only_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  
  if (code == LV_EVENT_CLICKED) {
    char utf8str[4];
    hexUCS2utf8str(utf8str, klist_data[klistIdx].kmd->unicode);
    // Set the LVGL textarea widget to display the Kanji character.
    lv_textarea_set_text(kanji_text, utf8str);
    kana_to_kanji((uint16_t *) &klist_data[klistIdx].kmd->unicode, 1);
    // Close the window after
    close_okuri_window_cb(e);
  }
}

/**
 * Function: okuri_button_cb()
 */
static void okuri_button_cb(lv_event_t * e) {
  lv_area_t btnCoords;
  lv_point_t point;
  char utf8str[4], iconstr[30];
  uint16_t ucsList[10];
  lv_event_code_t code = lv_event_get_code(e);
  
  if (code == LV_EVENT_CLICKED) {
    lv_obj_t * obj = lv_event_get_target(e);
    uint16_t listNdx = lv_obj_get_index(obj);  // Index of list item.
    lv_obj_t * btn = lv_obj_get_child(obj, 0); // Get button of list item.
    lv_obj_get_coords(btn, &btnCoords);
    lv_indev_t * indev = lv_indev_get_act();
    lv_indev_get_point(indev, &point);
    if (point.x < btnCoords.x2) {
      // Clicked on the Kanji character of the button.
      // Convert the Kanji Unicode into a UTF8 string.
      hexUCS2utf8str(utf8str, klist_data[klistIdx].kmd->unicode);
      // Combine the kanji UTF-8 string and okurigana UTF-8 string.
      strcpy(iconstr, utf8str);
      strcat(iconstr, klist_data[klistIdx].omd->clist[listNdx]);
      // Set the LVGL textarea widget to display the Kanji character.
      lv_textarea_set_text(kanji_text, iconstr);
      // Text displayed in text area.
      // Replaces Kana Unicode values with selected Kanji Unicode value.
      uint16_t hexCount = utf8str2hexUCS(ucsList, iconstr);
      kana_to_kanji(ucsList, hexCount);
      // Close the okurigana window.
      // Retrieve the list object.
      obj = lv_obj_get_parent(obj);
      // Retrieve the window content object.
      obj = lv_obj_get_parent(obj);
      // Retrieve the Window object.
      obj = lv_obj_get_parent(obj);
      // Delete the window object.
      lv_obj_del(obj);
    }
  }
}

/**
 * Function: build_okuri_window
 */
void build_okuri_window() {
  char utf8str[4];
  char iconstr[30];
  lv_obj_t * lbl;

  // Building okuri window.
  // Create window object.
  lv_obj_t * win = lv_win_create(lv_scr_act(), 40);
  lv_obj_set_size(win, 320, 240);
  // Generate UTF8 Unicode string of Kanji.
  hexUCS2utf8str(utf8str, klist_data[klistIdx].kmd->unicode);
  // Create 'Kanji only' button on left end of banner.
  lv_obj_t * btn = lv_win_add_btn(win, utf8str, 40);
  // Set the button to use KanaKanjiFont32.
  lv_obj_set_style_text_font(btn, &KanaKanjiFont32, 0);
  // Attach callback function for 'Kanji only' button.
  lv_obj_add_event_cb(btn, kanji_only_cb, LV_EVENT_CLICKED, NULL);
  // Create the title for the window.
  lv_win_add_title(win, "Okurigana");
  // Can the title string be centered somehow?
  
  // Create 'Cancel' button on bottom right end of banner.
  btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 40);
  // Attach callback function to 'Cancel' button.
  lv_obj_add_event_cb(btn, close_okuri_window_cb, LV_EVENT_CLICKED, NULL);
  // Retrieve Content object.
  lv_obj_t * cont = lv_win_get_content(win);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
  // Create list object in window object.
  okuri_list = lv_list_create(cont);
  lv_obj_set_size(okuri_list, 300, 180);
  // Create buttons for list for up to size limit, adding paging button if necessary.
  for (int i = 0; i < klist_data[klistIdx].omd->len; i++) {
    strcpy(iconstr, utf8str);
    strcat(iconstr, klist_data[klistIdx].omd->clist[i]);
    switch (klist_data[klistIdx].omd->alist[i]) {
      case none:
        btn = lv_list_add_btn(okuri_list, iconstr, "Simple reading");
        break;
      case prefix:
        btn = lv_list_add_btn(okuri_list, iconstr, "Prefix reading");
        break;
      case suffix:
        btn = lv_list_add_btn(okuri_list, iconstr, "Suffix reading");
        break;
      }
    
    // lv_obj_set_style_bg_color(btn, bg_clr, LV_PART_MAIN);
    // The first child widget (index 0) is the 'icon', which is a label.
    lbl = lv_obj_get_child(btn, 0);
    // Set the child font.
    lv_obj_set_style_text_font(lbl, &KanaKanjiFont32, 0);
    // Attach callback function to list object.
    lv_obj_add_event_cb(btn, okuri_button_cb, LV_EVENT_CLICKED, NULL);
  }
}

/**
 * Function: kanji_list_cb -  Callback function for the LVGL list buttons for
 * Kanji values.  The button pressed references the Kanji Unicode character 
 * that replaces the Kana characters displayed in the LVGL textarea widget 
 * and replaces the Kana characters in the UCS queue.
 * Input: Pointer to LVGL event structure.
 * Output: None.
 */
static void kanji_list_cb(lv_event_t * e) {
  lv_area_t btnCoords;
  lv_point_t point;
  char utf8str[4];
  uint16_t hexCount;
  uint16_t ucsList[20];
  
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);
  affix_enum affix;
  char okuri[10];
  
  if (code == LV_EVENT_CLICKED) {
    uint16_t listNdx = lv_obj_get_index(obj);  // Index of list item.
    lv_obj_t * btn = lv_obj_get_child(obj, 0); // Get button of list item.
    lv_obj_get_coords(btn, &btnCoords);
    lv_indev_t * indev = lv_indev_get_act();
    lv_indev_get_point(indev, &point);
    if (point.x < btnCoords.x2) {
      // Clicked button on Kanji symbol.
      // Determine appropriate index into the Kanji list.
      klistIdx = listStart + listNdx - 1;
      switch (klist_data[klistIdx].type) {
        case KUNYOMI:
        case ONYOMI:
        case NANORI:
          // Determine if list of okurigana is not available.
          if (klist_data[klistIdx].omd != NULL) {
            // Pointer to okurigana metadata is not NULL.
            // Call function to display okurigana window.
            build_okuri_window();
          } else {
            // There are no affixes or Okurigana!
            // Convert the Kanji Unicode into a UTF8 string.
            hexUCS2utf8str(utf8str, klist_data[klistIdx].kmd->unicode);
            // Set the LVGL textarea widget to display the Kanji character.
            lv_textarea_set_text(kanji_text, utf8str);
            // Replaces Kana Unicode values with selected Kanji Unicode value.
            kana_to_kanji((uint16_t *) &klist_data[klistIdx].kmd->unicode, 1);
          }
          break;
        case DICTIONARY:
            // Set the LVGL textarea widget to display the Kanji character.
            lv_textarea_set_text(kanji_text, (char *) klist_data[klistIdx].omd->clist[0]);
            hexCount = utf8str2hexUCS(ucsList, (char *)klist_data[klistIdx].omd->clist[0]);
            kana_to_kanji(ucsList, hexCount);
          break;
      }
    }
  }
}

/**
 * Function: kanji_nav_up_cb - Callback function for the LVGL list button for
 * movement of the LVGL list 'window' towards more common Kanji characters.
 * This button will be the first button in the list, but may me hidden.
 * Input: Pointer to LVGL event structure.
 * Output: None.
 */
static void kanji_nav_up_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    // Change the beginning of the viewpoint of the displayed Kanji list
    // towards the more common Kanji and regenerate the Kanji list.
    listStart -= MAX_REAL_LIST;
    build_kanji_list();
    // Place view on last button of list.
    lv_obj_t * obj = lv_obj_get_child(kanji_list, lv_obj_get_child_cnt(kanji_list) - 1);
    lv_obj_scroll_to_view(obj, LV_ANIM_ON);
    // Moving to more-common Kanji entries.
  }
}

/**
 * Function: kanji_nav_dn_cb - Callback function for the LVGL list button for
 * movement of the LVGL list 'window' towards less common Kanji characters.
 * This button will be the last button in the list, but may me hidden.
 * Input: Pointer to LVGL event structure.
 * Output: None.
 */
static void kanji_nav_dn_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    // Change the beginning of the viewpoint of the displayed Kanji list
    // towards the less common Kanji and regenerate the Kanji list.
    listStart += MAX_REAL_LIST;
    build_kanji_list();
    // Place view on last button of list.
    lv_obj_t * obj = lv_obj_get_child(kanji_list, 0);
    lv_obj_scroll_to_view(obj, LV_ANIM_ON);
    // Moving to less-common Kanji entries.
  }
}

/**
 * Function clear_kanji_list - Removes all buttons from the Kanji list LVGL 
 * list widget except for the first and last buttons, which are used for
 * navigation of the list 'window' along the Kanji list.  The navigation
 * buttons are hidden as they may not need to be displayed.
 * Input: None.
 * Output: None.
 */
void clear_kanji_list(void) {
  lv_obj_t * btn;
  
  // Remove existing list elements, if any, except for first and last.
  uint32_t btnCount = lv_obj_get_child_cnt(kanji_list);
  for (int i = 1; i < btnCount - 1; i++) {
    btn = lv_obj_get_child(kanji_list, 1);
    lv_obj_del(btn);
  }

  // Hide the navigation buttons.
  btn = lv_obj_get_child(kanji_list, 0);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
  btn = lv_obj_get_child(kanji_list, 1);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Function: make_list_button - Create a button for the Kanji list LVGL list widget,
 * setting up icon, text, and background color.
 * Input:
 *   *icon - Pointer to icon to be displayed.
 *   *text - Pointer to character string to be displayed.
 *   bg_clr - LVGL structure representing the background color of the list button.
 * Output: Pointer to list button object.
 */
lv_obj_t* make_list_button(const void *icon, const char *text, lv_color_t bg_clr) {
  lv_obj_t *btn, *lbl;
  // Add button to the list, seting its background color.
  btn = lv_list_add_btn(kanji_list, icon, text);
  lv_obj_set_style_bg_color(btn, bg_clr, LV_PART_MAIN);
  // The first child widget (index 0) is the 'icon', which is a label.
  lbl = lv_obj_get_child(btn, 0);
  // Set the child font.
  lv_obj_set_style_text_font(lbl, &KanaKanjiFont32, 0);
  return (btn);
}

/**
 * Function: build_kanji_list - Create and display list buttons according to
 * the content of the Kanji data list provided by build_kanji_list_data (found
 * in kdict.cpp).
 * Input: None.
 * Output: None.
 */
void build_kanji_list(void) {
  uint16_t buttonCount = 0, countLimit;
  lv_obj_t * btn;
  lv_color_t color;
  char utf8str[4];

  // Remove existing list elements, if any, except for first and last,
  // which are navigation buttons.  The navigation buttons are hidden.
  clear_kanji_list();

  if (totalListSize != 0) {
    if (totalListSize > MAX_REAL_LIST) {
      if (listStart == 0) {
        // At beginning of list, so hide top list navigation button,
        // but show bottom list navigation button.
        btn = lv_obj_get_child(kanji_list, 0);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        btn = lv_obj_get_child(kanji_list, 1);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
        countLimit = MAX_REAL_LIST;
      } else if (listStart + MAX_REAL_LIST <= totalListSize) {
        // Somewhere in middle of list, so show both top list and
        // bottom list navigation button.
        btn = lv_obj_get_child(kanji_list, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
        btn = lv_obj_get_child(kanji_list, 1);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
        countLimit = listStart + MAX_REAL_LIST;
      } else {
        // At end of list, so show top list navigation button, but
        // hide the bottom list navigation button.
        btn = lv_obj_get_child(kanji_list, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
        btn = lv_obj_get_child(kanji_list, 1);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        countLimit = totalListSize;
      }
    } else {
      // Lists that are "short" don't need the list navigation buttons.
      // They are hidden by the call to clear_kanji_list() above.
      countLimit = totalListSize;
    }

    for (int i = listStart; i < countLimit; i++) {
      // At this point, the lowest reading will cause a button to be added to the
      // list widget with the appropriate Kanji, meaning string, and color.  If
      // no reading was chosen, the all cases will be skipped.
      switch (klist_data[i].type) {
        case KUNYOMI:
          color = grn_color;
          break;
        case ONYOMI:
          color = blu_color;
          break;
        case NANORI:
          color = ylw_color;
          break;
        case DICTIONARY:
          color = orn_color;
          break;
      }

      switch (klist_data[i].type) {
        case KUNYOMI:
        case ONYOMI:
        case NANORI:
          // Convert Unicode value to UTF8 string.
          hexUCS2utf8str(utf8str, klist_data[i].kmd->unicode);
          // Create a button for the Kanji list.
          btn = make_list_button(utf8str, klist_data[i].kmd->meaning, color);
          break;
        case DICTIONARY:
          // Create a button for the Kanji list.
          btn = make_list_button(klist_data[i].omd->clist[0], klist_data[i].omd->clist[1], color);
          break;
      }

      // Add the callback to the list button.
      lv_obj_add_event_cb(btn, kanji_list_cb, LV_EVENT_CLICKED, NULL);
      buttonCount++;
    }

    // Obtain new button count and move button for
    // next lesser-ranked set to end of list.
    uint32_t btnCount = lv_obj_get_child_cnt(kanji_list);
    btn = lv_obj_get_child(kanji_list, 1);
    lv_obj_move_to_index(btn, btnCount - 1);
  }
}

static uint32_t radiobutton_index = 0;
/**
 * Function: create_radiobutton
 */
void create_radio_button(lv_obj_t * parent, const char * txt, int x, int y) {
  lv_obj_t * obj = lv_checkbox_create(parent);
  lv_obj_set_pos(obj, x, y);
  lv_checkbox_set_text(obj, txt);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
}

/**
 * Function: radiobutton_event_cb
 */
static void radiobutton_event_cb(lv_event_t * e) {
  //uint32_t * active_id = lv_event_get_user_data(e);
  lv_obj_t * cont = lv_event_get_current_target(e);
  lv_obj_t * act_cb = lv_event_get_target(e);
  //lv_obj_t * old_cb = lv_obj_get_child(cont, *active_id);
  lv_obj_t * old_cb = lv_obj_get_child(cont, radiobutton_index);

  /*Do nothing if the container was clicked*/
  if(act_cb == cont) return;
  lv_obj_clear_state(old_cb, LV_STATE_CHECKED);   /*Uncheck the previous radio button*/
  lv_obj_add_state(act_cb, LV_STATE_CHECKED);     /*Uncheck the current radio button*/
  //*active_id = lv_obj_get_index(act_cb);
  radiobutton_index = lv_obj_get_index(act_cb);
}

/**
 * Function: save_macro_mode_cb
 */
static void save_macro_mode_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  
  if (code == LV_EVENT_CLICKED) {
    unicodeMode = radiobutton_index;
    writeNonvolatileSettings(tsCalData, &unicodeMode);
    close_okuri_window_cb(e);
  }
}

/**
 * Function: create_macro_select - Create window displaying Macro
 * radio buttons
 */
void create_macro_select_window() {
  lv_obj_t * win = lv_win_create(lv_scr_act(), 40);
  lv_obj_set_size(win, 320, 240);
  // Create 'Save' button on left end of banner.
  lv_obj_t * btn = lv_win_add_btn(win, LV_SYMBOL_OK, 40);
  // Attach callback function for 'Save' button.
  lv_obj_add_event_cb(btn, save_macro_mode_cb, LV_EVENT_CLICKED, NULL);
  // Create the title for the window.
  lv_win_add_title(win, "Select Macro Output Mode");
  // Create 'Cancel' button on bottom right end of banner.
  btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 40);
  // Attach callback function to 'Cancel' button.
  lv_obj_add_event_cb(btn, close_okuri_window_cb, LV_EVENT_CLICKED, NULL);
  // Retrieve Content object.
  lv_obj_t * cont = lv_win_get_content(win);
  // Add radiobutton callback function.
  radiobutton_index = unicodeMode;
  //lv_obj_add_event_cb(cont, radiobutton_event_cb, LV_EVENT_CLICKED, &radiobutton_index);
  lv_obj_add_event_cb(cont, radiobutton_event_cb, LV_EVENT_CLICKED, NULL);
  // Construct 'radio buttons' for the macro output modes.
  create_radio_button(cont, "MS Windows Unicode Input", 0, 0);
  create_radio_button(cont, "Linux Unicode Input", 0, 30);
  create_radio_button(cont, "MacOS Unicode Input", 0, 60);
  // Make the checkbox checked according to the unicode mode currently selected.
  lv_obj_add_state(lv_obj_get_child(cont, unicodeMode), LV_STATE_CHECKED);
}

/**
 * Function: calibrate_cb - Callback function to execute the
 * touchscreen calibration routine and save the calibration
 * data to non-volatile memory.
 * Input: Pointer to LVGL event structure.
 * Output: None.
 */
static void calibrate_cb(lv_event_t * e)
{
  tft.calibrateTouch(tsCalData, 0xffff, 0x0000, 15);
  tft.setTouch(tsCalData);
  // At this point, update the EEPROM storage.
  tft.fillRect(0, 0, 16, 16, 0xffff);
  tft.fillRect(0, 224, 16, 16, 0xffff);
  tft.fillRect(304, 0, 16, 16, 0xffff);
  tft.fillRect(304, 224, 16, 16, 0xffff);
  writeNonvolatileSettings(tsCalData, &unicodeMode);
}

/**
 * Function create_touch_calibration_window()
 */
void create_touch_calibration_window(void) {
  lv_obj_t * win = lv_win_create(lv_scr_act(), 40);
  lv_obj_set_size(win, 320, 240);
  // Create the title for the window.
  lv_win_add_title(win, "               Touch Screen Calibration");
  // Retrieve Content object.
  lv_obj_t * cont = lv_win_get_content(win);

  // Create label to contain text.
  lv_obj_t * label = lv_label_create(cont);
  lv_obj_set_pos(label, 10, 40);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
  lv_label_set_text(label, "Press Start Calibration to start.  Touch corners in order indicated.  When done, close the message.");
  lv_obj_set_width(label, 300);  /*Set smaller width to make the lines wrap*/
 
  // Create 'Start' button.
  lv_obj_t * btn = lv_btn_create(cont);
  lv_obj_set_pos(btn, 10, 100);
  lv_obj_add_event_cb(btn, calibrate_cb, LV_EVENT_CLICKED, NULL);
  //lv_obj_align(btn, LV_ALIGN_CENTER, 0, -40);
  label = lv_label_create(btn);
  lv_label_set_text(label, "Start Calibration");
  lv_obj_center(label);

  // Create 'Close' button on bottom right end of banner.
  btn = lv_btn_create(cont);
  lv_obj_set_pos(btn, 180, 100);
  lv_obj_add_event_cb(btn, close_okuri_window_cb, LV_EVENT_CLICKED, NULL);
  //lv_obj_align(btn, LV_ALIGN_CENTER, 0, -40);
  label = lv_label_create(btn);
  lv_label_set_text(label, "Close");
  lv_obj_center(label);
}

/**
 * Function: settings_event_cb - Callback function used by the
 * Settings LVGL dropdown widget.  Current operations supported are: 
 *   - Clearing the Kana textarea widget and the Kana entry queue.
 *   - Selection of Unicode macro mode and saving to non-volatile storage.
 *   - Displaying the LVGL messagebox to perform touchscreen calibration.
 * Input: Pointer to LVGL event structure.
 * Output: None.
 */
static void settings_event_cb(lv_event_t * e) {
  lv_obj_t * dropdown = lv_event_get_target(e);
  uint16_t ndx = lv_dropdown_get_selected(dropdown);

  switch (ndx) {
    case 0:
      // Clear the rightmost character in the text area.
      lv_textarea_del_char(kanji_text);
      // Reduce input queue by one character and rebuild Kanji list.
      decrement_input_queue();
      break;
    case 1:
      //Serial.println("Clear input text and readings");
      lv_textarea_set_text(kanji_text, "");
      clear_input_queue();
      clear_kanji_list();
      listStart = 0;
      break;
    case 2:
      // Open window to display list of radiobuttons to select Macro output mode.
      create_macro_select_window();
      break;
    case 3:
      // Create calibration window.
      create_touch_calibration_window();
      break;
  }
}

/**
 * Function: make_interface - Create the display using LVGL widgets.
 * Input: None.
 * Output: None.
 */
void make_interface(void) {
  lv_obj_t *btn;

  // Create a drop down list for Settings.
  settings = lv_dropdown_create(lv_scr_act());
  lv_obj_set_size(settings, 32, 40);
  lv_obj_set_pos(settings, 0, 0);
  // The items in the menu are set up according to the Unicode style in use.
  lv_dropdown_set_options(settings,
                          "Delete last character\n"
                          "Clear input and Kanji list\n"
                          "Select Macro output mode...\n"
                          "Calibrate Screen..."
                          );
  // Set a fixed text to display on the button of the drop-down list.
  lv_dropdown_set_text(settings, "");
  // In a menu, we don't need to show the last clicked item.
  lv_dropdown_set_selected_highlight(settings, false);
  lv_obj_add_event_cb(settings, settings_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Create textarea for Kana/Kanji text box.
  kanji_text = lv_textarea_create(lv_scr_act());
  lv_textarea_set_one_line(kanji_text, true);          // One line text area.
  lv_textarea_set_cursor_click_pos(kanji_text, false); // Disable manual cursor positioning.
  lv_obj_clear_flag(kanji_text, LV_OBJ_FLAG_SCROLLABLE);
  lv_textarea_set_max_length(kanji_text, 9);           // No more than nine characters.
  lv_obj_set_pos(kanji_text, 32, 0);
  lv_obj_set_size(kanji_text, 288, 40);
  lv_obj_set_style_text_font(kanji_text, &KanaKanjiFont32, 0);

  // Create a list that is not populated with buttons.
  kanji_list = lv_list_create(lv_scr_act());
  lv_obj_set_size(kanji_list, 320, 200);
  lv_obj_set_pos(kanji_list, 0, 40);
  lv_obj_set_style_pad_hor(kanji_list, 0, 0);
  btn = make_list_button("\xE2\xAC\x86", "Click for more frequenly used Kanji", wht_color);
  lv_obj_add_event_cb(btn, kanji_nav_up_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
  btn = make_list_button("\xE2\xAC\x87", "Click for less frequenly used Kanji", wht_color);
  lv_obj_add_event_cb(btn, kanji_nav_dn_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
}

/**
 * Function: display_handler - Call the LVGL handler from code outside
 * the display code section.
 * Input: None.
 * Output: None.
 */
void display_handler(void) {
  lv_timer_handler(); /* let the GUI do its work */
  delay( 5 );
}
