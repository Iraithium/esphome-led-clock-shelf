This is an ESP32 version of the DIY Machines LED Clock Shelf with ESPHome support for Home Assistant integration.  If you're not familiar with ESPHome, please familiarize yourself with how it works at https://esphome.io/

There are some difference from the DIY Machines version you will need to be aware of:
- This version uses ESP32, not Arduino.
- This version users 32 segments with 9 LEDs per segment. If you require something different, you will need to update the C++ code yourself.
- This version is wired exactly like the clock from https://github.com/florianL21/LED-ClockShelf.  Please consult the documention there for wiring diagrams.
- This version can only display 12 hours (32 segments).  24-hour (37 segments) will require some code modifications.  My original intention was to make this compatible for 24 hours, but there was no way for me to test the changes with my clock. If you are able to get this working, feel free to contribute back to the project.
