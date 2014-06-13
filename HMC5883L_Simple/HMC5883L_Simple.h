/**
 * Simple HMC5883L Compass Library
 *
 * Implements a simple interface to get a Heading in Degrees out of a 
 * HMC5883L based compass module.
 * 
 * Datasheet: http://goo.gl/w1criV
 * 
 * Copyright (C) 2014 James Sleeman
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 * 
 * @author James Sleeman, http://sparks.gogo.co.nz/
 * @license MIT License
 */

// Please note, the Arduino IDE is a bit retarded, if the below define has an
// underscore other than _h, it goes mental.  Wish it wouldn't  mess
// wif ma files!
#ifndef HMC5883LSimple_h
#define HMC5883LSimple_h

/* NOTE:
 * There are both 7- and 8-bit versions of I2C addresses.
 * 7 bits identify the device, and the eighth bit determines
 * if it's being written to or read from. The Wire library uses
 * 7 bit addresses throughout. If you have a datasheet or sample code 
 * that uses 8 bit address, you'll want to drop the low bit
 * (i.e. shift the value one bit to the right), yielding an 
 * address between 0 and 127.
 * 
 * The HMC datasheet says...
 * " The default (factory) HMC5883L 7-bit slave address is 0x3C for write operations, or 0x3D for read operations. "
 * which is of course silly, what they mean is that these are 8=bit addresses and thus we want to shift right 
 * one of them to get the 7-bit address that Wire wants
 * 
 * 0x3C = 111100
 * >> 1 = 11110
 *      = 0x1E
 * 
 * 0x3D = 111101
 * >> 1 = 11110
 *      = 0x1E
 *   
 */

#define COMPASS_I2C_ADDRESS  0x3C >> 1

#define COMPASS_CONFIG_REGISTER_A 0x00
#define COMPASS_CONFIG_REGISTER_B 0x01
#define COMPASS_MODE_REGISTER     0x02
#define COMPASS_DATA_REGISTER     0x03

// We use 16 bits for storing various configs
//  xxxxxxxxxxxxxxMM
//  MODE:

#define COMPASS_CONTINUOUS 0x00
#define COMPASS_SINGLE     0x01
#define COMPASS_IDLE       0x02

//  xxxxxxxxxxxSSSxx
//  SCALE:
//   A lower value indicates a higher precision
//   but "noisier", magentic noise may necessitate
//   you to choose a higher scale.

#define COMPASS_SCALE_088  0x00 << 2
#define COMPASS_SCALE_130  0x01 << 2
#define COMPASS_SCALE_190  0x02 << 2
#define COMPASS_SCALE_250  0x03 << 2
#define COMPASS_SCALE_400  0x04 << 2
#define COMPASS_SCALE_470  0x05 << 2
#define COMPASS_SCALE_560  0x06 << 2
#define COMPASS_SCALE_810  0x07 << 2

//  xxXXXYYYZZZxxxxx
//  ORIENTATION: 
#define COMPASS_NORTH 0x00 
#define COMPASS_SOUTH 0x01
#define COMPASS_WEST  0x02
#define COMPASS_EAST  0x03
#define COMPASS_UP    0x04
#define COMPASS_DOWN  0x05

// When "pointing" north, define the direction of each of the silkscreen'd arrows
// (imagine the Z arrow points out of the top of the device) only N/S/E/W are allowed
#define COMPASS_HORIZONTAL_X_NORTH  ( (COMPASS_NORTH << 6)  | (COMPASS_WEST  << 3)  | COMPASS_UP    ) << 5
#define COMPASS_HORIZONTAL_Y_NORTH  ( (COMPASS_EAST  << 6)  | (COMPASS_NORTH << 3)  | COMPASS_UP    ) << 5
#define COMPASS_VERTICAL_X_EAST     ( (COMPASS_EAST  << 6)  | (COMPASS_UP    << 3)  | COMPASS_SOUTH ) << 5
#define COMPASS_VERTICAL_Y_WEST     ( (COMPASS_UP    << 6)  | (COMPASS_WEST  << 3)  | COMPASS_SOUTH ) << 5

class HMC5883L_Simple
{
	public:
        
    // Constructor
	  HMC5883L_Simple();
  
    // Configuration Methods
    void SetScale( uint16_t sampling_mode );
    void SetOrientation( uint16_t sampling_mode );    
    void SetDeclination( int declination_degs , int declination_mins, char declination_dir );
    void SetSamplingMode( uint16_t sampling_mode );
    	  
    // Get a heading in degrees
    float GetHeadingDegrees();
    
    
	protected:
	  void     Write(uint8_t address, uint8_t byte);
    uint8_t  Read(uint8_t register_address, uint8_t buffer[], uint8_t length);
    
    struct MagnetometerSample
    {
      int X;
      int Y;
      int Z;
    };
    
    MagnetometerSample ReadAxes();
        
    uint16_t  mode;
    float    declination_offset_radians;
    
  private:
    uint8_t  i2c_address;
};
#endif