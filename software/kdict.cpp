/* kdict.cpp */
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

#include <cstddef>
#include <cstdint>
#include <Arduino.h>
#include <Print.h>
#include "kdict.h"
#include "kanji_ms.h"
#include "kanji_md.h"
#include "onyomi.h"
#include "kunyomi.h"
#include "nanori.h"
#include "dictionary.h"

kanji_list_data klist_data[500];
uint16_t totalListSize;

/**
 * Function: get_reading - Recursively traverse through a Binary Structured Tree
 * (BST) using the provided Kana key to obtain the matching reading, if there is one.
 * Input:
 *   key - Kana key encoded as an unsigned 64-bit number.
 *   *parent node - Parent node of the BST.
 * Output:
 *   Pointer to Kanji reading metadata or null pointer if a reading is not found.
 */
reading_md* get_reading(uint32_t key, bbt_node *parent_node) {
  reading_md* metadata = NULL;

  // Compare supplied key to key of node.
  if (key == parent_node->key) {
    // The key matches for this node!
    // Match obtained, so return onyomi metadata of node.
    metadata = (reading_md *) parent_node->rmd;
  } else if (key < parent_node->key) {
    // The value is lower than the key.  Accessing left child node.
    // Determine whether pointer to left node is a null pointer.
    if (parent_node->lnode != NULL) {
      // Recurse down left node of BST, returning metadata or null pointer.
      metadata = get_reading(key, (bbt_node *) parent_node->lnode);
    }
  } else {
    // The value is higher than the key.  Accessing right child node.
    // Determine whether pointer to right node is a null pointer.
    if (parent_node->rnode != NULL) {
      // Recurse down right node of BST, returning metadata or null pointer.
      metadata = get_reading(key, (bbt_node *) parent_node->rnode);
    }
  }

  // Return either a metadata pointer or a null pointer.
  return (metadata);
}


/**
 * Function: murmer_32_scramble
 * Code taken from Wikipedia webpage MurmurHash (https://en.wikipedia.org/wiki/MurmurHash)
 */
static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

/**
 * Function: murmer_32
 * Code taken from Wikipedia webpage MurmurHash (https://en.wikipedia.org/wiki/MurmurHash)
 * slightly modified to use uint8_t for the length specification and loop counter.
 */
