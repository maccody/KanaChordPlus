/* display.h */
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

extern uint16_t listStart;
extern uint8_t unicodeMode;

void display_setup(void);
void make_interface(void);
void clear_kanji_list(void);
void build_kanji_list(void);
void set_kana_text(uint16_t *queueData, uint16_t queueLen);
void insert_kana_char(uint16_t ucsData);
void clear_text(void);
void display_handler(void);
