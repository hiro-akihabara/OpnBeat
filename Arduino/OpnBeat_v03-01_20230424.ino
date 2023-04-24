#include <EEPROM.h>
#include <TimerOne.h>
#include <ISD1700.h>
#include <LiquidCrystal_I2C.h>//library for 16x2 I2C LCD
#include <AnalogMultiButton.h>//library for reading all buttons and keys
#include <Adafruit_NeoPixel.h>//library for WS2812B full color LEDs
#define WITHOUT_BUTTON 1//without button option for ClickEncoder.h 
#include <ClickEncoder.h>//library forRotary Encorder

#define FIRST 0
#define SECOND 1
//for WS2812B full color LED
#define LED_PIN 2
#define LED_COUNT  2

//74HC4040 binary ripple counter controls to select analog switches for decide Rocs of ISD1700 
const byte analog_SW_reset = 0;
const byte analog_SW_clk = 16;
//for AnalogMultiButton
const byte BUTTONS_PIN = A7;
const byte BUTTONS_TOTAL = 12;//number of buttons and keys
const int BUTTONS_VALUES[BUTTONS_TOTAL] = {425,478,533,592,644,710,760,814,864,910,941,976};//actual ADC values with 3.3V reference
const byte pad_1 = 0;
const byte pad_2 = 1;
const byte pad_3 = 2;
const byte pad_4 = 3;
const byte pad_5 = 4;
const byte pad_6 = 5;
const byte pad_7 = 6;
const byte pad_8 = 7;
const byte button_encoder = 8;
const byte pad_back = 9;
const byte pad_play = 10;
const byte pad_rec = 11;

byte bpm = 100;//BPM, initial setting is 100
unsigned long time_now = 0;
byte Start_addr = 16;//playback start address, trimmed by rotary encoder
byte End_addr =16;//playback end address, trimmed by rotary encoder
byte Start_addr_1_ROM;//chip_1 start and end address stored in EEPROM
byte End_addr_1_ROM;
byte Start_addr_2_ROM;//chip_2 start and end address stored in EEPROM
byte End_addr_2_ROM;
byte Start_addr_3_ROM;//chip_3 start and end address stored in EEPROM
byte End_addr_3_ROM;
byte Start_addr_4_ROM;//chip_4 start and end address stored in EEPROM
byte End_addr_4_ROM;
byte Start_addr_5_ROM;//chip_5 start and end address stored in EEPROM
byte End_addr_5_ROM;
byte Start_addr_6_ROM;//chip_6 start and end address stored in EEPROM
byte End_addr_6_ROM;
byte Start_addr_7_ROM;//chip_7 start and end address stored in EEPROM
byte End_addr_7_ROM;
byte Start_addr_8_ROM;//chip_8 start and end address stored in EEPROM
byte End_addr_8_ROM;
int value;//value loads rotary encoder value
int last = 0;//last updated value of rotary encorder
byte chip_n = 1;//selected chip number of 1 to 8
byte rhythm_n = 1;//selected rhythm number of 1to8
byte track_n = 1;//selected track number of 1to8
byte menu_column = 0;
byte menu_row = 0;
byte cursor_row = 0;//cursor position on LCD
byte cursor_column = 0;//cursor position on LCD
boolean first_second = 0;//0:ryhthm 1st part (16bit), 1:ryhthm 2nd part (16bit)
byte volume[8];//volume of Chip1, Chip2, chip3....
byte volume_last[8];//previous updated volume
uint16_t pattern_eeprom = 0b0000000000000000;//load 16bit rhythm data from EEPROM
uint16_t pattern_eeprom_current = 0b0000000000000000;//rhythm data under being edited
uint16_t pattern_eeprom_last;//rhythm data last updated
byte pattern_digit;//only a digit of rhythm data for diplaying rhythm on LCD 
unsigned int eeprom_address = 0;
uint32_t rhythm_eeprom[8];//complete rhythm data which consists of patterns of 8 chips, load them from EEPROM
int key;//read ADC value to determine analog buttons input value
word SR0;//Status Resister 0 of ISD1700
uint8_t SR1;//Status Resister 1 of ISD1700
uint16_t apc = 0b0000100111100000;//initial APC resister value of ISD1700 which define operation mode of it.
uint16_t apc_ISDchipOne, apc_ISDchipTwo, apc_ISDchipThree, apc_ISDchipFour, apc_ISDchipFive, apc_ISDchipSix, apc_ISDchipSeven, apc_ISDchipEight;//APC resister values of each chips
byte track_data;//a digit of track data displayed on LCD in track edit mode
boolean value_updated = 0;//rotary encoder value, 1: updated, 0: not updated
boolean rec_status = 0;//Recording status in rhythm edit mode, 0: not recording, 1: recording
boolean play_status = 0;//Playing status in rhythm edit mode 0: pause, 1: playing
byte k;//used for counting in rhythm edit mode
byte REC_quality = 0b00000000;//this controls Rocs value and recording sound quality of chip 1 to 8, 0: Lo-RES, 1: Hi-RES

/////////////////////     custom character fonts for I2C LCD    //////////////////////////
const byte customChar_BAR[8] = {//separator used in track edit mode
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000
};

