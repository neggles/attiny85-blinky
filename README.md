# attiny85-blinky
Arduino blinky shenanigans for ATtiny85/Gemma/Trinket and WS2812B LEDs.

This was written and tested using the Arduino ATtiny core from http://highlowtech.org/?p=1695, but ought to work with other cores.

Uses a hardware pin-change interrupt, and the Adafruit Neopixel library.

It has been confirmed to operate on an Adafruit GEMMA and 3.3v Trinket, though it's important to note that if your cheap WS2812B LEDs are receiving over ~4v VCC, a 3.3v micro may not be able to push the DIN pin hard enough.

Also included is an eagle .brd/.sch pair for a small CR2032-powered widget with a SOIC ATTiny85 and 4 LEDs.
Battery life isn't amazing, and you should omit/bridge the 3.3v regulator if you want it to work under USB power, so it's not perfect, but it's neat.
(I'm not actually sure why I kept the 3.3v regulator, if we're honest. It does very little at best.)

PCBs can be bought directly from OSHPark here, at about six bucks for 3: https://oshpark.com/shared_projects/sMGC09Kl

Here's a pic of a couple I made, sans USB connections since I hadn't received the sockets at this point; http://i.imgur.com/KQKQHtf.jpg

Otherwise, this code is fairly flexible and modular (on purpose) and I tried to comment it as best I could.

Let me know what you think :)