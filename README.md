# Touching

[Elisabeth Schneller](https://www.elisabethschneller.ch/), [Donat Fritschy](https://www.fritschy.ch/), "Touching", 2016

The artwork "Touching" by Elisabeth Schneller consists of a stela composed by 7 cubes made of black fired clay. The cubes show the text "Touch me" in Braille on one side, and seven engraved texts on other surfaces.

The installation by Donat Fritschy adds interactive sound to the stela: As soon as someone is touching one of the engraved texts, the text is spoken out loud.

https://www.youtube.com/watch?v=2-fBDPu78pA

## Hardware

* [Arduino UNO](https://store.arduino.cc/arduino-uno-rev3)
* [Adafruit MP3 shield](https://www.adafruit.com/product/1790)
* 4 Ultrasonic Sensors HC-SR04 (2cm - 500cm)
* 2 Amplifiers and Electro dynamical exciters mounted inside one of the cubes, providing sound

## Software

This [arduino sketch](src/TouchMe.ino) provides the logic. The four ultrasonic sensors mounted at the top of the stela are polled periodically. If one of the measurements falls into the defined range for a given text, the corresponding MP3 file is played.

Libraries used:
* https://www.arduino.cc/en/Reference/SPI
* https://www.arduino.cc/en/Reference/SD
* https://www.arduino.cc/en/Reference/EEPROM
* https://playground.arduino.cc/Code/NewPing/
* https://github.com/adafruit/Adafruit_VS1053_Library
* https://github.com/jeelabs/jeelib/blob/master/Ports.h
