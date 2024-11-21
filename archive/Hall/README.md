# HALL
This is a readme for the Hall effect sensor.
<p align="center">
<img src="https://user-images.githubusercontent.com/62113118/128959811-d2c72415-9c3e-4705-9301-e6f85e10d6a2.png" alt="Hall" width="50%" align="center">
</p>

## Parts List
* Arduino supported microcontroller
* [Hall Sensor](https://www.littelfuse.com/~/media/electronics/datasheets/hall_effect_sensors/littelfuse_hall_effect_sensors_55100_datasheet.pdf.pdf) (model used is Littlefuse 55100 3H04A 2025)
* Magnet
* Pull-up Resistor (250+ Ohms)
* Ceramic capacitor (100nF, optional)

## Wiring

Refer to the [Hall sensor datasheet](https://www.littelfuse.com/~/media/electronics/datasheets/hall_effect_sensors/littelfuse_hall_effect_sensors_55100_datasheet.pdf.pdf) for information on wiring.

The three wire version of the Hall sensor is used in this example.
Red - VDD  
Black - GND  
Blue - OUT  

Wiring Instructions:
1. Connect the Hall sensor VDD and GND pins
2. Connect the Hall sensor OUT pin to the desired digital input pin on the Arduino
3. Connect the pull-up resistor from the OUT pin of the Hall sensor to VDD
4. If using a capacitor, connect it to VDD and GND close to the sensor

 Example wiring schematic:

 ![schematic](https://user-images.githubusercontent.com/62113118/128958764-50b08afe-d30d-48d9-bb46-91ff9e0d78ad.png)

## Deployment

1. Connect the hardware following the wiring schematic.
2. Compile and flash **hall_effect.ino**

## Testing Procedure

1. Move the sensor past the magnet, close enough to get a reading (within ~1cm)
2. Verify that the correct rpm is displayed in the serial monitor. See below for an example of the output

![schematic](https://user-images.githubusercontent.com/62113118/128959092-b4fdebfd-535b-459e-97fe-7fd8b8f2da1f.png)

## To-Do
* Interrupt version of hall-effect code
* Timing of interrupt version vs polling version

## Other Information
* A capacitor is not required for testing, however should be added for transient voltage suppression in noisy environments
