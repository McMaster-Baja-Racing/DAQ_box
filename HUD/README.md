# HUD

This readme is for the Heads-Up-Display and related files.
#### Demo
![IMG_1119](https://user-images.githubusercontent.com/6884645/124519293-dc9d1300-ddb6-11eb-8ae3-a471309b16ff.gif)


## Hardware
#### Parts List
- Arduino supported microcontroller (and supported by [Adafruit_NeoPixel lib](https://github.com/adafruit/Adafruit_NeoPixel))
- Neopixel stick(s) or equivalent - [digikey link](https://www.digikey.ca/en/products/detail/kitronik-ltd/35129/8635461), [datasheet](https://resources.kitronik.co.uk/pdf/35129-zip-strip-datasheet.pdf)
- On/Off [switch](https://www.digikey.ca/en/products/detail/cit-relay-and-switch/AST11SEBQ/12503280?s=N4IgTCBcDaIIIGUAqBGFCCiAhAiiAugL5A)
- OPTIONAL: Rotary potentiometer for testing

#### Wiring
Refer to this [guide](https://learn.adafruit.com/adafruit-neopixel-uberguide/basic-connections) first, for background information.

Schematic (refer to code for microcontroller pin #'s):

![image](https://user-images.githubusercontent.com/6884645/124518109-a316d880-ddb3-11eb-802a-97ed3c4e0ea7.png)


### Deployment
**NOTE** `This code is for development - actual production code will not print to serial port or use potentiometer.`
- Connect hardware following sche,atic and pin in the code.
- Compile and flash [HUD_10LED_RPM.ino](./HUD_10LED_RPM/HUD_10LED_RPM.ino)

### Testing procedure
Upload sketch, connect all wires, connect rotary pot.
1. Turn switch on/off to confirm that works.
2. Use pot to check each each LED is coloured as expected

### To-Do
- update code from C style to C++ class

### Supplementary Information

##### How to tune/pick colours: 
- Deploy [neopixel_colour_tuning.ino](./neopixel_colour_tuning/neopixel_colour_tuning.ino)
- Enter the R,G,B values [0-255] as promted to see the output of those channels combined.

##### NEOPIXEL Best Practices for most reliable operation:
```
- Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
- MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
- NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
- AVOID connecting Neostrip on a LIVE CIRCUIT. If you must, ALWAYS
  connect GROUND (-) first, then +, then data.
```
