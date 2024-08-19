![KanaChord_glamor](./images/KanaChordPlus_glamor.jpg)
# KanaChord Plus Keyboard 【カナコード・プラス・キーボード】
## Overview 【概要】
KanaChord Plus is an auxiliary keyboard that works in parallel with a standard English keyboard and generates Unicode keyboard macros to render Japanese Kana and Kanji characters.  This allows the user to immediately start typing in Japanese on any supported computer, rather that installing special software and switching language input modes on a single keyboard.  KanaChord Plus can assist the Japanese learner by providing a means to improve their skills by writing in Japanese, in addition to practicing their reading and speaking.  Note that during the composing of this README, KanaChord Plus Keyboard was used to type the Kana characters displayed.  KanaChord Plus is a follow-on project of ![KanaChord Keyboard](https://github.com/maccody/KanaChord), which supports typing of just Hiragana and Katakana Unicode characters.

### Writing in Japanese 【日本語で書く】
The Japanese writing system consists of three components: Hiragana, Katakana, and Kanji. Hiragana consists of 48 syllabic characters that are used for some words of Japanese origin and grammatical sentence fragments, e.g., あなた, これ, の (‘you’, ‘this’, possessive particle).  Katakana also consists of 48 syllabic characters that are used for foreign loan words and names, e.g., テーブル, クッキー, マック (‘table’, ‘cookie’, ‘Mac’).  The table below shows the syllabary of Hiragana and Katakana, known collectively as the Kana.
![Kana Table](./images/Kana_Table.png)

### What about Kanji? 【漢字はどうですか？】
Kanji consists of Chinese logography and conveys main ideas and names in sentences.  A Japanese word containing Kanji can consist one or more Kanji characters, e.g., 私, 東京, 何 (’, ‘Tokyo’, ‘what’), and Hiragana e.g. 買い物、ご飯、寒い ('shopping', 'meal', 'cold').  There are currently about fify-thousand Kanji characters and new ones are added on occassion.  In Japan, it is expected that a graduating high-school student, from entering elementary school, learn the Jōyō Kanji【常用漢字】, which contains 2136 Kanji characters that are reqularly used or are found in government documents.  A college graduate, especially with an advanced degree, would be expected to learn a thousand or more additional Kanji usually associated with their major. 

### Inspiration for a Keyboard 【キーボードの動機】
Over the last century, there have been a number of mechanical and electro-mechanical keyboards have been developed to type Kana and Kanji characters.  A quick web search for 'japanese keyboard' will provide a number of interesting links to follow.  These keyboards all shared common problems.  They were very expensive and slow at typing compared to contemporary English typewriters.  Only a relatively small subset of Kanji (a few thousand) were available.

The availability of inexpensive computers in recent decades have obsoleted these typewriters, replacing purely hardware solutions with ubiquitious keyboards augmented with software running on the computer.  The software implements an Input Method Editor (IME) that takes Romaji (Roman (english) characters that model the Kana syllabary) and predict one or more Japanese words that may contain Kanji. Junferno's video [The Challenge of Making a Keyboard for Every Language](https://www.youtube.com/watch?v=MBQvN03i4-4) provides a nice overview of the difficulties of creating a keyboard that supports non-English characters and the succession of typewriters by computers with keyboards and IMEs.

The layout of the Kana table inspires a chording keyboard input method. Chording is where multiple keys are pressed simultaneously on the keyboard to obtain a desired character output.  The illustration below provides further explanation and demonstration of how chording is used on the KanaChordPlus Keyboard.
![Kana Chording Example](./images/Kana_Chording_Example.png)

A chording keyboard concept is sufficient for generating all of the Kana.  It is impractical for Kanji, though, as even a chording keyboard would quickly become impractical when attempting to support more than a few dozen Kanji.  Instead, KanaChord Plus incorporates an 'incremental' IME.  As the Hiragana or Katakana are typed, they are presented on a touch screen along with a list of corresponding Kanji characters and/or Japanese words containing Kanji.  The list is updated with each new Kana character typed, incrementally.  This is not a word-predictive IME, as the Kanji and Japanese words presented are only those corresponding to the Kana currently typed.  The user can either send the Kana characters typed to the computer or select one of the Kanji characters or Japanese words to replace the Kana characters and send that to the computer instead.
## Features 【特徴】
### Dynamic Key Coloring 【動的なキーの色付け】
KanaChordPlus includes dynamic key coloring that provides indication of character type, Kana mode, and error feedback.  The three Shift keys on the left keypad are normally white:
- Voicing mark, a.k.a. dakuten or ten-ten (ﾞ),
- Plosive mark, a.k.a. handakuten or maru (ﾟ),
- Small characters, a.k.a. chiisai (小), also used to ‘shift’ to alternate punctuation and special characters.

Punctuation keys on the top two rows of the right keypad are also normally white:
- Vertical iteration mark (〳,〴,〵), continuation mark (ー), and separation (・) characters ,
- Sentence pausing (、) and ending characters (。, ! , ?),
- Wide space and ellipsis (…) characters,
- Commonly-used quotation (「, 」,『, 』), bracket (【, 】,〈, 〉), and special (〜,※) characters.

The Kana and horizontal iteration mark (ゝ,ゞ,ヽ,ヾ) keys are green for Hiragana mode or blue for Katakana mode, as selected by the mode key (ひカ) at upper right-hand corner of the right keypad.
![KanaChordPlus_installed_grn](./images/KanaChordPlus_installed_grn.jpg)
![KanaChordPlus_installed_blu](./images/KanaChordPlus_installed_blu.jpg)

### Error Feeback 【エラーフィードバック】
Feedback is provided to the user for incorrect key combinations by changing those keys red.  Releasing the keys will cause the keys to revert to their original colors.  Some examples are provded below:

Keyboard error lighting with press of ! and お key combination.
![KanaChordPlus_error_1](./images/KanaChordPlus_error_1.jpg)

Keyboard error lighting with press of ﾞ, な, and あ key combination.
![KanaChordPlus_error_2](./images/KanaChordPlus_error_2.jpg)

### 'Incremental' Input Method Editor 【「インクリメンタル」インプットメソッドエディター】
As mentioned previously, Kanji characters and Japanese words containing Kanji are typed via an 'incremental' Input Method Editor (IME).  As Kana characters are typed, they are presented at the top of KanaChord Plus display in an single-line Editor Window.  Below the Editor Window, a Kanji List is generated, consisting of Japanese words and Kanji characters.  The content of the Kanji List is updated as new Kana character are entered, i.e., incrementally.  The list is displayed with Japanese words presented first, followed by Kanji characters ordered according to the most common first.  The color of each entry indicates whether it is a word or particular Kanji reading:
- Orange - Japanese word
- Green - Kanji with kunyomi or Japanese reading
- Blue - Kanji with onyomi or Chinese reading
- Brown - Kanji with nanori or name reading

Each entry in the Kanji List has two parts.  On the left side, the kanji character or word (one or more Kanji, sometimes with Kana) is presented.  On the right side, a definition in English is presented.  Definitions are provided to aid the user in making the appropriate selection.  Note that each entry on the list is pronounced according to the Kana present in the Editor Window, i.e., they are homophones, sounding the same.  

The procedure for selecting a Japanese word or Kanji character is illustrated in the animated GIF below.  Note that the left portion of a Kanji List entry must be pressed.  Pressing the definition portion of an entry has no effect.
![KanaChordPlus_demo](./images/KanaChordPlus_demo.gif)

The length of the Kana List can exceed that of the vertical extent of the display.  The list can be scrolled by dragging a finger up or down along the right portion of the list.  Up to ten entries are displayed at any time.  If more than ten entries are in the list, special list entries are provided to select the next set of less common Kanji (move down the list) and select the next set of more common Kanji (move back up the list).


### Support Menu 【サポートメニュー】
Pressing on the inverted caret at the upper left hand corner of the display will a display a dropdown menu listing several support functions.  The first two entries support making changes to the editor window.  The last two entries support configuration of KanaChord Plus.
- **Delete last character** - Deletes the last character present in the editor window.  The word and Kanji list is regenerated according to the characters now present in the editor window.  Note that the content of the Kanji List may not be valid if there is Kanji characters in the editor window.
- **Clear input and Kanji list** - Deletes the content of the editor window and clears the Kanji List.
- **Select Macro output mode...** - Opens a window for Unicode kee sequence selection.  Press the button labeled with a checkmark to save the selected mode to flash memory.  Press the button labeled with an **x** to close the window, abandoning any changes.  KanaChord Plus can output different Unicode key sequences to support different operating systems and applications.  There are currently three settings available:
  - Four-characer hexidecimal Unicode value, followed by Alt-X: Supports Microsoft Word and Wordpad, and LibreOffice Writer.
  - Ctrl-Shift-u, followed by four-character hexidecimal Unicode value: Supports Linux applications like LibreOffice Writer and Firefox.
  - Option key, followed by four-character hexidecimal Unicode value: Supports MacOS applications (not yet tested).
- **Calibrate Screen...** - Opens a window to propt the user to initiate touch screen calibration.  To function properly, the touch screen of the KanaChord Plus display needs to be calibrated.  When the window opens, the user has the option of starting a calibration or closing the window without performing a calibration.  Follow the instructions displayed in the window to perform the calibration.  Once the calibration steps are completed, the calibration values are automatically saved to flash memory.  The user can then close the window or repeat calibration, if desired.  Note that the first time KanaChord Plus is used after programming, the calibration screen will be automatically displayed to require the user to calibrate the touch screen.

## Implementation Overview 【実装の概要】
The primary electical componets of the KanaChord Keyboard are a Raspberry Pi Pico (RP2040) microcontroller, an Adfruit NeoKey 5x6 Ortho Snap-Apart keyboard PCB, and thirty Cherry MX mechanical key switches.  The keycaps and keyboard enclosure are 3D printed designs created with TinkerCAD.  Blank keycaps and an OpenSCAD tight-fit box generator were found on Thingiverse.  No special hardware, such as screws are needed to mount the electrical components and assemble the enclosure.  Details on the electronics, enclosure, and their assembly can be found in the hardware subdirectory.

KanachordPlus' software was developed with the Arduino IDE (version 1.8.19) and Earle Philhower's RP2040 board support package.  The USB Keyboard library, Adafruit Neopixel library, and Mark Stanley's Key and Keypad libraries provided critical software functions.  Details on the software can be found in the software subdirectory.

## Hardware and Software Licenses 【ハードウェアとソフイウェアのライセンス】
![Cc_by-nc-nd_icon svg](./images/Cc_by-nc-nd_icon.svg.png)
KanaChordPlus' circuit design and the STL files for the keycaps, component mounting parts, and enclosure parts are placed under Creative Commons Attribution-NonCommercial-NoDerivatives license.

![GNU_GPL_V3](./images/GNU_GPL_V3.png)
KanaChord Plus' source code that I wrote (KanaChordChordPlus.ino, kana.h, kbd_mgt.cpp, kbd_mgt.h, **AND OTHERS**) are placed under GNU General Public License, Version 3.0.

The slightly-modified versions of Mark Stanley's Key and Keypad libraries (Key2040.cpp, Key2040.h, Keypad2040.cpp, and Keypad2040.h) fall under the license of the original sources.

## Feedback is Welcome! 【フィードバックは大歓迎です！】
My goal is to provide an axuilary keyboard that is useful to those who want to start writing in Japanese, while learning Japanese.  I hope to have provided operational features, keycaps, and Asian special characters that would most likely be used.  If you have recommendations for additional features, alternate keycaps, or Asian special characters, please let me know.  I will accomodate as many recommendations as feasible.

## The Future of KanaChord Plus Keyboard 【カナコード・プラス・キーボードの今後】
[As, mentioned above, KanaChord Keyboard outputs only Kana Unicode, not Kanji Unicode.  This is an obviously glaring omission for writing in Japanese.  A software IME (Imput Method Editor) that accepts Kana Unicode input could be used to make up for this shortcoming.  This takes away from the goal of not needing to install special software.]: #

[To that end, I have been developing the next version of this keyboard, which also outputs Kanji Unicode. I call it KanaChord Plus Keyboard (カナコード・プラス・キーボード). It is approaching the end stages of development and I hope to release it soon.  I can say that it will output Unicodes for over 6,000 of the most commonly used Kanji. The user will be able to select from them, as they type the appropriate onyomi (Chinese), kunyomi (Japanese), or nanori (name) readings using the Kana. This will help the user learn Kanji, as they learn the proper readings and pronunciations from the Kana typed.]: #
