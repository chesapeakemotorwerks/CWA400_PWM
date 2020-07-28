/*
 * This is s simple PWM controller for the CWA 400 Electric Water Pump. 
 * Please note PWM output is inverted.
 * 
 * Written by RJ Nunnally 2019. Debounce & OLED driver code are not mine, see notices in comments.
 * I am not responsible for any damage as a result of using this code!
 */

//OLED Driver
 /*********************************************************************
Original sourse: https://github.com/adafruit/Adafruit_SSD1306
This is an example for our Monochrome OLEDs based on SSD1306 drivers
This code and video explains and shows you how to use SSD1306 OLED with 128x32 pixel Display.
You will see custom code for the display to display text and live values such as voltage, temperature, pressure etc.

This example is for a 128x32 size display using I2C 

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

 * Watch the video for this code to learn it fully.
  * Watch the video here: https://youtu.be/RjyulqVsz2o
 * this code is offered "as is" without any warranty.
 *  Updated by Ahmad S. For Robojax.com video tutorial
 * on March 18, 2018 at 10:21 at Ajax, Ontario, Canada
 * Please view other Robojax codes and videos at http://robojax.com/learn/arduino
 * if you are sharing this code, you must keep this copyright note.
 * 
*********************************************************************/

//debounce code from https://github.com/mathertel/RotaryEncoder
/*
 * See http://www.mathertel.de/License.aspx

Software License Agreement (BSD License)

Copyright (c) 2005-2014 by Matthias Hertel,  http://www.mathertel.de/

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
•Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. 
•Neither the name of the copyright owners nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//rotary encoder

const uint8_t encoderKnobPinA = 2;
const uint8_t encoderKnobPinB = 3;
const uint8_t LATCHSTATE = 3;
int8_t oldState = 3;
const int8_t KNOBDIR[] = {
  0, -1,  1,  0,
  1,  0,  0, -1,
 -1,  0,  0,  1,
  0,  1, -1,  0};
int positionInternal = 0;
int positionExternal = 0;
int oldPositionExternal = 0;
//------------------------

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
double count=0;
double DC = 25;

/*
 * PIN connection:
 * pin connection see: https://www.arduino.cc/en/Reference/Wire
 * for UNO: SDA to A4, SCL to A5
 * for Mega2560: SDA to 20, SCL to 21
 * for Leonardo: SDA to 2, SCL to 3
 * for Due: SDA to 20, SCL to 21
 * VCC to 5V
 * GND to GND :-)
 */


// this is the Width and Height of Display which is 128 xy 32 
#define LOGO16_GLCD_HEIGHT 32
#define LOGO16_GLCD_WIDTH  128 


#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {
  pinMode(encoderKnobPinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(encoderKnobPinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(1,tick,CHANGE); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(0,tick,CHANGE); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  
                   
  Serial.begin(9600);
  

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(2000);
   // Clear the buffer.
  display.clearDisplay(); 
  analogWrite(5, 255-(255*(DC/100)));
}
void tick(){
  int sigA = digitalRead(encoderKnobPinA);
  int sigB = digitalRead(encoderKnobPinB);
  int8_t thisState = sigA | (sigB << 1);
  if (oldState != thisState) {
    positionInternal += KNOBDIR[thisState | (oldState<<2)];
    if (thisState == LATCHSTATE)
      positionExternal = positionInternal >> 2;   
    oldState = thisState;
  }
}

void loop() {
  
   
   if(oldPositionExternal != positionExternal) {
    DC = DC - (oldPositionExternal - positionExternal);
    if(DC > 100){
      DC = 100;
    }else if(DC < 0){
     DC = 0; 
    }
    analogWrite(5, 255-(255*(DC/100)));
     
    oldPositionExternal = positionExternal;
   }
   // Serial.println(encoderPos);
    
    
  String vString =  String(DC, 0) + "%";// using a float and the 
  String vString2 = String(positionExternal);
  if (DC <= 1){
    vString2 = "STP";
  } else if (DC <= 8){
    vString2 = "EMR";
  } else if (DC <=12){
    vString2 = "RST";
  } else if (DC <= 85){
    double Speed = DC - 13;
    Speed = (Speed/73) * 100;
    vString2 = String(Speed, 0) + "%";
  } else if (DC >= 86 && DC <=97){
    vString2 = "MAX";
  } else if(DC >=98){
    vString2 = "EMR";
  }
 
  display.clearDisplay();
  robojaxText("Duty :", 0, 0, 2, false);
  if (DC!=100){
  robojaxText(vString, 92, 0, 2, false);
  } else {
    robojaxText(vString, 80, 0, 2, false);
  }
  //robojaxText("%", 110, 2, 1, false);
  robojaxText("Speed: ", 0, 18, 2, false);
  robojaxText(vString2, 92, 18, 2, false);
  display.display();
//  oldEncPos = encoderPos;
  // }
  //delay(50);
 // }
 
   
 
}


/*
 * robojaxText(String text, int x, int y,int size, boolean d)
 * text is the text string to be printed
 * x is the integer x position of text
 * y is the integer y position of text
 * z is the text size, 1, 2, 3 etc
 * d is either "true" or "false". Not sure, use true
 */
void robojaxText(String text, int x, int y,int size, boolean d) {

  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x,y);
  display.println(text);
  if(d){
    display.display();
  }
  
  //delay(100);
}
