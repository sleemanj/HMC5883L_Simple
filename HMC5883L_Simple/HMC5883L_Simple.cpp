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

#include <Arduino.h>

// PLEASE NOTE!
// The Arduino IDE is a bit braindead, even though we include Wire.h here, it does nothing
// you must include Wire.h in your main sketch, the Arduino IDE will not include Wire
// in the build process otherwise.
#include <Wire.h>
#include "HMC5883L_Simple.h"

HMC5883L_Simple::HMC5883L_Simple()
{
  declination_offset_radians = 0;
  mode = COMPASS_SINGLE | COMPASS_SCALE_130 | COMPASS_HORIZONTAL_X_NORTH;  
  i2c_address = COMPASS_I2C_ADDRESS;  // NB: The HMC5883L does not appear to be able to have any different address.
                                      //     so this is a bit moot.                                      
}

/** Set declination in degrees, minutes and direction (E/W)
 *   See http://www.magnetic-declination.com/
 */

void HMC5883L_Simple::SetDeclination( int declination_degs , int declination_mins, char declination_dir )
{    
  // Convert declination to decimal degrees
  switch(declination_dir)
  {
    // North and East are positive   
    case 'E': 
      declination_offset_radians = ( declination_degs + (1/60 * declination_mins)) * (M_PI / 180);
      break;
      
    // South and West are negative    
    case 'W':
      declination_offset_radians =  0 - (( declination_degs + (1/60 * declination_mins) ) * (M_PI / 180));
      break;
  } 
}

/** Set the sampling mode to one of COMPASS_CONTINUOUS or COMPASS_SINGLE
 */

void HMC5883L_Simple::SetSamplingMode( uint16_t sampling_mode )
{  
  // Mode is the bits marked M in mode
  //    xxxxxxxxxxxSSSMM
  mode = (mode & ~0x03) | (sampling_mode & 0x03);

  Write(COMPASS_MODE_REGISTER, mode & 0x03);  
}

/** Set the scale to one of COMPASS_SCALE_088 through COMPASS_SCALE_810
 * Higher scales are less sensitive and less noisy
 * Lower scales are more sensitive and more noisy
 */

void HMC5883L_Simple::SetScale( uint16_t scale )
{
  // Scale is the bits marked S in mode
  //    xxxxxxxxxxxSSSMM  
  mode = (mode & ~0x1C) | (scale & 0x1C);

  Write(COMPASS_CONFIG_REGISTER_B, (( mode >> 2 ) & 0x07) << 5);
}

/** Set the orientation to one of COMPASS_HORIZONTAL_X_NORTH 
 * through COMPASS_VERTICAL_Y_WEST
 *  
 */

void HMC5883L_Simple::SetOrientation( uint16_t orientation )
{
  // Orientation is the bits marked XXXYYYZZZ in mode
  //    xxXXXYYYZZZxxxxx
  mode = (mode & ~0x3FE0) | (orientation & 0x3FE0);    
}

/** Get the heading of the compass in degrees. */
float HMC5883L_Simple::GetHeadingDegrees()
{     
  // Obtain a sample of the magnetic axes
  MagnetometerSample sample = ReadAxes();
  
  float heading;    
  
  // Determine which of the Axes to use for North and West (when compass is "pointing" north)
  float mag_north, mag_west;
   
  // Z = bits 0-2
  switch((mode >> 5) & 0x07 )
  {
    case COMPASS_NORTH: mag_north = sample.Z; break;
    case COMPASS_SOUTH: mag_north = 0-sample.Z; break;
    case COMPASS_WEST:  mag_west  = sample.Z; break;
    case COMPASS_EAST:  mag_west  = 0-sample.Z; break;
      
    // Don't care
    case COMPASS_UP:
    case COMPASS_DOWN:
     break;
  }
  
  // Y = bits 3 - 5
  switch(((mode >> 5) >> 3) & 0x07 )
  {
    case COMPASS_NORTH: mag_north = sample.Y;  break;
    case COMPASS_SOUTH: mag_north = 0-sample.Y; ;  break;
    case COMPASS_WEST:  mag_west  = sample.Y;  break;
    case COMPASS_EAST:  mag_west  = 0-sample.Y;  break;
      
    // Don't care
    case COMPASS_UP:
    case COMPASS_DOWN:
     break;
  }
  
  // X = bits 6 - 8
  switch(((mode >> 5) >> 6) & 0x07 )
  {
    case COMPASS_NORTH: mag_north = sample.X; break;
    case COMPASS_SOUTH: mag_north = 0-sample.X; break;
    case COMPASS_WEST:  mag_west  = sample.X; break;
    case COMPASS_EAST:  mag_west  = 0-sample.X; break;
      
    // Don't care
    case COMPASS_UP:
    case COMPASS_DOWN:
     break;
  }
    
  // calculate heading from the north and west magnetic axes
  heading = atan2(mag_west, mag_north);
  
  // Adjust the heading by the declination
  heading += declination_offset_radians;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*M_PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*M_PI)
    heading -= 2*M_PI;
   
  // Convert radians to degrees for readability.
  return heading * 180/M_PI; 
}


/** Read the axes from the magnetometer.
 * In SINGLE mode we take a sample.  In CONTINUOUS mode we 
 * just grab the most recent result in the registers.
 */

HMC5883L_Simple::MagnetometerSample HMC5883L_Simple::ReadAxes()
{
  if(mode & COMPASS_SINGLE) 
  {    
    Write(COMPASS_MODE_REGISTER, (uint8_t)( mode & 0x03 ));  
    delay(66); // We could listen to the data ready pin instead of waiting.
  }
  
  uint8_t buffer[6];
  Read(COMPASS_DATA_REGISTER, buffer, 6);

  MagnetometerSample sample;
  
  // NOTE:
  // The registers are in the order X Z Y  (page 11 of datasheet)
  // the datasheet when it describes the registers details then in order X Y Z (page 15)
  // stupid datasheet writers
  sample.X = (buffer[0] << 8) | buffer[1];  
  sample.Z = (buffer[2] << 8) | buffer[3];
  sample.Y = (buffer[4] << 8) | buffer[5];
  
  return sample;
}

/** Write data to the compass by I2C */
void HMC5883L_Simple::Write(uint8_t register_address, uint8_t data)
{
  Wire.beginTransmission(i2c_address);
  Wire.write(register_address);
  Wire.write(data);
  Wire.endTransmission();
}

/** Read data from the compass by I2C  
 */
uint8_t HMC5883L_Simple::Read(uint8_t register_address, uint8_t buffer[], uint8_t length)
{
  // Write the register address that we will begin the read from, this
  // has the effect of "seeking" to that register
  Wire.beginTransmission(i2c_address);
  Wire.write(register_address);
  Wire.endTransmission();
  
  // Read the data starting at that register we seeked
  Wire.requestFrom(i2c_address, length);

  if(Wire.available() == length)
  {
    for(uint8_t i = 0; i < length; i++)
    {
      buffer[i] = Wire.read();
    }
    
    return length;
  }

  return 0;
}