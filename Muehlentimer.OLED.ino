
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>                 //includes the EEPROM library


#define OLED_RESET 4 // not used / nicht genutzt bei diesem Display
Adafruit_SSD1306 display(OLED_RESET);

const int optoOne = 12;   //specifies pin number for optocoupler 1
int RCLpin = 11;           // GPIO for  Radio PowerPlug

int presetOneVal = 0;   //initialises value for preset one hold time.
int presetTwoVal = 0;   //initialises value for preset two hold time. 

int upState = 0;         //
int downState = 0;       //
int selectState = 0;     //
int oldSelectState = 0;  //
int activeState = 0;     //
int oldActiveState = 0;  //
int purgeState = 0;      //
int lcdState = 0;        //
int oldlcdState = 0;     //More initialisation


/*-------- for countdown on the LCD-------------*/
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long pOneMillis = 0;
unsigned long pOneMillisLength = 0;
unsigned long pTwoMillis = 0;
unsigned long pTwoMillisLength = 0;
int dig1 = 0;
int dig2 = 0;
float countDown = 0;

int x = 0;

// define some values used by the panel and buttons
int lcd_key = 0;
int adc_key_in = 0;

void setup(){

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C / mit I2C-Adresse 0x3c initialisieren

  pinMode(optoOne, OUTPUT);          //specifies the optocoupler 1 pin as an output
  digitalWrite(optoOne, LOW);
  
 
display.clearDisplay();
  
  // set text color / Textfarbe setzen
  display.setTextColor(WHITE);
  // set text size / Textgroesse setzen
  display.setTextSize(1);
  // set text cursor position / Textstartposition einstellen
  display.setCursor(1,0);
  // show text / Text anzeigen
  display.println("GrinderTimer");
  display.setTextSize(1);
  display.setCursor(0,10);
  display.println("for Espresso");
  display.display();
  delay(5000);          // delay for dramatic effect!
  display.clearDisplay();          // clear lcd
  display.display();
  sniffPresets();                       // read the preset values saved in the eeprom
  presetOneLCD();                       // display the preset 1 settings

  pinMode(RCLpin, OUTPUT);          // specifies OUTPUT Pin for Radio PowerPlug
  
}

void loop() {

  buttonCheck();           //check the state of the buttons
  currentMillis = millis();//current millis becomes millis (for countdown purposes)
  presetFunctions();       //run the main code (functions for presets and grinder motor on)
}

  /*--------------check button positions and write values pertaining to their states ------------------*/
void buttonCheck(){
 
adc_key_in = analogRead(0); // read the value from the sensor
 
  if(adc_key_in < 50){         //when the active is pressed
  activeState = 1;                                                            
  purgeState = 0;                                                          
  } else if(adc_key_in < 195){  //when the up button is pressed
  upState = 1;                                                            
  downState = 0;                                                            
  } else if (adc_key_in < 380) { //when the down button ist pressed
   upState = 0;
   downState = 1;
   } else if (adc_key_in < 555) { //when the purge button is pressed  
     activeState = 0;
     purgeState = 1;
     } else if(adc_key_in < 790){  //when the select button is pressed                                                    
  delay(200);                                                             //wait 0.2 seconds (debounce)
  selectState = 1 - selectState;                                          //the selectState changes state from 0 to 1 or 1 to 0.
   } 
   else {                                                             //in any other conditions
     downState = 0;                                                     //
     upState = 0;                                                       //
     activeState = 0;                                                   //
     purgeState = 0;                                                    //the up, down, active and purge states are 0
   }
}


/*-------- displays the preset 1 information -----------------*/
void presetOneLCD() {
  display.clearDisplay();                     //clears previous info on LCD screen
  display.setCursor(1,0);              //sets cursor to column 4, row 0.
  display.print("Espresso :1shot");           //prints "preset 1"
  display.setCursor(0,10);              //sets cursor to column 0, row 1.
  display.print("Mahldauer:");        //prints "duration = "
  display.print(presetOneVal/10.0);      //prints the preset one time value.
  display.print("s");
  display.display();
}


