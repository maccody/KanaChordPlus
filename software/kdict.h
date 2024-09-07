/* kdict.h */
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

#ifndef KANJI_METADATA_TYPE
#define KANJI_METADATA_TYPE
typedef struct kanji_md {
  const uint16_t unicode;
  const uint16_t rank;
  const char* const meaning;
} kanji_md;
#endif

#ifndef AFFIX_ENUM
#define AFFIX_ENUM
enum affix_enum {
  none,   // No enhancments to reading (e.g., xyz).
  prefix, // Reading is a prefix (e.g., wxyz-).
  suffix, // Reading is a suffix (e.g., -wxyz).
  jword,  // Japanese word string with jukujikun or irregular reading.
  meaning // English meaning string for Japanese word enumerated above.
};
#endif

#ifndef OKURI_METADATA_TYPE
#define OKURI_METADATA_TYPE
typedef struct okuri_md {
  const uint8_t len;
  const affix_enum * const alist;
  const char **clist;
} okuri_md;
#endif

#ifndef READING_METADATA_TYPE
#define READING_METADATA_TYPE
typedef struct reading_md {
  const uint16_t len;
  const okuri_md * const * olist;
  const kanji_md * const * klist;
} reading_md;
#endif

#ifndef BBT_NODE_TYPE
#define BBT_NODE_TYPE
typedef struct bbt_node {
  const uint32_t key;
  const reading_md* rmd;
  const bbt_node * const lnode;
  const bbt_node * const rnode;
} bbt_node;
#endif

#ifndef READING_TYPE_ENUM
#define READING_TYPE_ENUM
  // enum readingType { NONE, KUNYOMI, ONYOMI, NANORI, JUKUJIKUN, IRREGULAR };
  enum readingType { NONE, KUNYOMI, ONYOMI, NANORI, DICTIONARY };
#endif

typedef struct kanji_list_data {
  readingType type;
  okuri_md *omd;
  kanji_md *kmd;
} kanji_list_data;

extern kanji_list_data klist_data[500];
extern uint16_t totalListSize;

void build_kanji_list_data(uint16_t *kanaList, uint8_t kanaCount);
