# KanaChord Plus Software
The software for KanaChord Plus was developed with open-source tools and libraries:
- Arduino Integrated Development Environment (Version 1.8.19),
- Wiring programming language (C++-ish),
- [Earle Philhower's RP2040 Arduino board support package](https://github.com/earlephilhower/arduino-pico),
- Libraries used:
   - [Arduino USB Keyboard Emulation](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/),
   - [Mark Stanley's Arduino Keypad](https://github.com/Chris--A/Keypad),
   - [Adafruit Neopixel](https://github.com/adafruit/Adafruit_NeoPixel),
   - [Light and Versatile Graphics Library](https://lvgl.io/), version 8.4.

Note that the source code for the Keypad library has been slightly modified for the RP2040 and the file names have been renamed accordingly.  The output drive current for the keyboard polling lines has been set to 12 milliamps, which is the maximum for the RP2040.  It was found that the default setting of 4 milliamps provided insufficient current for the keyboard.

## Unicode Data
The file kana.h contains C++ arrays containing the 16-bit Unicode values for Kana (Hiragana and Katakana) characters, and some Chinese/Japanese/Korean punctuation and special characters.  The file kanji_md.h contains C++ structures containing 16-bit Unicode values for Kanji characters.  Details on these Unicode blocks can be found in the following PDFs:
- [Official Unicode Consortium Hiragana code chart](https://www.unicode.org/charts/PDF/U3040.pdf)
- [Official Unicode Consortium Katakana code chart](https://www.unicode.org/charts/PDF/U30A0.pdf)
- [Official Unicode Consortium CJK Symbols and Special characters code chart](https://www.unicode.org/charts/PDF/U3000.pdf)
- [Official Unicode Consortium General Punctuation characters code chart](https://www.unicode.org/charts/PDF/U2000.pdf)
- [Official Unicode Consortium CJK Unified Ideographs code chart](https://www.unicode.org/charts/PDF/U4E00.pdf)

The arrays containing the Kana are grouped according Hiragana and Katakana character sets. Individual arrays represent unaugmented (base) characters and augmented characters, i.e., ten-ten, maru, and small (chiisai) characters. Each array is organized by ten 'consonants' (rows) and six 'vowels' (columns).  Special characters are also grouped according to Hiragana and Katakana character sets, although this results in most characters being duplicates in these sets. This was done to simplify the code used to access the arrays.  

The function getKanaUnicode(), defined in kbd_mgt.cpp, takes inputs of Kana mode, special character mode, shift mode, and index row and column to select a Unicode character value. If the selected Unicode value is 0x0000, then an illegal key has been selected.

## Compiling and Uploading the KanaChord Keyboard Software
First, start the Arduino IDE and click on Tools on the menubar and select Manage Libraries from the drop-down menu.  Install Earle Philhower's RP2040 Arduino board support package and the Arduino USB Keyboard emulation library.  Next, select Tools on the menubar and select Board from the drop-down menu.  Select 'Raspberry Pi RP2040 Boards' from the displayed cascade menu, and then 'Raspberry Pi Pico' from the next displayed cascade menu.

Now that Pico board support is in place, adjust the board parameters as shown in the picture below:  
![KanaChord_Plus_Setup](./images/KanaChord_Plus_Arduino_setup.jpg)

Place the KanaChord Plus source files into a new directory named KanaChordPlus. Load KanaChordPlus.ino with the Arduino IDE. Click the Verify button to ensure that the KanaChord source code compiles successfully.  Compiling will take some time because of the size of the dictionary header files.  The compile status should be as shown in the picuture below.  If compiling fails, check that all needed files and libraries are installed and the configuration is correct.

![KanaChordPlus_Arduino_compile](./images/KanaChordPlus_Arduino_compile.jpg)

Now, connect the Raspberry Pi Pico to the computer with a USB cable.  Click the Upload button to compile the KanaChord source code and upload the compiled binary to the Pico.  If the upload fails, make sure that the USB cable is securely connected to the Pico and the computer performing the programming. It may also be necessary to hold down the BOOTSEL button on the Pico while plugging the USB cable into the computer.

## Details on the KanaChord Plus software
The software in KanaChord Plus is much more complicated compared to that in the original [KanaChord Keyboard](https://github.com/maccody/KanaChord).  KanaChord only has to support the typing of Unicode for the Kana and a few special characters.  This was accomplished via a keyboard polling loop, a look-up table to match valid key combinations to Unicode, and sending them through a USB keyboard emulation to the host computer.  KanaChord Plus adds support for Kanji Unicode output, which is not trivial and required some compromises.  The [dictionaries of Kanji and Japanese words](./dictionaries/README.md), the incremental IME (Input Method Editor), and the touch screen display, with [custom large font](./lvgl/README.md), all contributed to the complexities.  

Fortunately, the Raspberry Pi Pico has hardware features that were not fully exploited by the KanaChord Keyboard.  One of two processing cores in the Pico are unused, as well as over 90% of the 2MB of flash ROM. Two SPI (Serial Peripheral Interface) ports and additional general purpose I/O ports also contribute to enabling the addition of extra capabilities required by the software in KanaChord Plus.

Having two cores in the RP2040 microcontroller enables hardware parallelism, dividing the processing load so that no single processor is overloaded.   In the Pico, Core 0 of the RP2040 is dedicated to interfacing with the USB interface.  In the KanaChord software, Core 0 was also used to poll the keyboard.  It made sense to keep these functions together in KanaChord Plus.  Core 1 is dedicated to running the incremental IME, the display, and the touch screen, as their functions are tied closely together.

The subsections below describe various elements of the KanaChord Plus software.  Note that these descriptions are not detailed and do not cover every function in the code base.  There are some helper functions that are only inferred.  Also, the flowcharts may lack some details.  The best way to fully understand the code is to study the source code itself.

### Arduino setup() and loop() functions
A high-level flowchart of the Arduino setup() and loop() functions for Core 0 and setup1() and loop1() functions Core 1 is presented below:   
![Software_Flowchart](./images/KanaChord_Plus_top_level_flowchart.gif)

Core 0 setup() processing initializes keypad polling, Neopixel configuration, and USB keyboard emulation.  Core 0 loop() processing manages keypad inputs, Neopixel states, and Unicode macro outputs through the USB keyboard emulation.  A lot of the keypad processing performed by Core 0 involves determining whether a pressed key combination is valid or not.  If the combination is invalid, the Neopixels of the pressed key combination are turned red.  When the invalid key combination is released, the key colors are restored to their proper states.  If the combination is valid, a 16-bit Unicode, paired with a command flag, is sent to Core 1 via the Core 0-to-Core 1 FIFO.

Unicode characters are recieved from Core 1 via the Core 1-to-Core 0 FIFO. The Unicode characters are convered to Unicode macro sequences and placed into an output queue for output through the USB keyboard emulation.  The macro sequence sent is determined by a mode variable that is user-configurable and stored in non-volatile storage.  The macro sequences currently supported are for Microsoft Windows applications (e.g., MS Word, Wordpad, LibreOffice), Linux applications (e.g., LibreOffice, Firefox), and MacOS applications (functionality not tested yet).  The transmission rate of the macro code sequences is controlled to avoid sending garbled sequences to the host computer.

Core 1 setup() processing initializes the ILI9341 TFT display controller, the XPT2046 touch screen controller, the LVGL library, and the LVGL main interface.  Core 1 loop() processing manages the incremental IME, the LVGL display, and touch screen functions that allow the user to select Kanji and Japanese words based upon the Kana characters typed.  The simplicity of the loop belies the functionality performed by Core 1.  There are several complex functions performed by the incremental IME and the LVGL event handler executes callback functions as necessary.

Core 1 receives 16-bit Unicode and command flag via the the Core 0-to-Core 1 FIFO.  These are separated and the command flag determines how the Unicode code is inserted into the editor queue.  A command flag value of zero indicates that the Unicode is a Kana character.  This causes the Kana Unicode to be appended to the editor queue, unless the queue has been modified through selection of a Kanji or Japanese word.  In that case, the editor queue is first flushed of its contents, sent to Core 0 via the Core 1-to-Core 0 FIFO, and the Kana Unicode then placed into the emptied editor queue.  A command flag value of one indicates tha the Unicode is a special character.  The character's treatment depends upon the character.  If the special character is not a wide space, the Unicode is appended to editor queue the editor queue is flushed.  If the character is a wide space it is not appended to the editor queue and the editor queue is flushed.  If the editor queue is already empty, the wide space Unicode is placed in the editor queue and the queue is then flushed.  Doing this allows for conditional output of wide space characters, which normally do not appear within Japanese sentences.

### Functions that set up the hardware

![Hardware setup functions](./images/KanaChord_Plus_hardware_setup_functions.gif)

### Functions for configuration settings

![Configuration settings functions](./images/KanaChord_Plus_settings_functions.gif)

### Functions to manage the Reading dictionaries

![Reading dictionary fuctions](./images/KanaChord_Plus_reading_functions.gif)

### Functions to display and manage Kanji lists

![Kanji display functions](./images/KanaChord_Plus_Kanji_display_functions.gif)

### Functions to display and manage Okurigana lists

![Okurigana display functions](./images/KanaChord_Plus_Okurigana_display_functions.gif)

