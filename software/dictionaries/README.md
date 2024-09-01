# KanaChord Plus Dictionaries

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
- Novel 5K most common Kanji Novel_5K.csv. Source: [Novel 5k](https://docs.google.com/spreadsheets/d/1l2MNM5OWznIRVm98bTCA1qPNAFnM48xJIyUPtchxyb0/edit?usp=sharing)
- Kanji Dict (kanjidic2.xml) Source: [The KANJIDIC Project](http://www.edrdg.org/wiki/index.php/KANJIDIC_Project)
- Core 10K (Core10k.csv) list of over ten thousand common Japanese words with pronunciations and meanings. Source: [Core 10000](https://core6000.neocities.org/10k/)
- Core 5K Frequency (Core5kFrequencyMod3.csv) list of over five thousand of the most common Japanese words, with pronunciations and meanings.  Modified to place single entries on each line.  Source: [Core 5000 Frequency](https://core6000.neocities.org/freq/)
- Core 6k (Core6kMode.csv) list of over six thousand common Japanese words with pronunciations and meanings.  Modified to place single entries on each line. Source: [Core 6000](https://core6000.neocities.org/)
- Jukujikun words (jukujikun_mod.txt) List of official and unofficial Jukujikun, or Japanese words with pronunciations that do not match the Kanji representing those words.  Modified to combine official and unofficial lists. Source: [Kanjium - the ultimate kanji resource](https://github.com/mifunetoshiro/kanjium)
- 44492 Japanese Word frequency list (44492-japanese-words-latin-lines-removed.txt) - Source: [hingston/Japanese Github repo](https://github.com/hingston/japanese/blob/master/44492-japanese-words-latin-lines-removed.txt)

