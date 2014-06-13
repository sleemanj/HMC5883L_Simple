HMC5883L_Simple
===============

Simple to use Arduino library to interface to HMC5883L Magnetometer (Digital Compass)

This library makes getting a compass heading from an HMC5883L Magnetometer very simple, it includes adjustment for your local Magnetic Declination which can be entered directly in degrees minutes and direction, the library handles all the dirty work.

In it's most simple form, here's the code to get a heading!

    HMC5883L_Simple Compass;
    Compass.SetDeclination(23, 35, 'E');  
    float heading = Compass.GetHeadingDegrees();
  
Can't get much easier than that.  You can obtain your declination (specific to your geographic location) from http://www.magnetic-declination.com/ and once you have entered that, point the X axis arrow where you want to go.

I wrote this library because the existing HMC5883L libraries make it too complicated, and some are just out-right badly written with all sorts of bugs.

Download, Install and Example
-----------------------------

* Download: http://sparks.gogo.co.nz/HMC5883L_Simple.zip
* Open the Arduino IDE (1.0.5)
* Select the menu item Sketch > Import Library > Add Library
* Choose to install the HMC5883L_Simple.zip file you downloaded
* Now you can choose File > Examples > HMC5883L_Simple > Compass


Connecting The GY-273 To Your Arduino
-------------------------------------
This library was developed to be used with the GY-273 Compass board which includes a 3v3 regulator and 4k7 pullups to the 3v3 output, so you can safely use the GY-273 with your 5v Arduino (or 3v3 Arduino).

Below are the connections for a typical Arduino.

 * GY-273 Compass Module  ->  Arduino
 * VCC  -> VCC  (See Note Below)
 * GND  -> GND
 * SCL  -> A5/SCL, (Use Pin 21 on the Arduino Mega)
 * SDA  -> A4/SDA, (Use Pin 20 on the Arduino Mega)
 * DRDY -> Not Connected (in this example)

  
GY-273 Compass Board Orientation
--------------------------------

Options are available to set the orientation of the board depending on how you have mounted it, horizontal, vertical, short and long edge-ways.  The other libraries make you figure that out yourself, it made my head hurt doing it, so I made it easy for you.

The default is that the board will be horizontal (with respect to the surface of the earth) and will give a heading of 0 degrees when the silkscreeen'd arrow of the X axis points North.

Tilt Compensation
-----------------

None.  No attempt to compensate for tilt is made.  AFAIK, doing so requires an accellerometer, and maths well above what I am capable of.  Maybe somebody will be kind enough to add support for tilt compensation, without making the library complicated!

This means in simple terms, two axis of the board must be parallel to the earth surface (and you must know which two).

Accuracy/Correctness
--------------------

Maybe.  I'm not a mathematical person, nor am I a geographical person, I don't even have a real compass to compare with, but the library works for me, sitting here in my office it produces what appear to be largely correct headings, I think I've got the calculations correct, but if I don't, please fork it, fix it, and submit it back to me.  Don't complain to me if your uber expensive quad copter flies off into the night on a heading which will end it up on a small island in the middle of the pacific.