uint32_t murmur3_32(const uint8_t* key, uint8_t len, uint32_t seed)
{
  uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (uint8_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

/**
 * Function: make_key - Generate a unsigned 64-bit Kana key based on the list
 * of Kana Unicode values provded.  A flag determines whether the key is for
 * kunyomi/nanori or onyomi.
 * Input:
 *   *kanaList - Pointer to list of Kana Unicode values.
 *   kanaCount - Number of Kana Unicode values in the list.
 *   onyomiKey - false - kunyomi/nanori key, true - onyomi key.
 */
uint32_t make_key(uint16_t *kanaList, uint8_t kanaCount, bool onyomiKey) {
  uint8_t key[20]; // There should be no more than nine characters, ever.
  uint16_t lowHex;
  uint32_t keyHash;
  
  for (int i = 0; i < kanaCount; i++) {
    lowHex = kanaList[i] & 0x00ff; // Only need lowest byte of Unicode.
    if (onyomiKey) {
      // Modify Hiragana characters only.
      if (lowHex < 0x00a0) {
        lowHex += 0x0060;
      }
    }
    key[i] = (uint8_t) lowHex; // Only need lowest byte.
  }

  keyHash = murmur3_32(key, kanaCount, 0);
  //Serial.printf("Key hash: 0x%08x\n", keyHash);
  return(keyHash);
}

/**
 * Function: getKunyomiReading - Search the Kunyomi BST using the provided
 * Kana Unicode list.
 * Input:
 *   *kanaList - Pointer to list of Kana Unicode values.
 *   kanaCount - Number of Kana Unicode values in the list.
 * Output:
 *   Pointer to Kanji reading metadata or null pointer if a reading is not found.
 */
reading_md* getKunyomiReading(uint16_t *kanaList, uint8_t kanaCount) {
  // Convert Kana to key appropriate for Kunyomi reading.
  uint32_t key = make_key(kanaList, kanaCount, false);
  // Obtain reading and return result.
  return(get_reading(key, (bbt_node *) kunyomi_root_node));
}

/**
 * Function: getOnyomiReading - Search the Onyomi BST using the provided
 * Kana Unicode list.
 * Input:
 *   *kanaList - Pointer to list of Kana Unicode values.
 *   kanaCount - Number of Kana Unicode values in the list.
 * Output:
 *   Pointer to Kanji reading metadata or null pointer if a reading is not found.
 */
reading_md* getOnyomiReading(uint16_t *kanaList, uint8_t kanaCount) {
  // Convert Kana to key appropriate for Onyomi reading.
  uint32_t key = make_key(kanaList, kanaCount, true);
  // Obtain reading and return result.
  return(get_reading(key, (bbt_node *) onyomi_root_node));
}

/**
 * Function: getNanoriReading - Search the Nanori BST using the provided
 * Kana Unicode list.
 * Input:
 *   *kanaList - Pointer to list of Kana Unicode values.
 *   kanaCount - Number of Kana Unicode values in the list.
 * Output:
 *   Pointer to Kanji reading metadata or null pointer if a reading is not found.
 */
reading_md* getNanoriReading(uint16_t *kanaList, uint8_t kanaCount) {
  // Convert Kana to key appropriate for Nanori reading.
  uint32_t key = make_key(kanaList, kanaCount, false);
  // Obtain reading and return result.
  return(get_reading(key, (bbt_node *) nanori_root_node));
}

/**
 * Function: getDictionaryReading - Search the Dictionary BST using the provided
 * Kana Unicode list.
 * Input:
 *   *kanaList - Pointer to list of Kana Unicode values.
 *   kanaCount - Number of Kana Unicode values in the list.
 * Output:
 *   Pointer to Kanji reading metadata or null pointer if a reading is not found.
 */
reading_md* getDictionaryReading(uint16_t *kanaList, uint8_t kanaCount) {
  // Convert Kana to key appropriate for Nanori reading.
  uint32_t key = make_key(kanaList, kanaCount, false);
  // Obtain reading and return result.
  return(get_reading(key, (bbt_node *) dictionary_root_node));
}

/**
 * Function: build_kanji_list_data - Build list of reading metadata based on
 * the list of Kana Unicode provdied.  Readings for kunyomi, onyomi, and
 * nanori are collected and sorted by frequency ranking.  The list contained 
 * in the globalarray klist_data, with the length of the list stored in 
 * totalListSize.
 * Input:
 *   *kanaList - Pointer to list of Kana Unicode values.
 *   kanaCount - Number of Kana Unicode values in the list.
 * Output: None.
 */
void build_kanji_list_data(uint16_t *kanaList, uint8_t kanaCount) {
  uint16_t lowestRank, buttonCount = 0;
  readingType lowestReading;
  
  // Obtain the Kunyomi, Onyomi, and Nanori readings, if any.
  reading_md *kunyomi_md = getKunyomiReading(kanaList, kanaCount);
  reading_md *onyomi_md = getOnyomiReading(kanaList, kanaCount);
  reading_md *nanori_md = getNanoriReading(kanaList, kanaCount);
  reading_md *dictionary_md = getDictionaryReading(kanaList, kanaCount);

  // Determine the total number of list buttons required.
  totalListSize = 0;
  if (kunyomi_md != NULL) {
    totalListSize += kunyomi_md->len;
  }
  
  if (onyomi_md != NULL) {
    totalListSize += onyomi_md->len;
  }
  
  if (nanori_md != NULL) {
    totalListSize += nanori_md->len;
  }

  if (dictionary_md != NULL) {
    totalListSize += dictionary_md->len;
  }

  if (totalListSize != 0) {
    // Ordering...
    uint16_t kunIdx = 0;
    uint16_t onIdx = 0;
    uint16_t nanIdx = 0;
    uint16_t dictIdx = 0;
    uint16_t listIdx = 0;
  
    // Iterate through the chosen Kunyomi, Onyomi, and Nanori readings
    // generating a list ordered according to frequency rank, lowest first.
    do {
      // Determine which of three reading types has the lowest frequency rank.
      // Initialize to no reading chosen;
      lowestReading = NONE;
      // Initialize to a very large rank.
      lowestRank = 60000;
      // Test kunyomi if not a null pointer.
      // NOTE: Dictionary readings have top priority in the list.
      if (dictionary_md != NULL) {
        if (dictIdx < dictionary_md->len) {
          // Dictionary readings are forced to be the first entries in the list.
          lowestRank = 0;
          lowestReading = DICTIONARY;
        }
      }

      if (kunyomi_md != NULL) {
        // Test kunyomi if there are still readings left.
        if (kunIdx < kunyomi_md->len) {
          // Compare Kanji ranking to current lowest rank.
          if (kunyomi_md->klist[kunIdx]->rank < lowestRank) {
            lowestRank = kunyomi_md->klist[kunIdx]->rank;
            lowestReading = KUNYOMI;
            // Replaced by kunyomi of lower rank.
          }
        }
      }

      // Test onyomi if not a null pointer.
      if (onyomi_md != NULL) {
        // Test onyomi if there are still readings left.
        if (onIdx < onyomi_md->len) {
          // Compare Kanji ranking to current lowest rank.
          if (onyomi_md->klist[onIdx]->rank < lowestRank) {
            lowestRank = onyomi_md->klist[onIdx]->rank;
            lowestReading = ONYOMI;
            // Replaced by onyomi of lower rank.
          }
        }
      }

      // Test nanori if not a null pointer.
      if (nanori_md != NULL) {
        // Test nanori if there are still readings left.
        if (nanIdx < nanori_md->len) {
          // Compare Kanji ranking to current lowest rank.
          if (nanori_md->klist[nanIdx]->rank < lowestRank) {
            lowestRank = nanori_md->klist[nanIdx]->rank;
            lowestReading = NANORI;
            // Replaced by nanori of lower rank.
          }
        }
      }

      // At this point, the lowest reading will cause a button to be added to the
      // list widget with the appropriate Kanji, meaning string, and color.  If
      // no reading was chosen, the all cases will be skipped.
      switch (lowestReading) {
        case KUNYOMI:
          // Kunyomi list entry created.
          klist_data[listIdx].type = KUNYOMI;
          klist_data[listIdx].kmd = (kanji_md *) kunyomi_md->klist[kunIdx];
          if (kunyomi_md->olist != NULL) {
            klist_data[listIdx].omd = (okuri_md *) kunyomi_md->olist[kunIdx];
          } else {
            klist_data[listIdx].omd = NULL;
          }

          kunIdx++;
          listIdx++;
          break;
        case ONYOMI:
          // Onyomi list entry created.
          klist_data[listIdx].type = ONYOMI;
          klist_data[listIdx].kmd = (kanji_md *) onyomi_md->klist[onIdx];
          if (onyomi_md->olist != NULL) {
            klist_data[listIdx].omd = (okuri_md *) onyomi_md->olist[onIdx];
          } else {
            klist_data[listIdx].omd = NULL;
          }

          onIdx++;
          listIdx++;
          break;
        case NANORI:
          // Nanori list entry created.
          klist_data[listIdx].type = NANORI;
          klist_data[listIdx].kmd = (kanji_md *) nanori_md->klist[nanIdx];
          if (nanori_md->olist != NULL) {
            klist_data[listIdx].omd = (okuri_md *) nanori_md->olist[nanIdx];
          } else {
            klist_data[listIdx].omd = NULL;
          }
  
          nanIdx++;
          listIdx++;
          break;
        case DICTIONARY:
          // Dictionary list entry created.
          klist_data[listIdx].type = DICTIONARY;
          klist_data[listIdx].kmd = NULL;
          klist_data[listIdx].omd = (okuri_md *) dictionary_md->olist[dictIdx];
          dictIdx++;
          listIdx++;
          break;
      }

    // The loop will exit only if all metadata pointers are null or all readings
    // have been processed into buttons in the list widget.
    } while (lowestReading != NONE);
  }
}
