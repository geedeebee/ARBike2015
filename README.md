# ARBike2015 - IoT Bike Demo

Some basic code (a sketch) to run on the Arduberry (ATMEGA328P like an Arduino Uno) that controls the various sensors on the IoT Mountain bike demo.



16 Channels of data are collected and sent on demand as a serial stream to the Raspberry Pi. The RaspPi requests this data by sending an ascii "s" to the serial interface. About 10 readings per second cn be achieved - the bottle neck is the ultrasonic distance measurement sensor that requires a minimum time between pings...

The sketch is a modified version of the HQ code. Many edits were necessitated by the differences in hardware used, and some other tweaks in an attempt to maximize performance