/*--------- creates a function to display the setting of preset 1--------------*/
void presetOneSelect()  {         
  display.clearDisplay();                     //clears previous info on LCD screen
  display.setCursor(1,0);              //sets cursor to column 2, row 0.
  display.print("SET Espresso");       //prints "set preset 1"
  display.setCursor(0,10);              //sets cursor to column 0, row 1.
  display.print("Mahldauer:");        //prints "duration = "
  display.print(presetOneVal/10.0);    //prints the preset one time value.
  display.print("s");
  display.display();

}



/*-------- displays the preset 2 information -----------------*/ 
void presetTwoLCD() {
  display.clearDisplay();
  display.setCursor(1,0);
  display.print("Doppio   :2shots");
  display.setCursor(0,10);
  display.print("Mahldauer:");
  display.print(presetTwoVal/10.0);
  display.print("s");
  display.display();

}
/*--------- creates a function to display the setting of preset 2--------------*/
void presetTwoSelect()  {         
    display.clearDisplay();                     //clears previous info on LCD screen
  display.setCursor(1,0);              //sets cursor to column 2, row 0.
  display.print("SET Doppio");       //prints "set preset 1"
  display.setCursor(0,10);              //sets cursor to column 0, row 1.
  display.print("Mahldauer:");        //prints "duration = "
  display.print(presetTwoVal/10.0);    //prints the preset one time value.
  display.print("s");
    display.display();

}
/*---------------creates a function to display information on preset 1 and 2 while up and down are pressed --------------*/