const byte customChar_vol_0[8] = {//vol.0 graphic used in level setting
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

const byte customChar_vol_1[8] = {//vol.1 graphic used in level setting
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
};

const byte customChar_vol_2[8] = {//vol.2 graphic used in level setting
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111
};

const byte customChar_vol_3[8] = {//vol.3 graphic used in level setting
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111
};

const byte customChar_vol_4[8] = {//vol.4 graphic used in level setting
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

byte customChar_vol_5[8] = {//vol.5 graphic used in level setting
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

const byte customChar_vol_6[8] = {//vol.6 graphic used in level setting
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

const byte customChar_vol_7[8] = {//vol.7 graphic used in level setting
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

const byte customChar_play_icon[8] = {//vol.8 graphic used in level setting
  0b01000,
  0b01100,
  0b01110,
  0b01111,
  0b01111,
  0b01110,
  0b01100,
  0b01000
};

///////////////     object, instance    ///////////////////////////
ClickEncoder *encoder;
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
///// ISD1700 instance  ///////////////
ISD1700 chipOne(3);//chip1, SS1 is pin 3
ISD1700 chipTwo(4);//chip2, SS2 is pin 4
ISD1700 chipThree(5);//chip3, SS3 is pin 5
ISD1700 chipFour(6);//chip4, SS4 is pin 6
ISD1700 chipFive(7);//chip5, SS5 is pin 7
ISD1700 chipSix(8);//chip6, SS6 is pin 8
ISD1700 chipSeven(9);//chip7, SS7 is pin 9
ISD1700 chipEight(10);//chip8, SS8 is pin 10
//AnalogMultiButton
AnalogMultiButton buttons(BUTTONS_PIN, BUTTONS_TOTAL, BUTTONS_VALUES, 12);//debouce = 10ms
// pass a fourth parameter to set the debounce time in milliseconds
// this defaults to 20 and can be increased if you're working with particularly bouncy buttons

//NeoPixel, Full color LEDs
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

/////////////////     declaration of functions    ////////////////////////////// 
void timerIsr() {
  encoder->service();
}

////////////////////////////////////   Functions for Recording and Playing     /////////////////////////
void powerUpISD_all(){//power up all ISD1700 device
  Serial.println("------------------");
  chipOne.pu();
  Serial.println("ChipOne PU sent..");
  while(chipOne.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipOne.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipOne.INT() == 1){
    chipOne.clr_int();
    Serial.println("ChipOne INT cleared");
  }
  
  chipTwo.pu();
  Serial.println("ChipTwo PU sent..");
  while(chipTwo.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipTwo.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipTwo.INT() == 1){
    chipTwo.clr_int();
    Serial.println("ChipTwo INT cleared");
  }

  chipThree.pu();
  Serial.println("ChipThree PU sent..");
  while(chipThree.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipThree.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipThree.INT() == 1){
    chipThree.clr_int();
    Serial.println("ChipThree INT cleared");
  }

  chipFour.pu();
  Serial.println("ChipFour PU sent..");
  while(chipFour.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipFour.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipFour.INT() == 1){
    chipFour.clr_int();
    Serial.println("ChipFour INT cleared");
  }

  chipFive.pu();
  Serial.println("ChipFive PU sent..");
  while(chipFive.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipFive.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipFive.INT() == 1){
    chipFive.clr_int();
    Serial.println("ChipFive INT cleared");
  }

  chipSix.pu();
  Serial.println("ChipSix PU sent..");
  while(chipSix.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipSix.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipSix.INT() == 1){
    chipSix.clr_int();
    Serial.println("ChipSix INT cleared");
  }

  chipSeven.pu();
  Serial.println("ChipSeven PU sent..");
  while(chipSeven.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipSeven.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipSeven.INT() == 1){
    chipSeven.clr_int();
    Serial.println("ChipSeven INT cleared");
  }
  
  chipEight.pu();
  Serial.println("ChipEight PU sent..");
  while(chipEight.CMD_ERR() == 1){
    Serial.print("CMD ERROR..,");
    chipEight.pu();
    Serial.println("PU sent again..");
  }
  delay(100);//Typ. TPUD=50ms
  Serial.println("PU accepted..");
  if(chipEight.INT() == 1){
    chipEight.clr_int();
    Serial.println("ChipEight INT cleared");
  }

  while(chipOne.RDY() != 1){
  }
  Serial.println("chipOne Ready..");
  while(chipTwo.RDY() != 1){
  }
  Serial.println("chipTwo Ready..");
  while(chipThree.RDY() != 1){
  }
  Serial.println("chipThree Ready..");
  while(chipFour.RDY() != 1){
  }
  Serial.println("chipFour Ready..");
  while(chipFive.RDY() != 1){
  }
  Serial.println("chipFive Ready..");
  while(chipSix.RDY() != 1){
  }
  Serial.println("chipSix Ready..");
  while(chipSeven.RDY() != 1){
  }
  Serial.println("chipSeven Ready..");
  while(chipEight.RDY() != 1){
  }
  Serial.println("chipEight Ready..");
}

void configISD_all(){//only first time, set APC resister value in ISD chips and write it in their Non-volatile memory
  if(chipOne.rd_apc() != 0b0000100111100000){
     while(chipOne.RDY() != 1){
     }
     chipOne.wr_apc2(apc);
     while(chipOne.RDY() != 1){
     }
     chipOne.wr_nvcfg();
     while(chipOne.RDY() != 1){
     }
  }
  if(chipTwo.rd_apc() != 0b0000100111100000){
     while(chipTwo.RDY() != 1){
     }
     chipTwo.wr_apc2(apc);
     while(chipTwo.RDY() != 1){
     }
     chipTwo.wr_nvcfg();
     while(chipTwo.RDY() != 1){
     }
  }
  if(chipThree.rd_apc() != 0b0000100111100000){
     while(chipThree.RDY() != 1){
     }
     chipThree.wr_apc2(apc);
     while(chipThree.RDY() != 1){
     }
     chipThree.wr_nvcfg();
     while(chipThree.RDY() != 1){
     }
  }
  if(chipFour.rd_apc() != 0b0000100111100000){
     while(chipFour.RDY() != 1){
     }
     chipFour.wr_apc2(apc);
     while(chipFour.RDY() != 1){
     }
     chipFour.wr_nvcfg();
     while(chipFour.RDY() != 1){
     }
  }
  if(chipFive.rd_apc() != 0b0000100111100000){
     while(chipFive.RDY() != 1){
     }
     chipFive.wr_apc2(apc);
     while(chipFive.RDY() != 1){
     }
     chipFive.wr_nvcfg();
     while(chipFive.RDY() != 1){
     }
  }
  if(chipSix.rd_apc() != 0b0000100111100000){
     while(chipSix.RDY() != 1){
     }
     chipSix.wr_apc2(apc);
     while(chipSix.RDY() != 1){
     }
     chipSix.wr_nvcfg();
     while(chipSix.RDY() != 1){
     }
  }
  if(chipSeven.rd_apc() != 0b0000100111100000){
     while(chipSeven.RDY() != 1){
     }
     chipSeven.wr_apc2(apc);
     while(chipSeven.RDY() != 1){
     }
     chipSeven.wr_nvcfg();
     while(chipSeven.RDY() != 1){
     }
  }
  if(chipEight.rd_apc() != 0b0000100111100000){
     while(chipEight.RDY() != 1){
     }
     chipEight.wr_apc2(apc);
     while(chipEight.RDY() != 1){
     }
     chipEight.wr_nvcfg();
     while(chipEight.RDY() != 1){
     }
  }
}

void showISD_apc(){//Read APC resisters value of ISD chips and show them on Serial Monitor
  apc_ISDchipOne = chipOne.rd_apc();
  Serial.println(apc_ISDchipOne, BIN);
  apc_ISDchipTwo = chipTwo.rd_apc();
  Serial.println(apc_ISDchipTwo, BIN);
  apc_ISDchipThree = chipThree.rd_apc();
  Serial.println(apc_ISDchipThree, BIN);
  apc_ISDchipFour = chipFour.rd_apc();
  Serial.println(apc_ISDchipFour, BIN);
  apc_ISDchipFive = chipFive.rd_apc();
  Serial.println(apc_ISDchipFive, BIN);
  apc_ISDchipSix = chipSix.rd_apc();
  Serial.println(apc_ISDchipSix, BIN);
  apc_ISDchipSeven = chipSeven.rd_apc();
  Serial.println(apc_ISDchipSeven, BIN);
  apc_ISDchipEight = chipEight.rd_apc();
  Serial.println(apc_ISDchipEight, BIN);
}
  
void playSection(byte chip){//Play sound segment of selected ISD chip, trimed by rotary encoder in "Trim mode" 
  switch(chip){
    case 1:
      if(chipOne.PLAY() == 0){//if ISD is idle
        chipOne.set_play(Start_addr, End_addr);
      }
      else{//if ISD is during play
        chipOne.stop();
        while(chipOne.RDY() != 1){
        }
        chipOne.set_play(Start_addr, End_addr);
      }
      break;
    case 2:
      if(chipTwo.PLAY() == 0){
        chipTwo.set_play(Start_addr, End_addr);
      }
      else{
        chipTwo.stop();
        while(chipTwo.RDY() != 1){
        }
        chipTwo.set_play(Start_addr, End_addr);
      }
      break;
    case 3:
      if(chipThree.PLAY() == 0){
        chipThree.set_play(Start_addr, End_addr);
      }
      else{
        chipThree.stop();
        while(chipThree.RDY() != 1){
        }
        chipThree.set_play(Start_addr, End_addr);
      }
      break;
    case 4:
      if(chipFour.PLAY() == 0){
        chipFour.set_play(Start_addr, End_addr);
      }
      else{
        chipFour.stop();
        while(chipFour.RDY() != 1){
        }
        chipFour.set_play(Start_addr, End_addr);
      }
      break;
    case 5:
      if(chipFive.PLAY() == 0){
        chipFive.set_play(Start_addr, End_addr);
      }
      else{
        chipFive.stop();
        while(chipFive.RDY() != 1){
        }
        chipFive.set_play(Start_addr, End_addr);
      }
      break;
    case 6:
      if(chipSix.PLAY() == 0){
        chipSix.set_play(Start_addr, End_addr);
      }
      else{
        chipSix.stop();
        while(chipSix.RDY() != 1){
        }
        chipSix.set_play(Start_addr, End_addr);
      }
      break;
    case 7:
      if(chipSeven.PLAY() == 0){
        chipSeven.set_play(Start_addr, End_addr);
      }
      else{
        chipSeven.stop();
        while(chipSeven.RDY() != 1){
        }
        chipSeven.set_play(Start_addr, End_addr);
      }
      break;
    case 8:
      if(chipEight.PLAY() == 0){
        chipEight.set_play(Start_addr, End_addr);
      }
      else{
        chipEight.stop();
        while(chipEight.RDY() != 1){
        }
        chipEight.set_play(Start_addr, End_addr);
      }
      break;
  }
}

void playROM_pad(byte chip){//Play sound of selected ISD chip by manual play in "PLAY mode", with start and end address saved in EEPROM 
    switch(chip){
      case 1:
        if(chipOne.PLAY() == 0){//if ISD is idle
          chipOne.set_play(Start_addr_1_ROM, End_addr_1_ROM);
        }
        else{//if ISD is during play
          chipOne.stop();
          while(chipOne.RDY() != 1){
          }
          chipOne.set_play(Start_addr_1_ROM, End_addr_1_ROM);
        }
        break;
      case 2:
        if(chipTwo.PLAY() == 0){
          chipTwo.set_play(Start_addr_2_ROM, End_addr_2_ROM);
        }
        else{
          chipTwo.stop();
          while(chipTwo.RDY() != 1){
          }
          chipTwo.set_play(Start_addr_2_ROM, End_addr_2_ROM);
        }
        break;
      case 3:
        if(chipThree.PLAY() == 0){
          chipThree.set_play(Start_addr_3_ROM, End_addr_3_ROM);
        }
        else{
          chipThree.stop();
          while(chipThree.RDY() != 1){
          }
          chipThree.set_play(Start_addr_3_ROM, End_addr_3_ROM);
        }
        break;
      case 4:
        if(chipFour.PLAY() == 0){
          chipFour.set_play(Start_addr_4_ROM, End_addr_4_ROM);
        }
        else{
          chipFour.stop();
          while(chipFour.RDY() != 1){
          }
          chipFour.set_play(Start_addr_4_ROM, End_addr_4_ROM);
        }
        break;
       case 5:
        if(chipFive.PLAY() == 0){
          chipFive.set_play(Start_addr_5_ROM, End_addr_5_ROM);
        }
        else{
          chipFive.stop();
          while(chipFive.RDY() != 1){
          }
          chipFive.set_play(Start_addr_5_ROM, End_addr_5_ROM);
        }
        break;
      case 6:
        if(chipSix.PLAY() == 0){
          chipSix.set_play(Start_addr_6_ROM, End_addr_6_ROM);
        }
        else{
          chipSix.stop();
          while(chipSix.RDY() != 1){
          }
          chipSix.set_play(Start_addr_6_ROM, End_addr_6_ROM);
        }
        break;
      case 7:
        if(chipSeven.PLAY() == 0){
          chipSeven.set_play(Start_addr_7_ROM, End_addr_7_ROM);
        }
        else{
          chipSeven.stop();
          while(chipSeven.RDY() != 1){
          }
          chipSeven.set_play(Start_addr_7_ROM, End_addr_7_ROM);
        }
        break;
      case 8:
        if(chipEight.PLAY() == 0){
          chipEight.set_play(Start_addr_8_ROM, End_addr_8_ROM);
        }
        else{
          chipEight.stop();
          while(chipEight.RDY() != 1){
          }
          chipEight.set_play(Start_addr_8_ROM, End_addr_8_ROM);
        }
        break;  
    }
}

void playROM(byte chip, boolean repeat){//Play sound of selected ISD chip, in "Edit Rhythm" and "Edit Track" mode, if repeat is 1: play minimum sound element, 0: normal sound element.
    switch(chip){
      case 1:
        if(repeat == 0){
          chipOne.set_play(Start_addr_1_ROM, End_addr_1_ROM);
        }
        else{
          chipOne.set_play(Start_addr_1_ROM, Start_addr_1_ROM);
        }
        break;
      case 2:
          if(repeat == 0){
            chipTwo.set_play(Start_addr_2_ROM, End_addr_2_ROM);
          }
          else{
            chipTwo.set_play(Start_addr_2_ROM, Start_addr_2_ROM);
          }
          break;
      case 3:
          if(repeat == 0){
            chipThree.set_play(Start_addr_3_ROM, End_addr_3_ROM);
          }
          else{
            chipThree.set_play(Start_addr_3_ROM, Start_addr_3_ROM);
          }
          break;
      case 4:
          if(repeat == 0){
            chipFour.set_play(Start_addr_4_ROM, End_addr_4_ROM);
          }
          else{
            chipFour.set_play(Start_addr_4_ROM, Start_addr_4_ROM);
          }
          break;
      case 5:
          if(repeat == 0){
            chipFive.set_play(Start_addr_5_ROM, End_addr_5_ROM);
          }
          else{
            chipFive.set_play(Start_addr_5_ROM, Start_addr_5_ROM);
          }
          break;
      case 6:
          if(repeat == 0){
            chipSix.set_play(Start_addr_6_ROM, End_addr_6_ROM);
          }
          else{
            chipSix.set_play(Start_addr_6_ROM, Start_addr_6_ROM);
          }
          break;
      case 7:
          if(repeat == 0){
            chipSeven.set_play(Start_addr_7_ROM, End_addr_7_ROM);
          }
          else{
            chipSeven.set_play(Start_addr_7_ROM, Start_addr_7_ROM);
          }
          break;
      case 8:
          if(repeat == 0){
            chipEight.set_play(Start_addr_8_ROM, End_addr_8_ROM);
          }
          else{
            chipEight.set_play(Start_addr_8_ROM, Start_addr_8_ROM);
          }
          break;
    }
}

void saveEEPROM(byte chip){//save Recording and playback address of ISD chips into EEPROM
  switch(chip){
    case 1://chip1 start and end address
      Start_addr_1_ROM = Start_addr;
      End_addr_1_ROM = End_addr;
      EEPROM.write(9,Start_addr_1_ROM);
      EEPROM.write(10,End_addr_1_ROM);
      break;
    case 2://chip2 start and end address
      Start_addr_2_ROM = Start_addr;
      End_addr_2_ROM = End_addr;
      EEPROM.write(11,Start_addr_2_ROM);
      EEPROM.write(12,End_addr_2_ROM);
      break;
    case 3://chip3 start and end address
      Start_addr_3_ROM = Start_addr;
      End_addr_3_ROM = End_addr;
      EEPROM.write(13,Start_addr_3_ROM);
      EEPROM.write(14,End_addr_3_ROM);
      break;
    case 4://chip4 start and end address
      Start_addr_4_ROM = Start_addr;
      End_addr_4_ROM = End_addr;
      EEPROM.write(15,Start_addr_4_ROM);
      EEPROM.write(16,End_addr_4_ROM);
      break;
    case 5://chip5 start and end address
      Start_addr_5_ROM = Start_addr;
      End_addr_5_ROM = End_addr;
      EEPROM.write(17,Start_addr_5_ROM);
      EEPROM.write(18,End_addr_5_ROM);
      break;
    case 6://chip6 start and end address
      Start_addr_6_ROM = Start_addr;
      End_addr_6_ROM = End_addr;
      EEPROM.write(19,Start_addr_6_ROM);
      EEPROM.write(20,End_addr_6_ROM);
      break;
    case 7://chip7 start and end address
      Start_addr_7_ROM = Start_addr;
      End_addr_7_ROM = End_addr;
      EEPROM.write(21,Start_addr_7_ROM);
      EEPROM.write(22,End_addr_7_ROM);
      break;
    case 8://chip8 start and end address
      Start_addr_8_ROM = Start_addr;
      End_addr_8_ROM = End_addr;
      EEPROM.write(23,Start_addr_8_ROM);
      EEPROM.write(24,End_addr_8_ROM);
      break;
  }
}

void startRecord(byte chip){//start recording of selected ISD chips
  switch(chip){
    case 1:
      chipOne.g_erase();
      delay(100);
      chipOne.rec();
      break;
    case 2:
      chipTwo.g_erase();
      delay(100);
      chipTwo.rec();
      break;
    case 3:
      chipThree.g_erase();
      delay(100);
      chipThree.rec();
      break;
    case 4:
      chipFour.g_erase();
      delay(100);
      chipFour.rec();
      break;
    case 5:
      chipFive.g_erase();
      delay(100);
      chipFive.rec();
      break;
    case 6:
      chipSix.g_erase();
      delay(100);
      chipSix.rec();
      break;
    case 7:
      chipSeven.g_erase();
      delay(100);
      chipSeven.rec();
      break;
    case 8:
      chipEight.g_erase();
      delay(100);
      chipEight.rec();
      break;
  }
  lcd.print("recording");
  delay(500);
}

void playRhythm(){//Play rhythm pattern which consists of patterns of 8 chips
  byte n;
  byte m;
  for(m = 0; m < 32; m++){
    time_now = millis();
    for(n = 0; n < 8; n++){
      if(m < 31){
        if(bitRead(rhythm_eeprom[n],31-m) == 1){
          playROM(n+1,bitRead(rhythm_eeprom[n],30-m));
        }
      }
      else{
        if(bitRead(rhythm_eeprom[n],31-m) == 1){
          playROM(n+1,0);
        }
      }
    }
    while(millis() < time_now + (15000/bpm)){
    }
  }
}

void set_volume_all(){//set volume of all ISD chips, LSB 3 digit, 000: Max, 111: Min
  byte i;
  for(i=0 ; i<8 ; i++){
    switch(volume[i]){
      case 0://volume 0(Min.)
        apc = 0b0000100111100111;
        break;
      case 1://volume 1
        apc = 0b0000100111100110;
        break;
      case 2:
        apc = 0b0000100111100101;
        break;
      case 3:
        apc = 0b0000100111100100;
        break;
      case 4:
        apc = 0b0000100111100011;
        break;
      case 5:
        apc = 0b0000100111100010;
        break;
      case 6:
        apc = 0b0000100111100001;
        break;
      case 7://volume 7(Max.)
        apc = 0b0000100111100000;
        break;
    }
    switch(i){
      case 0:
        chipOne.wr_apc2(apc);
        delay(100);
        break;
      case 1:
        chipTwo.wr_apc2(apc);
        delay(100);
        break;
      case 2:
        chipThree.wr_apc2(apc);
        delay(100);
        break;
      case 3:
        chipFour.wr_apc2(apc);
        delay(100);
        break;
      case 4:
        chipFive.wr_apc2(apc);
        delay(100);
        break;
      case 5:
        chipSix.wr_apc2(apc);
        delay(100);
        break;
      case 6:
        chipSeven.wr_apc2(apc);
        delay(100);
        break;
      case 7:
        chipEight.wr_apc2(apc);
        delay(100);
        break;
    }
  }
}

void set_volume(byte chip_n){//set volume of each ISD chip, LSB 3 digit, 000: Max, 111: Min
  switch(volume[chip_n -1]){
    case 0:
      apc = 0b0000100111100111;
      break;
    case 1:
      apc = 0b0000100111100110;
      break;
    case 2:
      apc = 0b0000100111100101;
      break;
    case 3:
      apc = 0b0000100111100100;
      break;
    case 4:
      apc = 0b0000100111100011;
      break;
    case 5:
      apc = 0b0000100111100010;
      break;
    case 6:
      apc = 0b0000100111100001;
      break;
    case 7:
      apc = 0b0000100111100000;
      break;
  }
  switch(chip_n -1){
    case 0:
      chipOne.wr_apc2(apc);
      delay(100);
      break;
    case 1:
      chipTwo.wr_apc2(apc);
      delay(100);
      break;
    case 2:
      chipThree.wr_apc2(apc);
      delay(100);
      break;
    case 3:
      chipFour.wr_apc2(apc);
      delay(100);
      break;
    case 4:
      chipFive.wr_apc2(apc);
      delay(100);
      break;
    case 5:
      chipSix.wr_apc2(apc);
      delay(100);
      break;
    case 6:
      chipSeven.wr_apc2(apc);
      delay(100);
      break;
    case 7:
      chipEight.wr_apc2(apc);
      delay(100);
      break;
  }
}

void save_volume_all(){//save volume setting of all ISD chips into EEPROM
  EEPROM.write(0, volume[0]);
  EEPROM.write(1, volume[1]);
  EEPROM.write(2, volume[2]);
  EEPROM.write(3, volume[3]);
  EEPROM.write(4, volume[4]);
  EEPROM.write(5, volume[5]);
  EEPROM.write(6, volume[6]);
  EEPROM.write(7, volume[7]);
}

void save_volume(byte chip){//save volume setting of each ISD chip into EEPROM
  switch(chip){
    case 1:
      EEPROM.write(0, volume[0]);
      break;
    case 2:
      EEPROM.write(1, volume[1]);
      break;
    case 3:
      EEPROM.write(2, volume[2]);
      break;
    case 4:
      EEPROM.write(3, volume[3]);
      break;
    case 5:
      EEPROM.write(4, volume[4]);
      break;
    case 6:
      EEPROM.write(5, volume[5]);
      break;
    case 7:
      EEPROM.write(6, volume[6]);
      break;
    case 8:
      EEPROM.write(7, volume[7]);
      break;
  }
}

void read_volume_eeprom(){//read volume setting of all ISD chips when startup
  volume[0] = EEPROM.read(0);
  volume[1] = EEPROM.read(1);
  volume[2] = EEPROM.read(2);
  volume[3] = EEPROM.read(3);
  volume[4] = EEPROM.read(4);
  volume[5] = EEPROM.read(5);
  volume[6] = EEPROM.read(6);
  volume[7] = EEPROM.read(7);
}

////////////////////////////////////  Functions of menus　/////////////////////////
void display_menu(byte column, byte row){//display menu corresponds to current status and modes
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("->");
  if(column == 0){
    switch(row){
      case 0:
        lcd.setCursor(2, 0);
        lcd.print(F("Play"));
        lcd.setCursor(2, 1);
        lcd.print(F("Edit sound"));
        break;
      case 1:
        lcd.setCursor(2, 0);
        lcd.print(F("Edit sound"));
        lcd.setCursor(2, 1);
        lcd.print(F("Edit rhythm"));
        break;
      case 2:
        lcd.setCursor(2, 0);
        lcd.print(F("Edit rhythm"));
        lcd.setCursor(2, 1);
        lcd.print(F("Edit track"));
        break;
      case 3:
        lcd.setCursor(2, 0);
        lcd.print(F("Edit track"));
        lcd.setCursor(2, 1);
        lcd.print(F("BPM"));
        break;
      case 4:
        lcd.setCursor(2, 0);
        lcd.print(F("BPM"));
        lcd.setCursor(2, 1);
        lcd.print(F("setting"));
        break;
      case 5:
        lcd.setCursor(2, 0);
        lcd.print(F("setting"));
        break;
    }
  }
  else if(column == 1){
    if(chip_n < 8){
      lcd.setCursor(2, 0);
      lcd.print(F("Sound_"));
      lcd.print(chip_n);
      lcd.setCursor(2, 1);
      lcd.print(F("Sound_"));
      lcd.print(chip_n + 1);
    }
    else{
      lcd.setCursor(2, 0);
      lcd.print(F("Sound_8"));
    }
  }
  if(column == 2){
    lcd.setCursor(2, 0);
    lcd.print(F("play and trim"));
    lcd.setCursor(2, 1);
    lcd.print(F("sampling"));
  }
  if(column == 4 && row == 1){
    lcd.clear();
    lcd.home();
    lcd.print(F("BPM = "));
    lcd.print(bpm);
  }
  else if(column == 5 && row == 0){
    if(rhythm_n < 8){
      lcd.setCursor(2, 0);
      lcd.print(F("rhythm_"));
      lcd.print(rhythm_n);
      lcd.setCursor(2, 1);
      lcd.print(F("rhythm_"));
      lcd.print(rhythm_n + 1);
    }
    else{
      lcd.setCursor(2, 0);
      lcd.print(F("rhythm_"));
      lcd.print(rhythm_n);
    }
  }
  else if(column == 5 && row == 1){
    if(track_n < 8){
      lcd.setCursor(2, 0);
      lcd.print(F("track_"));
      lcd.print(track_n);
      lcd.setCursor(2, 1);
      lcd.print(F("track_"));
      lcd.print(track_n + 1);
    }
    else{
      lcd.setCursor(2, 0);
      lcd.print(F("track_"));
      lcd.print(track_n);
    }
  }
  else if(column == 6){
    switch(row){
      case 0:
        lcd.setCursor(2, 0);
        lcd.print(F("level"));
        lcd.setCursor(2, 1);
        lcd.print(F("clear rhythm"));
        break;
      case 1:
        lcd.setCursor(2, 0);
        lcd.print(F("clear rhythm"));
        lcd.setCursor(2, 1);
        lcd.print(F("clear track"));
        break;
      case 2:
        lcd.setCursor(2, 0);
        lcd.print(F("clear track"));
        lcd.setCursor(2, 1);
        lcd.print(F("initialize"));
        break;
      case 3:
        lcd.setCursor(2, 0);
        lcd.print(F("initialize"));
        break;
    }
  }
  else if(column == 10 && row == 0){
    lcd.clear();
    cursor_row = 0;
    lcd.home();
    lcd.cursor();
  }
}

void show_status(){//just for debuging, diplay variables status via Serial Monitor
  byte i;
  Serial.print("menu_column =");
  Serial.print(menu_column);
  Serial.print(", menu_row =");
  Serial.print(menu_row);
  Serial.print(", value =");
  Serial.print(value);
  Serial.print(", chip =");
  Serial.print(chip_n);
  Serial.print(", rhythm =");
  Serial.print(rhythm_n);
  Serial.print(", value_updated =");
  Serial.print(value_updated);
  Serial.print(", track_data =");
  Serial.println(track_data);
}

void display_volume(){//display volume setting of all ISD chips on LCD in "level setting menu"
  byte i;
  lcd.createChar(0, customChar_vol_0);
  lcd.createChar(1, customChar_vol_1);
  lcd.createChar(2, customChar_vol_2);
  lcd.createChar(3, customChar_vol_3);
  lcd.createChar(4, customChar_vol_4);
  lcd.createChar(5, customChar_vol_5);
  lcd.createChar(6, customChar_vol_6);
  lcd.createChar(7, customChar_vol_7);
  for(i = 0; i<8; i++){
    switch(volume[i]){
      case 0:
        lcd.setCursor(i,1);
        lcd.write((byte)0);
        break;
      case 1:
        lcd.setCursor(i,1);
        lcd.write((byte)1);
        break;
      case 2:
        lcd.setCursor(i,1);
        lcd.write((byte)2);
        break;
      case 3:
        lcd.setCursor(i,1);
        lcd.write((byte)3);
        break;
      case 4:
        lcd.setCursor(i,1);
        lcd.write((byte)4);
        break;
      case 5:
        lcd.setCursor(i,1);
        lcd.write((byte)5);
        break;
      case 6:
        lcd.setCursor(i,1);
        lcd.write((byte)6);
        break;
      case 7:
        lcd.setCursor(i,1);
        lcd.write((byte)7);
        break;
    }
    lcd.setCursor(cursor_row, 0);
  }
}

void lcd_row_arrow(boolean row){//display "->" cursor at either upper row or lower row on LCD
  if(row == 0){
    lcd.setCursor(0, 0);
    lcd.print("->");
    lcd.setCursor(0, 1);
    lcd.print("  ");
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("->");
  }
}

////////////////////////////////////    Functions related to EEPROM     /////////////////////////
uint16_t getPattern_eeprom(byte rhythm, byte sound, boolean first){//load 16bit rhythm data (1st or 2nd part) of only selected rhythm and chip from EEPROM
  eeprom_address = 25 + 32*(rhythm - 1) + 4*(sound - 1) + 2*first;
  pattern_eeprom = EEPROM.read(eeprom_address) << 8;
  pattern_eeprom += EEPROM.read(eeprom_address + 1);
  return pattern_eeprom;
}

void savePattern_eeprom(byte rhythm, byte sound, boolean first, uint16_t pattern){//save 16bit rhythm data (1st or 2nd part) of only selected rhythm and chip into EEPROM
  eeprom_address = 25 + 32*(rhythm - 1) + 4*(sound - 1) + 2*first;
  EEPROM.write(eeprom_address, pattern_eeprom_current >> 8);
  eeprom_address += 1;
  EEPROM.write(eeprom_address, (pattern_eeprom_current & 255));
}

void getRhythm_eeprom(byte rhythm_n){//load entire rhythm pattern which consists of 32bit rhythm data of 8 chips
  byte n;
  for(n = 0; n < 8; n++){
    rhythm_eeprom[n] = (getPattern_eeprom(rhythm_n, n+1, FIRST) * 65536);
    rhythm_eeprom[n] += getPattern_eeprom(rhythm_n, n+1, SECOND);
  }  
}

void displayPattern_eeprom(byte rhythm, byte sound, boolean first){//display rhythm pattern on LCD
  byte n;
  uint16_t pattern;
  pattern_eeprom_current = getPattern_eeprom(rhythm, sound, first);
  for(n = 0; n < 16; n++){
    pattern_digit = (pattern_eeprom_current & (32768 >> n)) >> (15-n);
    lcd.setCursor(n,1);
    lcd.print(pattern_digit);
  }
}

void displayTrack(byte track_n){//display track pattern on LCD
  byte n;
  lcd.home();
  cursor_column = 0;
  for(n = 0; n < 16 ; n++){
    eeprom_address = 281 + 32*(track_n -1) + (n / 2) + 8*cursor_column;
    if(n % 2 == 0){
      track_data = EEPROM.read(eeprom_address) >> 4;
    }
    else{
      track_data = EEPROM.read(eeprom_address) & 0b00001111;
    }
    lcd.setCursor(n, 0);
    if(track_data != 0){
      lcd.print(track_data);
    }
    else{
      lcd.print(" ");
    }
  }
  cursor_column = 1;
  for(n = 0; n < 16 ; n++){
    eeprom_address = 281 + 32*(track_n -1) + (n / 2) + 8*cursor_column;
    if(n % 2 == 0){
      track_data = EEPROM.read(eeprom_address) >> 4;
    }
    else{
      track_data = EEPROM.read(eeprom_address) & 0b00001111;
    }
    lcd.setCursor(n, 1);
    if(track_data != 0){
      lcd.print(track_data);
    }
    else{
      lcd.print(" ");
    }
  }
  lcd.home();
  cursor_column = 0;
}

void clearRhythm_all_eeprom(){//clear all pattern from rhythm 1 to 8 stored in EEPROM 
  int n;
  for(n = 25; n < 281; n++){
     EEPROM.write(n, 0);
  }
}

void clearTrack_all(){//clear all track from track 1 to 8 stored in EEPROM 
  int n;
  for(n = 281; n < 537; n++){
     EEPROM.write(n, 0);
  }
}

void updateSectionAddr(void){//load recording and playback start and end address of all ISD chips on startup
  Start_addr_1_ROM = EEPROM.read(9);
  End_addr_1_ROM = EEPROM.read(10);
  Start_addr_2_ROM = EEPROM.read(11);
  End_addr_2_ROM = EEPROM.read(12);
  Start_addr_3_ROM = EEPROM.read(13);
  End_addr_3_ROM = EEPROM.read(14);
  Start_addr_4_ROM = EEPROM.read(15);
  End_addr_4_ROM = EEPROM.read(16);
  Start_addr_5_ROM = EEPROM.read(17);
  End_addr_5_ROM = EEPROM.read(18);
  Start_addr_6_ROM = EEPROM.read(19);
  End_addr_6_ROM = EEPROM.read(20);
  Start_addr_7_ROM = EEPROM.read(21);
  End_addr_7_ROM = EEPROM.read(22);
  Start_addr_8_ROM = EEPROM.read(23);
  End_addr_8_ROM = EEPROM.read(24);

  Serial.print("S1,E1:");
  Serial.print(Start_addr_1_ROM);
  Serial.print(", ");
  Serial.println(End_addr_1_ROM);
  Serial.print("S2,E2:");
  Serial.print(Start_addr_2_ROM);
  Serial.print(", ");
  Serial.println(End_addr_2_ROM);
  Serial.print("S3,E3:");
  Serial.print(Start_addr_3_ROM);
  Serial.print(", ");
  Serial.println(End_addr_3_ROM);
  Serial.print("S4,E4:");
  Serial.print(Start_addr_4_ROM);
  Serial.print(", ");
  Serial.println(End_addr_4_ROM);
  Serial.print("S5,E5:");
  Serial.print(Start_addr_5_ROM);
  Serial.print(", ");
  Serial.println(End_addr_5_ROM);
  Serial.print("S6,E6:");
  Serial.print(Start_addr_6_ROM);
  Serial.print(", ");
  Serial.println(End_addr_6_ROM);
  Serial.print("S7,E7:");
  Serial.print(Start_addr_7_ROM);
  Serial.print(", ");
  Serial.println(End_addr_7_ROM);
  Serial.print("S8,E8:");
  Serial.print(Start_addr_8_ROM);
  Serial.print(", ");
  Serial.println(End_addr_8_ROM);
}

//////////////////////////////////// Functions related to 74HC4040 and analog switches to control recording sound quality  /////////////////////////
void set_REC_quality(byte quality){
  byte n;
  byte count = 0;
  digitalWrite(analog_SW_reset, HIGH);//74HC4040 reset, all output -> L, all analog SW :OFF
  if(quality > 0){
    digitalWrite(analog_SW_reset, LOW);
    for(n = 0; n < quality; n++){
      digitalWrite(analog_SW_clk, HIGH);
      time_now = micros();
      while(micros() < time_now + 50){ 
      }
      digitalWrite(analog_SW_clk, LOW);//74HC4040 count up
      time_now = micros();
      while(micros() < time_now + 50){ 
      }
      count ++;
    }
    Serial.print("REC_quality = ");
    Serial.println(count);
  }
}

void save_REC_quality(byte quality){//save recording and playback quality in EEPROM so that they will be set correctly after startup
  EEPROM.write(538, quality);
}

////////////////////////////////////   SETUP of Arduino (only one time)  //////////////////////////////////////////////////
void setup()
{
  pinMode(12,INPUT_PULLUP);//necessary for preventing SPI_MISO line floated during SPI idle state.
  analogReference(EXTERNAL);//using 3.3V as ADC reference
  Serial.begin(9600);
  // INITIALIZE NeoPixel object for W2812B full color LEDs
  strip.begin();           
  strip.setBrightness(10);
  strip.clear();
  strip.show();
  
  // initialize I2C LCD
  lcd.init();
  lcd.backlight();
  
  // initialize click encoder
  encoder = new ClickEncoder(A0, A1, -1, 4);//ClickEncoder(uint8_t A, uint8_t B, uint8_t BTN = -1, uint8_t stepsPerNotch = 1, bool active = LOW);

  // initialize ISD1700 chips
  powerUpISD_all();
  configISD_all();
  showISD_apc();
  
  updateSectionAddr();//load recording and playback start and end address of all ISD chips on startup
  read_volume_eeprom();//read volume setting of all ISD chips when startup
  set_volume_all();//set volume of all ISD chips, LSB 3 digit, 000: Max, 111: Min
  
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  UCSR0B &= ~(1<<4);//disable UART Rx so that can use D0 as digital pin
  pinMode(analog_SW_reset,OUTPUT);//74HC4040 reset
  pinMode(analog_SW_clk, OUTPUT);//74HC4040 clk
  digitalWrite(analog_SW_reset, HIGH);//reset 74HC4040
  digitalWrite(analog_SW_clk, LOW);
  
  // setting ISD chips sound quality which stored in EEPROM
  REC_quality = EEPROM.read(538);
  set_REC_quality(REC_quality);

  display_menu(0,0);// display top menu 
}

////////////////////////////////////    MAIN loop  　 ///////////////////////////////////////////////////
void loop() {
  byte n;
  byte i;
  byte p;
  byte loop = 0;
  /////////////   update buttons and keys status with ADC  /////////////
  buttons.update();
  /*
  //please use this to determine ADC values for reading buttons and keys
  key = analogRead(BUTTONS_PIN);
  Serial.println(key);
  */
  ////////////   update rotary encorder status   ///////////////////
  value += encoder->getValue();
  //show_status();//show statsus for debuging, be noted that if showing status, real time playing in "play mode" is impossible

  /////////////////////// Process in TOP menu  //////////////////////////////////
  if(menu_column == 0){//TOP menu
    if(value != last){//when encorder value is updated
        if(value<0){
          value = 0;
        }
        else if(value>5){
          value = 5;
        }
        menu_row = value;
        display_menu(menu_column, value);
        if(value == 5){
          lcd.setCursor(0,1);
          lcd.print("        ");
        }
        Serial.println(value);      
    }    
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      if(menu_row == 1){//"Edit sound" is selected
        menu_column = 1;
        menu_row = 0;
        display_menu(menu_column, menu_row);
        value = 0;
      }
      else if(menu_row == 2){//"Edit rhythm" is selected
        menu_column = 5;
        menu_row = 0;
        display_menu(menu_column, menu_row);
        value = 0;
      }
      else if(menu_row == 3){//"Edit track" is selected
        menu_column = 5;
        menu_row = 1;
        display_menu(menu_column, menu_row);
        value = 0;
      }
      else if(menu_row == 4){//"BPM" is selected
        menu_column = 4;
        menu_row = 1;
        lcd.clear();
        lcd.home();
        display_menu(menu_column, menu_row);
        lcd.setCursor(3,0);
        lcd.print(" = ");
        lcd.print(bpm);
      }
      else if(menu_row == 5){//"Setting" is selected
        menu_column = 6;
        menu_row = 0;
        value = 0;
        display_menu(menu_column, menu_row);
      }
    }    
    if(menu_row == 0){//play sound when PLAY mode is selected
      if(buttons.isPressed(pad_1)){
        playROM_pad(1);
      }
      else if(buttons.isPressed(pad_2)){
        playROM_pad(2);
      }
      else if(buttons.isPressed(pad_3)){
        playROM_pad(3);
      }
      else if(buttons.isPressed(pad_4)){
        playROM_pad(4);
      }
      else if(buttons.isPressed(pad_5)){
        playROM_pad(5);
      }
      else if(buttons.isPressed(pad_6)){
        playROM_pad(6);
      }
      else if(buttons.isPressed(pad_7)){
        playROM_pad(7);
      }
      else if(buttons.isPressed(pad_8)){
        playROM_pad(8);
      }    
    }
  }

  /////////////////////// Process in selecting Sound  //////////////////////////////////
  else if(menu_column == 1){
    if(value != last){//when encorder value is updated
      if(value<0){
        value = 0;
      }
      else if(value>=7){
        value = 7;
      }
      chip_n = value + 1;
      display_menu(menu_column, menu_row);
    }

    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      menu_column = 2;
      display_menu(menu_column, menu_row);
      chip_n = value +1;
      value = 0;
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 0;
      menu_row = 1;
      value = 1;
      display_menu(menu_column, menu_row);
    }
  }
  
  /////////////////////// Process in selecting menu "play and trim" or "Sampling"  //////////////////////////////////
  else if(menu_column == 2){
    if(value != last){//when encorder value is updated
      if(value % 2 == 0){
        menu_row = 0;
        lcd.setCursor(0,1);
        lcd.print(F("  "));
        lcd.home();
        lcd.print(F("->"));
      }
      else{
        menu_row = 1;
        lcd.home();
        lcd.print(F("  "));
        lcd.setCursor(0,1);
        lcd.print(F("->"));
      }
    }
    if(buttons.onPress(button_encoder) && menu_row == 0){//when encoder button is pressed on selecting "play and trim" menu
      menu_column = 11;
      menu_row = 0;
      if(EEPROM.read(9 + 2*(chip_n - 1)) >= 16){
        Start_addr = EEPROM.read(9 + 2*(chip_n - 1));
      }
      else{
        Start_addr = 16;
      }
      if(EEPROM.read(10 + 2*(chip_n - 1)) >= 16){
        End_addr = EEPROM.read(10 + 2*(chip_n - 1));
      }
      else{
        End_addr = 16;
      }
      lcd.clear();
      lcd.home();
      lcd.print(F("->"));
      lcd.setCursor(2, 0);
      lcd.print(F("START:"));
      lcd.print(Start_addr);
      lcd.setCursor(2, 1);
      lcd.print(F("END:"));
      lcd.print(End_addr);
      lcd.setCursor(14, 0);
      lcd.print(F("S"));
      lcd.print(chip_n);
      value = EEPROM.read(9 + 2*(chip_n - 1));
      encoder->setAccelerationEnabled(true);
    }
    if(buttons.onPress(button_encoder) && menu_row == 1){//when encoder button is pressed on selecting "Sampling" menu
      menu_column = 3;
      menu_row = 0;
      lcd.clear();
      lcd.print(F("->"));
      lcd.print(F("Lo-Fi"));
      lcd.setCursor(2,1);
      lcd.print(F("Hi-Fi"));
      value = 0;
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 1;
      display_menu(menu_column, menu_row);
      value = chip_n - 1;
    }
  }

  /////////////////////// Process in selecting "Lo-fi" or "Hi-Fi" in Sampling menu //////////////////////////////////
  else if(menu_column == 3){
    if(value != last){//when encorder value is updated
      if(value%2 == 0){
        lcd.setCursor(0,1);
        lcd.print("  ");
        lcd.home();
        lcd.print(F("->"));
      }
      else{
        lcd.home();
        lcd.print(F("  "));
        lcd.setCursor(0,1);
        lcd.print(F("->"));
      }
    }
    if(buttons.onPress(button_encoder) && menu_row == 0){//when encoder button is pressed
      if(value%2 == 0){//when selecting "Lo-Fi"
        bitWrite(REC_quality, chip_n - 1, 0);        
      }
      else{
        bitWrite(REC_quality, chip_n - 1, 1);
      }
      set_REC_quality(REC_quality & ~(1<<(chip_n -1)));
      menu_column = 4;
      menu_row = 0;
      lcd.setCursor(2, 0);
      lcd.print(F("start sampling"));
      switch(chip_n){
        case 1:
           chipOne.wr_apc2(0b0000100110100000);//enable Analog FT(feed through) of only ISD chip which will be recorded
           break;
        case 2:
           chipTwo.wr_apc2(0b0000100110100000);
           break;
        case 3:
           chipThree.wr_apc2(0b0000100110100000);
           break;
        case 4:
           chipFour.wr_apc2(0b0000100110100000);
           break;
        case 5:
           chipFive.wr_apc2(0b0000100110100000);
           break;
        case 6:
           chipSix.wr_apc2(0b0000100110100000);
           break;
        case 7:
           chipSeven.wr_apc2(0b0000100110100000);
           break;
        case 8:
           chipEight.wr_apc2(0b0000100110100000);
           break;
      }
      delay(100);
      lcd.setCursor(2,1);
      lcd.print(F("push Pad_"));
      lcd.setCursor(10,1);
      lcd.print(chip_n);
      value = 0;
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column -= 1;
      menu_row = 0;
      display_menu(menu_column, menu_row);
      lcd.noBlink();    
    }
  }

  /////////////////////// Process in "Sampling" menu //////////////////////////////////
  else if((menu_column == 4) && (menu_row == 0)){
    if(chip_n == 1 && buttons.onPress(pad_1)){
      startRecord(1);
    }
    else if(chip_n == 2 && buttons.onPress(pad_2)){
      startRecord(2);
    }
    else if(chip_n == 3 && buttons.onPress(pad_3)){
      startRecord(3);
    }
    else if(chip_n == 4 && buttons.onPress(pad_4)){
      startRecord(4);
    }
    else if(chip_n == 5 && buttons.onPress(pad_5)){
      startRecord(5);
    }
    else if(chip_n == 6 && buttons.onPress(pad_6)){
      startRecord(6);
    }
    else if(chip_n == 7 && buttons.onPress(pad_7)){
      startRecord(7);
    }
    else if(chip_n == 8 && buttons.onPress(pad_8)){
      startRecord(8);
    }
    if(chip_n == 1 && buttons.onRelease(pad_1)){
      chipOne.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    else if(chip_n == 2 && buttons.onRelease(pad_2)){
      chipTwo.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    else if(chip_n == 3 && buttons.onRelease(pad_3)){
      chipThree.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    else if(chip_n == 4 && buttons.onRelease(pad_4)){
      chipFour.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    else if(chip_n == 5 && buttons.onRelease(pad_5)){
      chipFive.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    else if(chip_n == 6 && buttons.onRelease(pad_6)){
      chipSix.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    else if(chip_n == 7 && buttons.onRelease(pad_7)){
      chipSeven.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    else if(chip_n == 8 && buttons.onRelease(pad_8)){
      chipEight.stop();
      set_REC_quality(REC_quality);
      save_REC_quality(REC_quality);
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 2;
      menu_row = 0;
      set_volume_all();
      delay(100);
      display_menu(menu_column, menu_row);
      apc_ISDchipOne = chipOne.rd_apc();
      Serial.println(apc_ISDchipOne, BIN);
      apc_ISDchipTwo = chipTwo.rd_apc();
      Serial.println(apc_ISDchipTwo, BIN);
      apc_ISDchipThree = chipThree.rd_apc();
      Serial.println(apc_ISDchipThree, BIN);  
      apc_ISDchipFour = chipFour.rd_apc();
      Serial.println(apc_ISDchipFour, BIN);
      apc_ISDchipFive = chipFive.rd_apc();
      Serial.println(apc_ISDchipFive, BIN);
      apc_ISDchipSix = chipSix.rd_apc();
      Serial.println(apc_ISDchipSix, BIN);
      apc_ISDchipSeven = chipSeven.rd_apc();
      Serial.println(apc_ISDchipSeven, BIN);
      apc_ISDchipEight = chipEight.rd_apc();
      Serial.println(apc_ISDchipEight, BIN);
    }
  }
  
  /////////////////////// Process in "BPM" setting menu //////////////////////////////////
  else if((menu_column == 4) && (menu_row == 1)){
    if(value != last){//when encorder value is updated
      if(value < 0){
        bpm --;
      }
      else{
        bpm ++;
      }
      value = 0;
      display_menu(menu_column, menu_row);
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 0;
      menu_row = 4;
      display_menu(menu_column, menu_row);
    }
  }
  
  /////////////////////// Process in selecting "rhythm_number" or "track_number" //////////////////////////////////
  else if(menu_column == 5){
    if((value != last) && menu_row == 0){//when encorder value is updated in selecting "rhythm_number"
      if(value < 0){
        value = 0;
      }
      else if(value >= 7){
        value = 7;
      }
      rhythm_n = value + 1;
      display_menu(menu_column, menu_row);
    }
    else if((value != last) && menu_row == 1){//when encorder value is updated in selecting "track_number"
      if(value < 1){
        value = 1;
      }
      else if(value >= 8){
        value = 8;
      }
      track_n = value;
      display_menu(menu_column, menu_row);
    }
    if(buttons.onPress(button_encoder) && menu_row == 0){//when encoder button is pressed in selecting "rhythm_number"
      int n;
      boolean pattern_digit;
      menu_column = 8;
      menu_row = 0;
      rhythm_n = value + 1;
      value = 0;
      lcd.clear();
      getRhythm_eeprom(1);
      for(n = 0; n < 32; n++){
        pattern_digit = bitRead(rhythm_eeprom[1],31-n);
        if(n < 16){
          lcd.setCursor(n,0);  
        }
        else{
          lcd.setCursor(n-16,1);
        }        
        lcd.print(pattern_digit);
      }   
    }
    else if(buttons.onPress(button_encoder) && menu_row == 1){//when encoder button is pressed in selecting "track_number"
      menu_column = 10;
      menu_row = 0;
      eeprom_address = 281 + 32*(track_n -1);
      track_data = EEPROM.read(eeprom_address);
      value = track_data >> 4;
      display_menu(menu_column, menu_row);
      displayTrack(track_n);
      value_updated = 0;
    }
    if(buttons.isPressed(pad_play)){//when PLAY button is pressed
      lcd.createChar(0, customChar_play_icon);
      lcd.setCursor(11,0);
      lcd.write((byte)0);
      getRhythm_eeprom(rhythm_n);
      playRhythm();
      lcd.setCursor(11,0);
      lcd.print(F(" "));
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      if(menu_column == 5 && menu_row == 0){
        menu_column = 0;
        menu_row = 2;
        value = 2;
        display_menu(menu_column, menu_row);
      }
      else if(menu_column == 5 && menu_row == 1){
        menu_column = 0;
        menu_row = 3;
        value = 3;
        display_menu(menu_column, menu_row);
      }
    }
  }

  /////////////////////// Process in selecting "level", "clear rhythm", "clear track", "initialize" //////////////////////////////////
  else if(menu_column == 6){
    if(value != last){//when encorder value is updated
      if(value<0){
        value = 0;
      }
      else if(value >=3){
        value =3;
      }
      menu_row = value;
      display_menu(menu_column, menu_row);
    }
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      if(menu_row == 0){//in selecting "level"
        menu_column = 7;
        lcd.clear();
        lcd.home();
        lcd.print(F("1"));
        lcd.setCursor(1,0);
        lcd.print(F("2"));
        lcd.setCursor(2,0);
        lcd.print(F("3"));
        lcd.setCursor(3,0);
        lcd.print(F("4"));
        lcd.setCursor(4,0);
        lcd.print(F("5"));
        lcd.setCursor(5,0);
        lcd.print(F("6"));
        lcd.setCursor(6,0);
        lcd.print(F("7"));
        lcd.setCursor(7,0);
        lcd.print(F("8"));
        lcd.setCursor(12,0);
        lcd.print(F("SAVE"));
        lcd.setCursor(0,0);
        lcd.blink();
        value = 7;
        display_volume();
      }
      else if(menu_row == 1){//in selecting "clear rhythm"
        menu_column = 9;
        lcd.clear();
        lcd.print(F("Sure?"));
      }
      else if(menu_row == 2){//in selecting "clear track"
        menu_column = 12;
        lcd.clear();
        lcd.print(F("Sure?"));
      }
      else if(menu_row == 3){//in selecting "initialize"
        lcd.setCursor(0, 1);
        lcd.print(F("Sure?"));
        menu_column = 13;
      }
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 0;
      menu_row = 5;
      display_menu(menu_column, menu_row);
      lcd.setCursor(0,1);
      lcd.print("        "); 
    }
  }
  
  /////////////////////// Process in "level setting" menu //////////////////////////////////
  else if(menu_column == 7){
    if(value != last){//when encorder value is updated
      if(value<0){
        value = 0;
      }
      else if(value >=7){
        value =7;
      }
      volume[cursor_row] = value;
      display_volume();
      set_volume(cursor_row +1);
    }
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      display_volume();
        if(cursor_row < 7){
          set_volume(cursor_row + 1);
          cursor_row ++;
        }
        else if(cursor_row == 7){//selecting "SAVE"
          set_volume(8);
          cursor_row = 10;
          lcd.setCursor(10,0);
          lcd.print("->");
        }
        else if(cursor_row == 10)
        {
          save_volume_all();
          lcd.print("  ");
          cursor_row = 0;
        }
        lcd.setCursor(cursor_row,0);
    }
    if(buttons.isPressed(pad_1)){//play sound which corresponds to pad_1 to pad_8
      playROM_pad(1);
    }
    else if(buttons.isPressed(pad_2)){
      playROM_pad(2);
    }
    else if(buttons.isPressed(pad_3)){
      playROM_pad(3);
    }
    else if(buttons.isPressed(pad_4)){
      playROM_pad(4);
    }
    else if(buttons.isPressed(pad_5)){
      playROM_pad(5);
    }
    else if(buttons.isPressed(pad_6)){
      playROM_pad(6);
    }
    else if(buttons.isPressed(pad_7)){
      playROM_pad(7);
    }
    else if(buttons.isPressed(pad_8)){
      playROM_pad(8);
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      lcd.noBlink();
      set_volume_all();
      menu_column = 6;
      menu_row = 0;
      display_menu(menu_column, menu_row);
    }
  }  

  /////////////////////// Process in "Rhythm edit" menu //////////////////////////////////
  else if(menu_column == 8){
    if(buttons.onPress(pad_play)){
      if(play_status == 0){
        play_status = 1;
        k = 0;
        time_now = millis();
        strip.setPixelColor(0, 0,255, 0);//color data -> GRB, play_LED
        strip.show();
      }
      else{
        play_status = 0;
        rec_status = 0;
        strip.clear();
        strip.show();
      }
    }
    if(buttons.onPress(pad_rec)){
      if(rec_status == 0){
        rec_status = 1;
        strip.setPixelColor(1, 255, 0, 0);//color data -> GRB, REC_LED
        strip.show();
      }
      else{
        rec_status = 0;
        strip.setPixelColor(1, 0, 0, 0);//color data -> GRB, REC_LED
        strip.show();
      }
    }    
    if(play_status == 1 && (millis() > time_now + (15000/bpm))){
      for(p = 0; p < 8; p++){
        if(k < 31){
          if(bitRead(rhythm_eeprom[p],31-k) == 1){
            playROM(p+1,bitRead(rhythm_eeprom[p],30-k));
          }
        }
        else{
          if(bitRead(rhythm_eeprom[p],31-k) == 1){
            playROM(p+1,bitRead(rhythm_eeprom[p],31));
          }
        }
      }      
      if(k == 0 && rec_status == 1){
        tone(17,2000,10);
      }
      else if(!(k % 4) && rec_status == 1){
        tone(17,1000,10);
      }
      if(k<31){
        k++;  
      }
      else{
        k = 0;
      }      
      time_now = millis();
    }
    if(buttons.onPress(pad_1)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[0],0,1);
          playROM(1,bitRead(rhythm_eeprom[0],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[0],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[0],32-k,1);
          playROM(1,bitRead(rhythm_eeprom[0],31-k));
        }
      }
      else{
        playROM(1,bitRead(rhythm_eeprom[0],31-k));
      }
    }
    else if(buttons.onPress(pad_2)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[1],0,1);
          playROM(2,bitRead(rhythm_eeprom[1],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[1],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[1],32-k,1);
          playROM(2,bitRead(rhythm_eeprom[1],31-k));
        }
      }
      else{
        playROM(2,bitRead(rhythm_eeprom[1],31-k));
      }
    }
    else if(buttons.onPress(pad_3)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[2],0,1);
          playROM(3,bitRead(rhythm_eeprom[2],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[2],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[2],32-k,1);
          playROM(3,bitRead(rhythm_eeprom[2],31-k));
        }
      }
      else{
        playROM(3,bitRead(rhythm_eeprom[2],31-k));
      }
    }
    else if(buttons.onPress(pad_4)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[3],0,1);
          playROM(4,bitRead(rhythm_eeprom[3],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[3],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[3],32-k,1);
          playROM(4,bitRead(rhythm_eeprom[3],31-k));
        }
      }
      else{
        playROM(4,bitRead(rhythm_eeprom[3],31-k));
      }
    }
    else if(buttons.onPress(pad_5)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[4],0,1);
          playROM(5,bitRead(rhythm_eeprom[4],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[4],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[4],32-k,1);
          playROM(5,bitRead(rhythm_eeprom[4],31-k));
        }
      }
      else{
        playROM(5,bitRead(rhythm_eeprom[4],31-k));
      }
    }
    else if(buttons.onPress(pad_6)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[5],0,1);
          playROM(6,bitRead(rhythm_eeprom[5],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[5],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[5],32-k,1);
          playROM(6,bitRead(rhythm_eeprom[5],31-k));
        }
      }
      else{
        playROM(6,bitRead(rhythm_eeprom[5],31-k));
      }
    }
    else if(buttons.onPress(pad_7)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[6],0,1);
          playROM(7,bitRead(rhythm_eeprom[6],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[6],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[6],32-k,1);
          playROM(7,bitRead(rhythm_eeprom[6],31-k));
        }
      }
      else{
        playROM(7,bitRead(rhythm_eeprom[6],31-k));
      }
    }
    else if(buttons.onPress(pad_8)){
      if(rec_status == 1){
        if(k == 0 && (millis() < time_now + (7500/bpm))){
          bitWrite(rhythm_eeprom[7],0,1);
          playROM(8,bitRead(rhythm_eeprom[7],31));
        }
        else if(millis() >= time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[7],31-k,1);
        }
        else if(millis() < time_now + (7500/bpm)){
          bitWrite(rhythm_eeprom[7],32-k,1);
          playROM(8,bitRead(rhythm_eeprom[7],31-k));
        }
      }
      else{
        playROM(8,bitRead(rhythm_eeprom[7],31-k));
      }
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      savePattern_eeprom(rhythm_n,chip_n, first_second, pattern_eeprom_current);
      menu_column = 5;
      menu_row = 0;
      display_menu(menu_column, menu_row);
      value = chip_n - 1;
      lcd.noBlink();
      lcd.noCursor();
    }
  }

  /////////////////////// Process in "Clear Rhythm" menu //////////////////////////////////
  else if(menu_column == 9){
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      lcd.home();
      lcd.print("clearing rhythms");
      clearRhythm_all_eeprom();
      lcd.setCursor(0,1);
      lcd.print("done.");
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 6;
      menu_row = 1;
      display_menu(menu_column, menu_row);
    }
  }

  /////////////////////// Process in "Track edit" menu //////////////////////////////////
  else if(menu_column == 10){
    if(value != last){//when encorder value is updated
      value_updated = 1;
      if(value<0){
        value = 0;
      }
      else if(value>=8){
        value = 8;
      }
      if(value != 0){
        lcd.print(value);
      }
      else{
        lcd.print(" ");
      }
      lcd.setCursor(cursor_row, cursor_column);
    }
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      if(value_updated == 1){
        eeprom_address = 281 + 32*(track_n -1) + (cursor_row / 2) + 8*cursor_column;
        if(cursor_row % 2 == 0){
          track_data = (value << 4) + (EEPROM.read(eeprom_address) & 0b00001111);
        }
        else{
          track_data = (EEPROM.read(eeprom_address) & 0b11110000) + value;
        }
        EEPROM.write(eeprom_address, track_data);
        value_updated = 0;
      }
      if(cursor_row < 15){
        cursor_row ++;
        lcd.setCursor(cursor_row, cursor_column);
      }
      else{
        cursor_row = 0;
        if(cursor_column == 0){
          cursor_column = 1;
        }
        else{
          cursor_column = 0;
        }
        lcd.setCursor(cursor_row, cursor_column);
      }
    }
    if(buttons.onReleaseAfter(button_encoder,500)){
      cursor_row --;
      if(cursor_row == -1 && cursor_column == 0){
        cursor_column = 1;
        cursor_row = 15;
      }
      else if(cursor_row == -1 && cursor_column == 1){
        cursor_column = 0;
        cursor_row = 15;
      }
      lcd.setCursor(cursor_row, cursor_column);
      eeprom_address = 281 + 32*(track_n -1) + (cursor_row / 2) + 8*cursor_column;
      track_data = EEPROM.read(eeprom_address);
        if(cursor_row % 2 == 0){
          value = track_data >> 4;
        }
        else{
          value = track_data & 0b00001111;
        }
      value_updated = 0;
    }
    if(buttons.isPressed(pad_play)){//when PLAY button is pressed
      for(n = 0; n < cursor_row ; n++){
        eeprom_address = 281 + 32*(track_n -1) + (n / 2) + 8*cursor_column;
        if(n % 2 == 0){
          track_data = EEPROM.read(eeprom_address) >> 4;
        }
        else{
          track_data = EEPROM.read(eeprom_address) & 0b00001111;
        }
        if(track_data != 0){
          getRhythm_eeprom(track_data);
          lcd.setCursor(n, cursor_column);
          playRhythm();
        }
      }
      lcd.setCursor(cursor_row, cursor_column);
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 5;
      menu_row = 1;
      value = 1;
      value_updated = 0;
      lcd.noCursor();
      display_menu(menu_column, menu_row);
    }
  }

  /////////////////////// Process in "Start Address, End Address triming" menu //////////////////////////////////
  else if(menu_column == 11){
    if(value != last){//when encorder value is updated
        lcd.setCursor(10, 1);
        lcd.print("      ");
        if(value > 255){
          value = 255;
        }
        else if(value < 16){
          value = 16;
        }
        if(menu_row == 0){
          Start_addr = value;
          lcd.setCursor(10, 0);
          lcd.print(" ");
          lcd.setCursor(8, 0);
          lcd.print(Start_addr);
        }
        else{
          End_addr = value;
          lcd.setCursor(8, 1);
          lcd.print(" ");
          lcd.setCursor(6, 1);
          lcd.print(End_addr);
        }
    }
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      lcd.setCursor(10, 1);
      lcd.print("      ");
      if(menu_row == 0){//if start address
        menu_row = 1;//then toggle to end address
        lcd_row_arrow(1);
        value = End_addr;
      }
      else{//if end address
        menu_row = 0;//then toggle to start address
        lcd_row_arrow(0);
        value = Start_addr;
      }
    }
    if(buttons.onReleaseAfter(button_encoder,500)){//when encoder button is long pressed
      if(menu_column == 11){//save START and END address to ROM
        saveEEPROM(chip_n);
        lcd.setCursor(10, 1);
        lcd.print("saved!");
        updateSectionAddr();
      }
    }
    if(buttons.isPressed(pad_play)){//when PLAY button is pressed
      playSection(chip_n);
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 2;
      menu_row = 0;
      value = 0;
    }
  }

  /////////////////////// Process in "Clear Track" menu //////////////////////////////////
  else if(menu_column == 12){
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      lcd.home();
      lcd.print("clearing tracks");
      clearTrack_all();
      lcd.setCursor(0,1);
      lcd.print("done.");    
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 6;
      menu_row = 2;
      display_menu(menu_column, menu_row);
    }
  }

  /////////////////////// Process in "initialize" menu //////////////////////////////////
  else if(menu_column == 13){
    if(buttons.onPress(button_encoder)){//when encoder button is pressed
      lcd.setCursor(0, 1);
      EEPROM.write(0, 7);//volume 0
      EEPROM.write(1, 7);//volume 1
      EEPROM.write(2, 7);
      EEPROM.write(3, 7);
      EEPROM.write(4, 7);
      EEPROM.write(5, 7);
      EEPROM.write(6, 7);
      EEPROM.write(7, 7);//volume 7
      lcd.print("volume initialized");
      Serial.println("volume0-7");
    }
    if(buttons.onRelease(pad_back)){//when back button is pressed
      menu_column = 6;
      menu_row = 3;
      display_menu(menu_column, menu_row);
      lcd.noBlink();    
    }
  }

  last = value;//save current rotary encoder value into last
}
