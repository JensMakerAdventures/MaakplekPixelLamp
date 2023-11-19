# MaakplekPixelLamp
Web controlled hacked 8x8 LED projector.

Project started on the Random Object Challenge 2023 at Maakplek (November 17-19th 2023).
https://maakplek.nl/wiki/doku.php?id=random_object_build_challenge_2023

This project is based off a "random object" which I received from Olav. The object is an 8x8 LED projector, made for parties. I'm using an ESP8266 to control it and expose a web interface so participants of the maker space and visitors can set nice pixel art. The LED board uses 8x SM74hc595. The original lamp can be bought here: https://nl.aliexpress.com/i/32885890252.html

### Power

It's rated at 10W and it actually used 10W. I tried power supplies <2A at 5V and they caused issues. Even the power supply (2A) I'm using right now gets unstable when using a chip that runs 3.3V logic, so I used a logic level converter to go to 5V logic. This problem doesn't exist when using a powerful power supply with 5A available. Also I put a 100 uF capacitor on the power supply. Otherwise the shift register chips weren't stable.

### Code
It took a while figuring out all the pins on this thing. Because there's markings on the LED board, then markings on the ribbon cable. These go to my own colored wires, then to the ESP8266. The ESP digital pinout to GPIO numbers also need to be looked up, so I made this conversion table.
- PLUS = VCC = MINUS WIRE SYMBOLS = RED
- MINUS = GND = PLUS WIRE SYMBOLS = BLACK
- A = SRCLK = LONG DASHES WIRE SYMBOLS = SH_CP = GREEN = D1 = GPIO5
- B = RCLK = CROSSES WIRE SYMBOLS = ST_CP = PURPLE = D2 = GPIO4
- IN = SER = SHORT DASHES WIRE SYMBOLS = DS = BLUE = D3 = GPIO0

### Web interface
I had to learn a bit of javascript, html and css for this, but I made a nice frontend. The interface can be accessed on PixelLamp.local, if connected to the makerspace wifi.
