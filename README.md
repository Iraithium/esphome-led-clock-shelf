This is an ESP32 version of the DIY Machines LED Clock Shelf with ESPHome support and Home Assistant integration.  If you're not familiar with ESPHome, please familiarize yourself with how it works at https://esphome.io/

There are some difference from the DIY Machines version you will need to be aware of:
- This version uses ESP32, not Arduino.
- This version users 32 segments with 9 LEDs per segment.  I would like to make this all configurable, but I'm not there yet. If you require a different setup, you will need to modify the C++ yourself.
- This version is wired exactly like the clock from https://github.com/florianL21/LED-ClockShelf (thank you florianL21 for the inspiration)  Please consult the documention there for wiring instructions and diagrams.
- This version can only display 12 hours (32 segments).  A 24-hour (37 segments) clock will require some code modifications.  My original intention was to make this configurable as well, but I have no way of testing with a 12-hour clock. If you are able to get this working, feel free to contribute back to the project.
