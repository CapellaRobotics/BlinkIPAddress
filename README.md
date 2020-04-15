# BlinkIPAddress
Program to read the devices IP address and then blink it out using the LEDs to allow for easier determination the BeagleBone's IP address.

This can be turned into a startup script that will launch on boot to allow users to always be able to determine the IP address when booting up the BeagleBone.


IP address is displayed using the four user leds on the BeagleBone. It will flash all the leds three times and then do a scroll accross to indicate the start displaying. Then it will flash each digit in binary (the ones place is on the right (led 0) the eights place is on the left (led 3)), all leds on indicates a period, a long pause indicates a zero.