void presetFunctions(){
  if ((upState == 1) && (selectState == 0)) {                         //when up is pressed and select is not engaged then  
    lcdState = 0;                                                     //lcdState is 1.
  }                                                                   //
  else if((downState == 1) && (selectState == 0)) {                   //when down is pressed and select is not egnaged then
    lcdState = 1;                                                     //lcdState is 0.
  }
  
  if ((lcdState == 0) && (oldlcdState == 1)) {                        //when down has been pressed once and select has not been pressed then 
    presetOneLCD();                                                   //show preset 1 information on the display.
    oldlcdState = 0;                                                  //make old lcd state 0 so the display is not repeated.
  }
  else if((lcdState == 1) && (oldlcdState ==0)) {                     //when up has been pressed once and select has not been pressed then 
    presetTwoLCD();                                                   //show preset 2 information on the lcd
    oldlcdState = 1;                                                  //make old lcd state 1 so the display is not repeated.
  }
  

    
    
/*---------------------- for SELECT stage on preset 1-----------------------*/    
  while((lcdState == 0) && (selectState ==1)) {                       //when lcd State is 0 (showing preset 1 info) and select has been engaged then
    presetOneSelect();                                                //display setting preset one info on LCD
    delay(100);                                                       //delay 0.1 seconds to decrease amount flashed to LCD
    oldlcdState = 1;                                                  //make oldlcdstate = 1 so that it will return to display preset 1 info once select is pressed
    buttonCheck();                                                    //check what buttons have been pressed (to know if select has been pressed so it can turn off, or if up or down have been pressed
    if(upState == 1) {                                                //if up button is pressed then
    presetOneVal = presetOneVal + 1;                                //add 0.1 seconds to the preset 1 time
    presetOneVal = constrain(presetOneVal, 1, 255);                //limit the value range to be 0.1 to 25.5
    }
    else if (downState == 1) {                                        //if the down button is pressed then
      presetOneVal = presetOneVal - 1;                              //minus 0.1 seconds to the preset 1 time
      presetOneVal = constrain(presetOneVal, 1, 255);              //limit the value range to be 0.1 to 25.5
    }
    oldSelectState = 1;                                               //make oldSelectState = 1 so we can write the value to memory later.
  }
/*---------------------- for SELECT stage on preset 2-----------------------*/  
   while((lcdState == 1) && (selectState ==1)) {                       //when lcd State is 1 (showing preset 2 info) and select has been engaged then
    presetTwoSelect();                                                //display setting preset 2 info on LCD
    delay(100);                                                       //delay 0.1 seconds to decrease amount flashed to LCD
    oldlcdState = 0;                                                  //make oldlcdstate = 0 so that it will return to display preset 1 info once select button is pressed
    buttonCheck();                                                    //check what buttons have been pressed (to know if select has been pressed so it can turn off, or if up or down have been pressed
    if(upState == 1) {                                                //if up button is pressed then
    presetTwoVal = presetTwoVal + 1;                                //add 0.1 seconds to the preset 2 time
    presetTwoVal = constrain(presetTwoVal, 1, 255);                //limit the value range to be 0.1 to 25.5
    }
    else if (downState == 1) {                                        //if the down button is pressed then
      presetTwoVal = presetTwoVal - 1;                              //minus 0.1 seconds to the preset 2 time
      presetTwoVal = constrain(presetTwoVal, 1, 255);              //limit the value range to be 0.1 to 25.5
    }
    oldSelectState = 1;                                               //make oldSelectState = 1 so we can write the value to memory later.
  }
    if ((lcdState == 0) && (selectState == 0) && (oldSelectState ==1)) {  //when preset 1 info is displayed, after select has changed from state 1 to state 0 then
  oldSelectState = 0;                                                     //old select state changes to 0 (so it doesn't repeat)
  spitPresetOne();                                                        //write the preset one value to the eeprom memory
  }
  else if ((lcdState == 1) && (selectState == 0) && (oldSelectState ==1)) {  //when preset 2 info is displayed, after select has changed from state 1 to state 0 then
  oldSelectState = 0;                                                        //old select state changes to 0 (so it doesn't repeat)
  spitPresetTwo();                                                           //write the preset two value to the eeprom memory
  }
  
  
  
  
/*---------------------- ACTIVE BUTTON FUNCTIONS---------------------*/  
  while((lcdState == 0) && (activeState == 1)){                   // when preset 1 is shown and the active button is pressed
    while(x < 1) {                                                // sets up a single use function
     for(x = 0; x < 1; x++) {                                     // do the next 5 lines, once
//       display.clearDisplay();                                               // clear the LCD 
//       display.setCursor(1,0);                                        // set the cursor to the top left
//       display.print("Mahle Espresso");                              // write "preset 1 active"
//       display.setCursor(0,10);                                        // set the cursor to the bottom left
//       display.print("Verbleibend:");                                 // write "remaining =" 
     } 
    }
    previousMillis = currentMillis;                               // save the the millis() as previousMillis as the button has been pressed
    pOneMillis = (presetOneVal*100);                              // convert presetOneVal from its EEPROM friendly form into a milliSecond friendly value
    pOneMillisLength = (previousMillis + pOneMillis);             // create the milliSecond value that this while loop should stop at
    countDown = (pOneMillisLength - millis());                    // countDown is the difference between the actual millis() value and pOneMillisLength
    dig1 = countDown/1000;                                        // dig1 is the first digit of countdown
    dig2 = ((countDown-(dig1*1000))/100);                         // dig2 is the second digit of countdown
//    display.setCursor(12,1);                                          // move the lcd Cursor to 12,1
//    display.print(dig1);                                              // print the value of dig1
//    display.print(".");                                               // print a decimal point
//    display.print(dig2);                                              // print the value of dig2
    
    if(millis() < pOneMillisLength) {                             // when millis is less than the milliSecond value that this while loop should stop at
      RCLswitch(0b100000000110);
      delay(20);
      digitalWrite(optoOne, HIGH);                                //
    } else {                                                      // when millis is greater that the millisecond value that this while loop should stop at 
      digitalWrite(optoOne, LOW);                                 //
      RCLswitch(0b100000000101);
      delay(20);
      presetOneLCD();                                             // return to the preset one display
      activeState = 0;                                            // make the activeState = 0 so the while loop stops
      x = 0;                                                      // return x to 0 so that the above "for" loop can be repeated
    }
  }
  
  while((lcdState == 1) && (activeState == 1)){                   // when preset 2 is shown and the active button is pressed
    while(x < 1) {                                                // sets up a single use function
     for(x = 0; x < 1; x++) {                                     // do the next 5 lines, once
       display.clearDisplay();                                               // clear the LCD 
       display.setCursor(1,0);                                        // set the cursor to the top left
       display.print("Mahle Doppio");                              // write "preset 2 active"
       display.setCursor(0,10);                                        // set the cursor to the bottom left
       display.print("Verbleibend:");                                 // write "remaining =" 
     } 
    }
    previousMillis = currentMillis;                               // save the the millis() as previousMillis as the button has been pressed
    pTwoMillis = (presetTwoVal*100);                              // convert presetTwoVal from its EEPROM friendly form into a milliSecond friendly value
    pTwoMillisLength = (previousMillis + pTwoMillis);             // create the milliSecond value that this while loop should stop at
    countDown = (pTwoMillisLength - millis());                    // countDown is the difference between the actual millis() value and pTwoMillisLength
    dig1 = countDown/1000;                                        // dig1 is the first digit of countdown
    dig2 = ((countDown-(dig1*1000))/100);                         // dig2 is the second digit of countdown
 //   display.setCursor(12,1);                                          // move the lcd Cursor to 12,1
//    display.print(dig1);                                              // print the value of dig1
//    display.print(".");                                               // print a decimal point
//    display.print(dig2);                                              // print the value of dig2
    
    if(millis() < pTwoMillisLength) {                             // when millis is less than the milliSecond value that this while loop should stop at
      RCLswitch(0b100000000110);
      delay(20);
      digitalWrite(optoOne, HIGH);                                //
    } else {                                                      // when millis is greater that the millisecond value that this while loop should stop at 
      RCLswitch(0b100000000101);
      delay(20);
      digitalWrite(optoOne, LOW);                                 //
      presetTwoLCD();                                             // return to the preset one display
      activeState = 0;                                            // make the activeState = 0 so the while loop stops
      x = 0;                                                      // return x to 0 so that the above "for" loop can be repeated
    }
  }

  
  while(purgeState == 1) {
    digitalWrite(optoOne, HIGH);
    RCLswitch(0b100000000110);
    delay(20);
    buttonCheck();
    if(purgeState == 0) {
      digitalWrite(optoOne, LOW);
      RCLswitch(0b100000000101);
      delay(20);
  }
  
  
}

}



