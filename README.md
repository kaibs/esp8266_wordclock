## esp8266_wordclock

this repo contains code for a [QLOCKTWO](https://qlocktwo.com/de/)-style clock with a German layout based on an ESP8266 (NodeMCU).

* using adressable led-stripes (WS2812B) with the [fastled](https://github.com/FastLED/FastLED)-library
* automatic time-synchronization via [NTP](https://github.com/arduino-libraries/NTPClient)
* brightness can be changed over [MQTT](https://github.com/knolleary/pubsubclient)
    * instructions/config for usage in [Home Assistant](https://www.home-assistant.io/) are found [here](https://github.com/kaibs/esp8266_wordclock/blob/main/HAconfig/example_config_home_assistant.yaml)

### clockface

I've included a .dxf with the original design of my clockface in this repository. I also provide a .png with 7874x7874 pixels resolution. This image is mirrored, so that it can be printed onto an adhesive film and glued onto the backside of a glass. This way the light of the LEDs isn't diffused before it arrives the mask. 

### debugging

There exists a debugging-mqtt-channel for testing LEDs and general functionality. Default is ```home/livingroom/wordclock/debugging```.
It expects a message in the format ```"DD.MM,hh:mm,Weekday"``` (e.g. ```"11.12,13:14,1"``` for Monday the 11. of december at 13:14). The NTP time-synchronisation can be re-enabled by sending ```"ntp"``` as as message.
