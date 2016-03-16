# Door Traffic Indicator Client

This app allows you to track and warn people of door traffic. This was motivated by the problem of opening doors and hitting people on the other side.

## Neccessary Hardware

- [FRDM-K64F](http://www.nxp.com/products/software-and-tools/hardware-development-tools/freedom-development-boards/freedom-development-platform-for-kinetis-k64-k63-and-k24-mcus:FRDM-K64F)
- [Grove Shield](http://www.seeedstudio.com/depot/Base-Shield-V2-p-1378.html)
- [Grove Ultrasonic Ranger](http://www.seeedstudio.com/depot/Grove-Ultrasonic-Ranger-p-960.html)
- [Grove PIR Motion Sensor](http://www.seeedstudio.com/depot/Grove-PIR-Motion-Sensor-p-802.html)
- [Grove Chainable RGB LED](http://www.seeedstudio.com/depot/Grove-Chainable-RGB-LED-p-850.html)

## Setup Guide

1. Plug an Ethernet cable into the K64F to connect it to your network
2. Plug the Grove shield into the K64F
3. Plug the Grove Chainable RGB LED into the port labeled **UART**
  - Be sure the switch on the corner of the shield is set to the **5V** setting
4. Plug the Grove Ultrasonic Ranger into the port labeled **D3**
5. Plug the Grove PIR Motion Sensor into the port labeled **D6**
6. Plug a micro USB cable into the K64F's usb port labeled **OpenSDA** on the bottom of the board

## Building and Running

Connect your K64F to your network via an Ethernet cable. Using [yotta](http://yotta.mbed.com/), build the app using the following commands:

```
yotta target frdm-k64f-gcc
yotta build
```

Drag and drop the file `build/frdm-k64f-gcc/source/door-indicator.bin` on to the K64F's mbed drive. Press the **RESET** button your K64F to run the app.