/*--------------Function to read preset values from EEPROM (needs only be done once during startup) ------------------*/

void sniffPresets() {  
  presetOneVal = EEPROM.read(0);     // read the preset one integer value from EEPROM
  presetTwoVal = EEPROM.read(1);     // read the preset two integer value from EEPROM
}


/*--------------Functions to write the preset values to the EEPROM (needs to be done ONCE after each change is made to the value)-----------*/

void spitPresetOne() {
  EEPROM.write(0,presetOneVal);         //write the integer into the eeprom
}

void spitPresetTwo() {
  EEPROM.write(1,presetTwoVal);         //write the integer into the eeprom 
}

/*--------------Functions to make use of the radio switched power plug-----------*/

void RCLswitch(uint16_t code) {
    for (int nRepeat=0; nRepeat<6; nRepeat++) {
        for (int i=4; i<16; i++) {
            RCLtransmit(1,3);
            if (((code << (i-4)) & 2048) > 0) {
                RCLtransmit(1,3);
            } else {
                RCLtransmit(3,1);
            }
        }
        RCLtransmit(1,31);    
    }
}

void RCLtransmit(int nHighPulses, int nLowPulses) {
    digitalWrite(RCLpin, HIGH);
    delayMicroseconds( 350 * nHighPulses);
    digitalWrite(RCLpin, LOW);
    delayMicroseconds( 350 * nLowPulses);
}
