# LVGL Support
## Custom LVGL configuration file
The default LVGL configuration file (lv_conf.h) is set up for general development.  Most of the LVGL capabilitiies and widgets are enabled.  While this is convenient for general development, a lot of the capabilities within LVGL are not used by KanaChord Plus.  Disabling unused LVGL features frees up flash memory in the Raspberry Pi Pico.  This was done to alow increasing the number of Japanese words in the dictionary.

The LVGL configuration file in this directory was used when compiling KanaChord Plus.  To use it, copy it into the Arduino libraries directory (Arduino/libraries/) after installing the LVGL library.  If there is an lv_conf.h file already present, it is advised to change its name or move it to another directory to preserve its contents.

## Custom font generation
**NOTE: This information is provided in case a new custom font file needs to be created to support different characters or a different font style.  The custom font has aready been created and placed in the directory with the other souce code.**  
A custom font was created that contains the Kana characters, the set of 6,100+ Kanji currently supported, and a few special characters.  The [LVGL Font Converter](https://lvgl.io/tools/fontconverter) is used to generate a custom font file.  These are the values inserted into the fields of the converter.
- Name: KanaKanjiFont32
- Size: 32
- Bpp: 1 bit-per-pixel
- Fallback: <none>
- Output format: C file
- No options checked
- TTF/WOFF font: Noto_Sans_Mono_CJK_JP-Regular.otf
- Range: Taken from file kana_kanji_subset.txt

After the font file is downloaded, it needs to be modifed to compile properly with the Arduino project.  At the top of the file is the following block, which is a conditional define statement:
>#ifdef LV_LVGL_H_INCLUDE_SIMPLE  
>#include "lvgl.h"  
>#else  
>#include "lvgl/lvgl.h"  
>#endif  

Edit this block of code to have only one include statement:
>#include "lvgl.h"

Save the file and copy it into the directory containing the other source code before compiling.


