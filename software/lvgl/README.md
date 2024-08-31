# Custom LVGL Configuration File
The default LVGL configuration file (lv_conf.h) is set up for general development.  Most of the LVGL capabilitiies and widgets are enabled.  While this is convenient for general development, a lot of the capabilities within LVGL are not used by KanaChord Plus.  Disabling unused LVGL features frees up flash memory in the Raspberry Pi Pico.  This was done to alow increasing the number of Japanese words in the dictionary.

The LVGL configuration file in this directory was used when compiling KanaChord Plus.  To use it, copy it into the Arduino libraries directory (Arduino/libraries/) after installing the LVGL library.  If there is an lv_conf.h file already present, it is advised to change its name or move it to another directory to preserve its contents.
