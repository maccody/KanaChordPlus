# KanaChord Plus dictionaries
Providing Kanji support in KanaChord Plus was a challenging task, due to the nature of the Japanese language!  Japanese is a phonetically limited language, with at least twelve (perhaps up to 21) distinct consonants and five distinct vowels.  The addition of mora (timing of pronunciation) and pitch accent add to the challenge.  The Kanji themselves are subject to interpretation, as the pronunciation of any particular Kanji can have Chinese readings (onyomi), Japanese readings (kunyomi), or readings assigned for name usage (nanori).  As a consequence, there can be many Kanji and Japanese words that are homophones (sound the same), e.g., 花 (hana) flower and 鼻 (hana) nose, where pitch accent differentiates them otherwise.  This Wikipedia web page on [Japanese phonology](https://en.wikipedia.org/wiki/Japanese_phonology) provides an introduction for a deeper dive into the topic.  

An additional challenge is providing a sufficiently diverse set of Kanji to support Japanese typing for most users.  There a currently over [fifty-thousand Kanji characters](https://en.wikipedia.org/wiki/Japanese_writing_system), although many are archaic and not in formal usage today.  Still, there are many thousands of Kanji that are in use and choosing which ones to support would be a daunting task.  Fortunately, as with other languages, some Kanji are used much more often than others, which provides a starting point for which Kanji to support.  The Japanese government mandates that all Japanese high school graduates learn the [常用漢字 (jōyō kanji)](https://en.wikipedia.org/wiki/J%C5%8Dy%C5%8D_kanji), or 'list of regular use Kanji' literally. These 2,136 Kanji provide a minimum standard for all Japanese to read and write.  Mulitple schollars have performed frequency analysis of Kanji to create more extensive lists of frequently-used Kanji.  The [Novel 5k](https://docs.google.com/spreadsheets/d/1l2MNM5OWznIRVm98bTCA1qPNAFnM48xJIyUPtchxyb0/edit?usp=sharing) spreadsheet and [Kanji Dict 2](http://www.edrdg.org/wiki/index.php/KANJIDIC_Project) XML document are two of these lists.  They were used to select 6,165 of the most common Kanji, with their associated meanings in English. This is the subset of Kanji that is currently supported by KanaChord Plus.  This Kanji subset should be suffcient for most users, possibly even at the collegiate level!  Note that the number of Kanji supported also drives the size of the Kanji font character set stored on the Pico.

Cross-referencing each reading to all associated Kanji requires 'inverting' the content of the Kanji Dict 2 XML document.  Kanji Dict 2 presents a structed list of Kanji, each with their relevant onyomi and kunyomi readings (in Katakana and Hiragana, respectively), nanori pronunciations (in hiragana), and English meanings.  In addition, many kunyomi pronunciations have [okurigana](https://en.wikipedia.org/wiki/Okurigana) (accompanying hiragana after the Kanji) used to inflect adjectives and form verbs.  Sequentially processing each Kanji entry and tabulating it into each reading yields a table of readings with all associated Kanji and their English meanings.  There are three dictionaries generated, one each for onyomi, kunyomi, and nanori.  Note that the okurigana are included in the kunyomi dictionary to allow for direct selection rather than typing each Hiragana manually.

Beyond the Kanji are Japanese words in general, which can be composed of Hiragana, Katakana, Kanji, or a combination of all three.  While it is possible to always type Kana characters, sometimes to generate individual Kanji, to then create individual Japanese words, this can become tedious, especially when a lot of the same words are typed frequently.  Consequently, a forth dictionary is a collection of commonly used Japanese words.  Coincidentally, a number of common Japanese words are [Jukujikun](https://en.wiktionary.org/wiki/jukujikun), or 'compound character reading', which use Kanji characters to convey the meaning but the pronunciation is of Japanese origin.  An example is 今日 (kyō) or 'today', which would have a literal pronunciation of 'ima hi' or 'now day', based upon the Kanji used.

Finding extensive lists (more than a few thousand) of Japanese words containing frequency ranking, reading, and English meaning proved a bit difficult.  Ultimately three Core word spreadsheets (used for Anki decks), a list of official and unofficial Jukujikun words, and a frequency-ordered list of over 44,000 Japanese words were used.  Not surprising, but many of the most-common Japanese words consist of Kana only and are not included in the dictionary.  Of the remaining most-common words containing Kanji, 6,240 are in the current word dictionary, limited due to memory constraints (discussed below).

## Data structures forming the dictionaries
Several approaches were considered to store the dictionaries on KanaChord Plus:
1. External flash card accessed through the Pico's SPI interface, using a flash card reader library - plenty of storage space, relatively slower access, and driver overhead.
2. Use part of the Pico's flash ROM as a flash drive, using a flash card reader library - Limited space (less than two megabytes), about four times faster access with QSPI compared to SPI, but same driver overhead.
3. Create data structures representing the dictionaries that are stored in the flash ROM's program memory and accessed directly - Limited storage space, though more efficient, and fastest access possible, with no drive overhead.

The third option was chosen for this implementation of KanaChord Plus.  Speed in retrieving Kanji and English meaning data for the Kana currently typed will impact the responsiveness of the keyboard and the overall user experience.  It is anticipated that a future version of KanaChord Plus would use a board-compatible, RP2040-based microcontroller with 4MB, 8MB, or 16MB of flash ROM.  The recent (August 2024) introduction of the Raspberry Pi Pico 2 (RP2340-based microcontroller), with 4MB of flash ROM would be a suitable replacement. The faster microcontroller could maintain or improve responsiveness, while providing extra flash ROM stoarge for either more Kanji, more Japanese words, or both!  

Several data structures were created to store the content of the four dictionaries and support rapid dictionary search based on the kana characters currently entered.  The illustration below shows the relationship of these data structures.

![Data structure relationship](./images/data_structure_relationship.gif)

A [Balanced](https://en.wikipedia.org/wiki/Self-balancing_binary_search_tree) [Binary Search Tree (BST)](https://en.wikipedia.org/wiki/Binary_search_tree) is employed to quickly search each dictionary.  The sorted keys are hashes of Kana character sequences.  The hash algorithm used is [32-bit Murmur Hash version 3](https://en.wikipedia.org/wiki/MurmurHash) - the implementation used is found on the referenced Wikipedia page.  Each node of the BST is a data structure (bbt_node) that contains the following elements:   
- An unsigned 32-bit key that is a Murmur hash of a reading (Kana character sequence).
- A pointer to a reading matadata structure  (reading_md* rmd) for the reading used for the key.
- Pointers to two child BST structures (bbt_node* lnode, bbt_node* rnode), which can be NULL if there is no child for a branch of the tree.

During the search down the BST of a dictionary, the submitted reading hash is compared to the structure's hash key.  If the submitted hash is either less than or greater than the hash key, the search continues to the structure referenced by the appropriate pointer.  If that pointer is null, then there is no match for the submitted reading hash.  If the submitted hash is equal to the key in the current BST structure, then the pointer to the associated reading metadata structure is returned.  

The reading metadata structure (reading_md) provides information regarding the Kanji and Japanese words associated with a reading.  The reading structure contains the following elements:
- An unsigned 16-bit value (len) indicating the number of Kanji and Okurigana or Japanese words associated with the reading (Kana characer sequence).
- A pointer to an array of pointers to Okurigana or Japanese word/meaning metadata structures (okuri_md** olist).  For the onyomi, kunyomi, and nanori dictionaries, this pointer will be set to the NULL pointer if there are no okurigana for any of the Kanji referenced.
- A pointer to an array of pointers to Kanji metadata structures (kanji_md ** klist).  For the Japanese word dictionary, this pointer will be set to the NULL value.  

Each reading metadata structure pointer returned from a directory search is accessed to build up a list of candidate Kanji and Japanese words that the used can choose from to replace the Kana character sequence.

The Okurigana/Japanese word metadata structure (okuri_md) has alternate uses depending upon which dictionary it is being used.  For the onyomi, kunyomi, and nanori dictionaries, this metadata structure references any Okurigana associated with a Kanji.  There can be multiple Okurigana that are referenced.  For the Japanese word dictionary, this metadata structure references the Japanese word (combination of Kana and Kanji characters, encoded in UTF-8) and the meaning of the word in English.  There will be only one Japanese word and meaning referenced.

The Okurigana / Japanese word metadata structure contains the following elements:
- An unsigned 8-bit value (len) indicating the number of Affix enumerations and Okurigana strings or Japanese word/meaning strings.
- Pointer to array of Affix enumerations (affix_enum *alist).  For the onyomi, kunyomi, and nanori dictionaries, the relevant enumerations are: none (simple reading without context) , prefix (prefix reading, placed before other Kanji), and suffix (suffix reading, placed after other Kanji).  For the Japanese word dictionary, the relevant enumerations are: jword (Japanese word in correconding index of olist) and meaning (meaning string in corresponding index of olist).  The content of alist is always one 'jword', followed by one 'meaning'.
- Pointer to array of Okurigana or Japanese words/meanings (char **clist).  All characters are UTF-8 encoded.

The Kanji metadata structure (kanji_md) refrences information about a Kanji associated with a reading.  The Kanji metadata structure contains the following elements:
- Unsigned 16-bit Unicode value (unicode) for the Kanji.
- Unsigned 16-bit value indicating rank order (rank) for the Kanji - lower value is more common.
- Character pointer to string containing English meaning for the Kanji (char *meaning).

The rank value is used when generating a list of Kanji and Japanese word for the reading (Kana characters) provided.  The list is ordered such that Japanese word are listed first, followed by Kanji, with the more common listed first.

## Programmatic dictionary file generation

**NOTE: This section is only useful if one wants to understand how the dictionary files are generated.  These files have already been generated and placed in their appropriate directories.  If you want to make changes to the dictionary files, then the Python script will need to be run and the files copied to the directory containing the other source code.**

The very large number of readings and their associated Kanji characters and Japanese words made it a big challenge to create all of the data structures for the four dictionaries.  It was decided to generate the dictionaries programmatically using a Python script.  This ensured consistency of data structure content and quick regeneration, when needed, to change or correct content.

The Python script, kana_kanji_dictionary.py has over 1400 lines of code and comments (fortunately!).  It was developed using Python 3.8.0.  The following Python libraries required by the script:
- XML (xml) - Extensible Markup Language library, particularly ElementTree support.
- Pandas (pandas) - Dataframe manipulation library.
- Itertools (itertools) - Advanced iteration function library.
- Murmur Hash version 3 (mmh3) - Murmur hash generation library.

The following files are read by the Python script during processing:
- Novel 5K most common Kanji (Novel_5K.csv). Source: [Novel 5k](https://docs.google.com/spreadsheets/d/1l2MNM5OWznIRVm98bTCA1qPNAFnM48xJIyUPtchxyb0/edit?usp=sharing)
- Kanji Dict 2 (kanjidic2.xml) Source: [The KANJIDIC Project](http://www.edrdg.org/wiki/index.php/KANJIDIC_Project)
- List of Kana characters and their corresponding least-significant byte of Unicode values (kana_list.csv).
- Core 10K list of over ten thousand common Japanese words with pronunciations and meanings (Core10k.csv). Source: [Core 10000](https://core6000.neocities.org/10k/)
- Core 5K Frequency list of over five thousand of the most common Japanese words, with pronunciations and meanings (Core5kFrequencyMod3.csv).  Modified to place single entries on each line.  Source: [Core 5000 Frequency](https://core6000.neocities.org/freq/)
- Core 6k list of over six thousand common Japanese words with pronunciations and meanings (Core6kMod.csv).  Modified to place single entries on each line. Source: [Core 6000](https://core6000.neocities.org/)
- List of official and unofficial Jukujikun, or Japanese words with pronunciations that do not match the Kanji representing those words (jukujikun_mod.txt).  Modified to combine official and unofficial lists. Source: [Kanjium - the ultimate kanji resource](https://github.com/mifunetoshiro/kanjium)
- 44492 Japanese Word frequency list (44492-japanese-words-latin-lines-removed.txt).  Source: [hingston/Japanese Github repo](https://github.com/hingston/japanese/blob/master/44492-japanese-words-latin-lines-removed.txt)

The following files are programatically generated by the Python script:
- kana_kani_subset.txt - List of Unicode hexidecimal values to submit to [LVGL font converter](https://lvgl.io/tools/fontconverter).
- kanji_ms.h - Contains character strings for meanings of Kanji meaning in English.
- kanji_md.h - Data structures containing Kanji Unicode values, rank of commonality, and references to English meaning string.
- onyomi.h - Data structures forming the dictionary of onyomi (Chinese) readings for Kanji.
- kunyomi.h - Data structures forming the dictionary of kunyomi (Japanese) readings for Kanji.
- nanori.h - Data structures forming the dictionary of nanori (name) readings for Kanji.
- dictionary.h - Data structures forming the dictionary of common Japanese words containing Kanji.

A fairly high-level flow of the Python script is as follows:
- Import supporting libraries (lines 1 - 4) and define helper functions:
  - Perform iteration on lambda function to combine contiguous 16-bit Unicode values into ranges (lines 6 - 9).
  - Convert a byte value into bits (lines 11 - 16).
  - Reverse the bit order of a value (lines 18 - 24).
  - print_balanced_bst - Recursively print out a balanced BST to an opened file (lines 26 - 68).
- Define a list of common Kanji found in some Kanji lists (lines 70 - 83).
- Load in the Novel 5K and KANJIDICT 2 dictionaries (lines 85 - 94).
- Initialize the first data frame (df1) to hold Kanji data (line 98).
- Iterate on the KANJIDICT 2 'character' element and perform the following (line 100):
  - Extract from the character element the Kanji Unicode, Kanji character, and frequency ranking, assigning a frequency rank if one is not present (lines 101 - 113).
  - Search for the Kanji character in the Novel 5K dictionary, extracting its rank if present, or assigning one if that Kanji is not present (lines 115 - 133).
  - Construct lists of meanings, onyomi, kunyomi, and nanori associated with the Kanji character (lines 135 - 159).
  - Further processing is skipped for this Kanji if there are no readings found (lines 161 - 164).
  - Add a new row to dataframe df1 using the information collected for the Kanji (lines 166 - 169).
- Create a subset of dataframe df1 with only those rows with Novel 5K rank less than 10000.  Create two sort on the subset: one on the Nover 5K rank in ascending order, the other on the Unicode value (line 174 - 187).
- Create a list of Kanji Unicode values, with contiguous Unicodes combined into ranges.  Then write some special Unicode characters, the Kana Unicode character ranges, and the Kanji list out to the text file, kana_kanji_subset.txt (lines 188 - 212).
- Write all of the Kanji meanings as C character strings into a C header file, kanji_ms.h.  The naming of each string conincides with the associated Kanji's Unicode number.  An ordered list of Kanji Unicodes and their Novel 5K ranks is also created (lines 214 - 229).
- Write the collected Kanji data to a C header file, kanji_md.h.  Use the ordered list to to assure that the Kanji metadata structures are named according to the associated Kanji's Unicode (lines 231 - 263).
- Create dataframes for the Onyomi, Kunyomi, and Nanori cross-references (line 265 - 272).
- Interating on each row of dataframe df1 perform the following (line 275):
  - For each Onyomi in the row (line 277):
    - Determine whether it is a suffix or not (lines 278 - 283).
    - Determine whether the Onyomi is already in a row of the Onyomi data frame, appeding the new data if it is, or adding a new row if it is not present (lines 289 - 305).
  - For each Kunyomi in the row (line 308):
    - Determine whether it is a prefix, suffix, or neither (lines 309 - 316).
    - Isolate Okurigana if present (lines 318 - 325).
    - Determine whether the Kunyomi is already in a row of the Kunyomi data frame, appeding the new data if it is, or adding a new row if it is not present (lines 327 - 371).
  - For each Nanori in the row (line 374):
    - Isolate Okurigana if present (lines 377 - 383).
    - Determine whether the Nanori is already in a row of the Nanori data frame, appeding the new data if it is, or adding a new row if it is not present (lines 385 - 402).
- For each row in the Onyomi dataframe (df2) (line 405):
  - Sort the Kanji for each reading by its frequency list in ascending order (lines 406 - 409).
  - Then sort the dataframe by the reading value in ascending order (line 412).
- For each row in the Kunyomi dataframe (df3) (line 415);
  - Sort the Kanji and associated Okurigana according to the frequency list in ascending order (line 416 - 431).
  - Then sort the dataframe by the reading value in ascending order (line 434).
- Sort the Nanori dataframe (df4) by the reading value in ascending order (line 445).
- Load in a list of mappings from Kana to least-significat byte of Unicode value and converting a Python dictionary (line 447 - 451).
- Output the Onyomi data structures to the file onyomi.h, first adding two new columns to the dataframe (lines 453 - 458).
  - Writing the structure typedefs to the file (lines 494 - 522)
  - Iterating on each row of sorted dataframe df2Sort (line 525):
    - Generate a Murmur3 hash key for the Kana reading (lines 526 - 536).
    - Add the Kana representation as an integer value and a byte string to the row for generating the Balanced BST later (lines 537 - 539).
    - Write out the Affix list, as a list of enumerations, and Okurigana metadata list as list of strings.  If all of the Affix values are 'none', this step is skipped (lines 541 - 562).
    - Write out the list of Kanji metadata structure references (lines 564 - 569).
    - Write out the Onyomi reading metadata structure referencing the list of Okurigana pointers, or NULL if none, and the list of Kanji metadata pointers (lines 571 - 576).
  - Write out the typedef for the Balanced BST to the file (lines 578 - 593).
  - Pull out the columns from the sorted dataframe df2Sort that are the Kana readings in integer form and byte string form and sort them according to the integer form in ascending order (lines 595 - 599).
  - Pull out the sorted Kana reading in byte string form and print out the Balance BST structures using the helper function print_balanced_bst (lines 597 - 603).
- Output the Kunyomi data structures to the file kunyomi.h, first adding two new columns to the dataframe (lines 609 - 614).
  - Writing the structure typedefs to the file (lines 648 - 676)
  - Iterating on each row of sorted dataframe df3Sort (line 679):
    - Generate a Murmur3 hash key for the Kana reading (lines 680 - 690).
    - Add the Kana representation as an integer value and a byte string to the row for generating the Balanced BST later (lines 691 - 693).
    - Write out the Affix list, as a list of enumerations, and Okurigana metadata list as list of strings.  If all of the Affix values are 'none', this step is skipped (lines 695 - 785).
    - Write out the list of Kanji metadata structure references (lines 787 - 793).
    - Write out the Kunyomi reading metadata structure referencing the list of Okurigana pointers, or NULL if none, and the list of Kanji metadata pointers (lines 796 - 804).
  - Write out the typedef for the Balanced BST to the file (lines 806 - 821).
  - Pull out the columns from the sorted dataframe df3Sort that are the Kana readings in integer form and byte string form and sort them according to the integer form in ascending order (lines 823 - 827).
  - Pull out the sorted Kana reading in byte string form and print out the Balance BST structures using the helper function print_balanced_bst (lines 830 - 833).
- Output the Nanori data structures to the file nanori.h, first adding two new columns to the dataframe (lines 838 - 843).
  - Writing the structure typedefs to the file (lines 877 - 905)
  - Iterating on each row of sorted dataframe df4Sort (line 908):
    - Generate a Murmur3 hash key for the Kana reading (lines 909 - 919).
    - Add the Kana representation as an integer value and a byte string to the row for generating the Balanced BST later (lines 920 - 922).
    - Write out the Okurigana metadata list as list of strings.  If there are no Okurigana data, this step is skipped (lines 924 - 987).
    - Write out the list of Kanji metadata structure references (lines 989 - 994).
    - Write out the Nanori reading metadata structure referencing the list of Okurigana pointers, or NULL if none, and the list of Kanji metadata pointers (lines 996 - 1001).
  - Write out the typedef for the Balanced BST to the file (lines 1003 - 1018).
  - Pull out the columns from the sorted dataframe df4Sort that are the Kana readings in integer form and byte string form and sort them according to the integer form in ascending order (lines 1020 - 1024).
  - Pull out the sorted Kana reading in byte string form and print out the Balance BST structures using the helper function print_balanced_bst (lines 1025 - 1030).
- Read in the Core 10K spreadsheet as a dataframe, dfC10K, and create a subset dataframe with just the Reading, Kanji, and Definition columns, dfPart1.  Add columns for rank and source, setting all to -1 and 'Core10K', respectively (lines 1038 - 1046).
- Read in the modified Core 5K Frequency spreadsheet as a dataframe, dfC5K (lines 1050 - 1052).
- Iterate on each row of the dataframe dfC5K, determining whether each word has Kanji that are in the subset of 6100+ most common Kanji (lines 1055 - 1064).
- Iterate on each row for the dataframe dfC5K, determining whether it is already in the dfC10K dataframe.  If it is, update the corresponding row of dataframe dfPart1 with the dfC5K rank value, which is the row number.  If is not matched, add a new row to dataframe dfPart1 with the new Kanji data and rank, with the source being 'Core5K' (lines 1066 - 1089).
- Copy the updated dataframe dfPart1 to a new dataframe dfPart2 (line 1092).
- Read in the modified Core 6K spreadsheet as dataframe, dfC6K (line 1095).
- Iterate on each row of the dataframe dfC6K, determining whether each word has Kanji that are in the subset of 6100+ most common Kanji (lines 1097 - 1106).
- Iterate on each row for the dataframe dfC6K, determining whether it is already in the dfPart1 dataframe.  If it is, do nothing, as there is no rank information available.  If is not matched, add a new row to dataframe dfPart2 with the new Kanji data and rank equal to -1, with the source being 'Core6K' (lines 1109 - 1126).
- Copy the updated dataframe dfPart2 to a new dataframe dfPart3 (line 1092).
- Read in the modified Jukujikun readings file as dataframe, dfJuku (line 1132).
- Iterate on each row of the dataframe dfJuku, determining whether each word has Kanji that are in the subset of 6100+ most common Kanji (lines 1134 - 1143).
- Iterate on each row for the dataframe dfJuku, determining whether it is already in the dfPart2 dataframe.  If it is, do nothing, as there is no rank information available.  If is not matched, add a new row to dataframe dfPart2 with the new Kanji data and rank equal to -1, with the source being 'Juku' (lines 1147 - 1165).
- Read in the 44492 japanese word file as dataframe, df44492 (line 1170).
- Iterate on each row for the dataframe df44492, determining whether it is already in the dfPart3 dataframe.  If it is, update rank information in the row(s) of dfPart3 according current row of df44492 plus 1.  There could be multiple results, so all must be updated.  If is not matched, no nothing (lines 1172 - 1181).
- Determine which rows in dfPart3 did not get ranked and drop them, creating the dictionary dataframe dfDict1 (lines 1183 - 118).
- Remove all word from dataframe dfDict1 that do not contain Kanji, creating another dictionary dataframe dfDict2 (line 1192 - 1206).
- Set an arbitrary upper limit for ranked words, maxRank (lines 1209 - 1229).
- Determine which words in dataframe dfDict2 exceed the maxRank.  Drop those words, creating the final dictionary dataframe dfDict3 (lines 1230 - 1232).
- Create an empty dataframe df5, with columns 'Reading', 'WordList', and 'MeaningList'. (line 1238).
- Iterate of the rows of dataframe dfDict3 (line 1241):
  - Determine whether the reading for the row in dataframe dfDict3 is already in dataframe df5 (line 1243).
    - If the reading is present, determine whether the word (called 'Kanji') is already associated with that reading.  If not, for that row in df5, append the word to the word list and the meaning to the meaning list.  If it is, just add the new meaning to the meaning list (lines 1246 - 1258).
    - If the reading is not present, add a new row to dataframe df5 with the reading, forming new word list and meaning list (lines 1259 - 1266).
- Sort dataframe df5 according to the reading in ascending order, creating a new dataframe, df6Sort.

- Output the Dictionary data structures to the file dictionary.h, first adding two new columns to the dataframe df6Sort (lines 1271 - 1276).
  - Writing the structure typedefs to the file (lines 1309 - 1337)
  - Iterating on each row of sorted dataframe df6Sort (line 1340):
    - Generate a Murmur3 hash key for the Kana reading (lines 1343 - 1352).
    - Add the Kana representation as an integer value and a byte string to the row for generating the Balanced BST later (lines 1353 - 1355).
    - Iterate on each element of the word list (line 1359):
      - Write out the Affix enumeration list, which has only the values 'jword', followed by 'meaning' (lines 1360 - 1362).
      - Write out the Okurigana list, which has only the Japanese word in UTF-8 characters, followed by the meaning string (lines 1363 - 1378).
      - Write out the Okurigana metadata that references the above Affix and Okurigana lists (lines 1379 - 1381).
    - Write out the list of Okurigana metadata pointers (lines 1384 - 1391).
    - Write out the Dictionary reading metadata structure referencing the list of Okurigana pointers.  The Kanji metadata list pointer is set to NULL (lines 1392 - 1394).
  - Write out the typedef for the Balanced BST to the file (lines 1396 - 1411).
  - Pull out the columns from the sorted dataframe df6Sort that are the Kana readings in integer form and byte string form and place them in a dataframe df6Sub.  Sort df6Sub according to the integer form in ascending order (lines 1413 - 1417).
  - Pull out the sorted Kana reading in byte string form and print out the Balance BST structures using the helper function print_balanced_bst (lines 1419 - 1421).

**If you have made it this far, I congratulate you!**
