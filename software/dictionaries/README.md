# KanaChord Plus dictionaries
Providing Kanji support in KanaChord Plus is a challenging task!  Japanese is a phonetically limited language, with at least twelve (perhaps up to 21) distinct consonants and five distinct vowels.  The addition of mora (timing of pronunciation) and pitch accent add to the challenge.  The Kanji themselves are subject to interpretation, as the pronunciation of any particular Kanji can have Chinese readings (onyomi), Japanese readings (kunyomi), or readings assigned for name usage (nanori).  As a consequence, there can be many Kanji and Japanese words that are homophones (sound the same), e.g., 花 (hana) flower and 鼻 (hana) nose, where pitch accent differentiates them otherwise.  This Wikipedia web page on [Japanese phonology](https://en.wikipedia.org/wiki/Japanese_phonology) provides an introduction for a deeper dive into the topic.  

And additional challenge is providing a sufficiently diverse set of Kanji to support Japanese typing for most users.  There a currently over [fifty-thousand Kanji characters](https://en.wikipedia.org/wiki/Japanese_writing_system), although many are archaic and not in formal usage today.  Still, there are many thousands of Kanji that are in use and choosing which ones to support would be a daunting task.  Fortunately, as with other languages, some Kanji are used much more than others, which provides a starting point for which Kanji to support.  The Japanese government mandates that all Japanese high school graduates learn the [常用漢字 (jōyō kanji)](https://en.wikipedia.org/wiki/J%C5%8Dy%C5%8D_kanji), or 'list of regular use Kanji', literally. These 2,136 Kanji provide a minimum standard for all Japanese to read and write.  Mulitple schollars have performed frequency analysis of Kanji to create more extensive lists of frequently-used Kanji.  The [Novel 5k](https://docs.google.com/spreadsheets/d/1l2MNM5OWznIRVm98bTCA1qPNAFnM48xJIyUPtchxyb0/edit?usp=sharing) spreadsheet and [Kanji Dict 2](http://www.edrdg.org/wiki/index.php/KANJIDIC_Project) XML document are two of these lists and are used for selecting 6,165 of the most common Kanji, with their associated meanings in English. This is the subset of Kanji that is currently supported by KanaChord Plus.  This Kanji subset should be suffcient for most users, even at the collegiate level!  Note that then number of Kanji supported also drives the size of the Kanji font character set stored on the Pico.

Cross-referencing each reading to all associated Kanji requires 'inverting' the content of the Kanji Dict 2 XML document.  Kanji Dict 2 presents a structed list of Kanji, each with their relevant onyomi and kunyomi readings (in Katakana and Hiragana, respectively), nanori pronunciations (in hiragana), and English meanings.  In addition, many kunyomi pronunciations have [okurigana](https://en.wikipedia.org/wiki/Okurigana) (accompanying hiragana after the Kanji) used to inflect adjectives and verbs.  Sequentially processing each Kanji entry and tabulating it to each reading yields a table of readings with all Kanji associated Kanji and there English meaning.  There are three dictionaries generated, one each for onyomi, kunyomi, and nanori.  Note that the okurigana are included in the kunyomi dictionary to allow for direct selection rather than typing each Hiragana manually.

Beyond the Kanji are Japanese words in general, which can be composed of Hiragana, Katakana, Kanji, or a combination of all three.  While it is possible to always type Kana characters, sometimes to generate individual Kanji, to then create individual Japanese words, this can become tedious, especially when a lot of the same words are typed frequently.  Consequently, a forth dictionary is a collection of commonly used Japanese words.  Coincidentally, a number of common Japanese words are [Jukujikun](https://en.wiktionary.org/wiki/jukujikun), or 'compound character reading', which use Kanji characters to convey the meaning but the pronunciation is of Japanese origin.  An example is 今日 (kyō) or 'today', which would have a literal pronunciation of 'ima hi' or 'now day', based upon the Kanji used.

Finding extensive lists (more than a few thousand) of Japanese words containing frequency ranking, reading, and English meanging proved a bit difficult.  Ultimately three Core word spreadsheets, used for Anki decks, a list of official and unofficial Jukujikun words, and a frequency-ordered list of over 44,000 Japanese words.  Not surprising, many of the most-common Japanese words consist of Kana only and are not part of the dictionary.  Of the remaining most-common words containing Kanji, 6240 are in the current dictionary, due to memory constraints (discussed below).

## Data structures forming the dictionaries
Several approaches were considered to store the dictionaries on KanaChord Plus:
1. External flash card accessed through the Pico's SPI interface, using a flash card reader library - plenty of storage space, relatively slower access, and driver overhead.
2. Use part of the Pico's flash ROM as a flash drive, using a flash card reader library - Limited space (less than two megabytes), about four times faster access with QSPI compared to SPI, but same driver overhead.
3. Create data structures representing the dictionaries that are stored in the flash ROM's program memory and accessed directly - Limited storage space, though more efficient, and fastest access possible, with no drive overhead.

The third option was chosen for this implementation of KanaChord Plus.  Speed in retrieving Kanji and English meaning data for the Kana currently typed will impact the responsiveness of the keyboard and the overall user experience.  It is anticipated that a future version of KanaChord Plus would use a board-compatible, RP2040-based microcontroller with 4MB, 8MB, or 16MB of flash ROM.  The recent (August 2024) introduction of the Raspberry Pi Pico 2 (RP2340-based microcontroller), with 4MB of flash ROM would be a suitable replacement. The faster microcontroller could maintain or improve responsiveness, while providing extra flash ROM stoarge for either more Kanji, more Japanese words, or both!  

Several data structures were created to store the content of the four dictionaries and support rapid dictionary search based on the kana characters currently entered.  The illustration below shows the relationship of these data structures.

![Data structure relationship](./images/data_structure_relationship.gif)

A [Binary Search Tree (BST)](https://en.wikipedia.org/wiki/Binary_search_tree) is employed to quickly search each dictionary.  The sorted keys are hashes of Kana character sequences.  The hash algorithm used is [32-bit Murmur Hash version 3](https://en.wikipedia.org/wiki/MurmurHash) - the implementation used is found on the referenced Wikipedia page.  Each node of the BST is a data structure contains the following elements:   
- A 32-bit key that is a Murmur hash of a reading (Kana character sequence).
- A pointer to a reading matadata structure for the reading used for the key.
- Pointers to two child BST structures, which can be NULL if there is no child for a branch of the tree.

During the search down the BST of a dictionary, the submitted reading hash is compared to the structure's hash key.  If the submitted hash is either less than or greater than the hash key, the search continues to the structure referenced by the appropriate pointer.  If that pointer is null, then there is no match for the submitted reading hash.  If the submitted hash is equal to the key in the current BST structure, then the pointer to the associated reading metadata structure is returned.  

The reading metadata structure provides information regarding the Kanji and Japanese words associated with a reading.  The reading structure contains the following elements:
- A value indicating the number of Kanji and Okurigana or Japanese words associated with the reading (Kana characer sequence).
- A pointer to an array of pointers to Okurigana or Japanese word/meaning metadata structures.  Depending upon the dictionary, this pointer may be a null pointer.
- A pointer to an array of pointers to Kanji metadata structures.  Depending upon the dictionary, this pointer may be a null pointer.





Okurigana / Japanese word metadata structure
- Number of Affix enumerations and Okurigana strings or Japanese word/meaning strings.
- Pointer to array of Affix enumerations.  
- Pointer to array of Okurigana or Japanese words/meanings.

Kanji metadata structure
- 16-bit Unicode value for the Kanji.
- Number indicating rank order value - lower value is more common.
- Character pointer to string containing English meaning for the Kanji.






## Programmatic dictionary file generation
Need to programatically generate some files, due the the large amount of data.
- kana_kani_subset.txt - List of Unicode hexidecimal values to submit to LVGL font converter.
- kanji_ms.h - Contains character strings for meanings of Kanji meaning in English.
- kanji_md.h - Data structures containing Kanji Unicode values, rank of commonality, and references to English meaning string.
- onyomi.h - Data structures forming the dictionary of onyomi (Chinese) readings for Kanji.
- kunyomi.h - Data structures forming the dictionary of kunyomi (Japanese) readings for Kanji.
- nanori.h - Data structures forming the dictionary of nanori (name) readings for Kanji.
- dictionary.h - Data structures forming the dictionary of common Japanese words containing Kanji.


Describe the Python script and what it does.  

Python libraries needed by the script
- Xml library, particularly xml.etree.ElementTree
- Pandas library - Dataframe manipulation
- Itertools library
- Freetype library?
- Mmh3 library - Murmur hash generatoin library.

Describe each data file in this directory, its source, and any modifications made to simplify processing.  
- Novel 5K most common Kanji (Novel_5K.csv). Source: [Novel 5k](https://docs.google.com/spreadsheets/d/1l2MNM5OWznIRVm98bTCA1qPNAFnM48xJIyUPtchxyb0/edit?usp=sharing)
- Kanji Dict 2 (kanjidic2.xml) Source: [The KANJIDIC Project](http://www.edrdg.org/wiki/index.php/KANJIDIC_Project)
- List of Kana characters and their corresponding least-significant byte of Unicode values (kana_list.csv).
- Core 10K list of over ten thousand common Japanese words with pronunciations and meanings (Core10k.csv). Source: [Core 10000](https://core6000.neocities.org/10k/)
- Core 5K Frequency list of over five thousand of the most common Japanese words, with pronunciations and meanings (Core5kFrequencyMod3.csv).  Modified to place single entries on each line.  Source: [Core 5000 Frequency](https://core6000.neocities.org/freq/)
- Core 6k list of over six thousand common Japanese words with pronunciations and meanings (Core6kMod.csv).  Modified to place single entries on each line. Source: [Core 6000](https://core6000.neocities.org/)
- List of official and unofficial Jukujikun, or Japanese words with pronunciations that do not match the Kanji representing those words (jukujikun_mod.txt).  Modified to combine official and unofficial lists. Source: [Kanjium - the ultimate kanji resource](https://github.com/mifunetoshiro/kanjium)
- 44492 Japanese Word frequency list (44492-japanese-words-latin-lines-removed.txt).  Source: [hingston/Japanese Github repo](https://github.com/hingston/japanese/blob/master/44492-japanese-words-latin-lines-removed.txt)

