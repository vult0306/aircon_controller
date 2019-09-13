#ifndef _HEADERFILE_H
#define _HEADERFILE_H

#define KEY_MODE_PIN 5
#define KEY_NEXT_PIN 6
#define KEY_BACK_PIN 7
#define KEY_OK_PIN 8

#define KEY_MODE_IN       !digitalRead(KEY_MODE_PIN)
#define KEY_NEXT_IN       !digitalRead(KEY_NEXT_PIN)
#define KEY_BACK_IN       !digitalRead(KEY_BACK_PIN)
#define KEY_OK_IN         !digitalRead(KEY_OK_PIN)

#define ON 1
#define OFF 0
#define MAX_TEM 40          //highest temperature
#define MIN_TEM 18          //lowest temperature
#define MAX_PER 24
#define MIN_PER 1
#define MAX_LOAD_NUM 2      //aircons
#define RELAY_A_PIN 3       //pin 3
#define RELAY_B_PIN 4       //pin 4

byte load[MAX_LOAD_NUM]={RELAY_A_PIN,RELAY_B_PIN};
String state_str[]={
"OFF",
"ON "
};

enum screen {
MAN,
AUT,
CONF,
TEM,
TIME,
PER,
INVALID
};

#define MAX_SCR_NUM 6
String modex[]={
"MAN",
"AUT",
"CONF",
"TEM:",
"SET CURRENT TIME",
"PERIOD:"
};

enum key_pressed_t {
  KEY_MODE,
  KEY_NEXT,
  KEY_BACK,
  KEY_OK,
  KEY_INVALID
};

extern void key_hw_init();
extern key_pressed_t key_getcode();


#define LOAD_SIZE       sizeof(byte)
#define MODE_SIZE       sizeof(screen)
#define TEM_SIZE        sizeof(byte)
#define PER_SIZE        sizeof(byte)
#define CONTROL_SIZE    MAX_LOAD_NUM*LOAD_SIZE + MODE_SIZE + TEM_SIZE + PER_SIZE
#define EEPROM_ADDR     0
#define LOAD1_ADDR      EEPROM_ADDR
#define LOAD2_ADDR      LOAD1_ADDR+LOAD_SIZE
#define MODE_ADDR       LOAD2_ADDR+LOAD_SIZE
#define TEM_ADDR        MODE_ADDR+MODE_SIZE
#define PER_ADDR        TEM_ADDR+TEM_SIZE

union Setting
{
    //the dump arrays has the same size with structure Control.
    //this array is used to interact with eeprom in byte
    byte dump[CONTROL_SIZE];
    struct Control{
        byte load[MAX_LOAD_NUM];
        byte tem;       // 18-40 control temperature
        byte per;       // 00-23
        screen mode;
    }control;
};

Setting setting;

#endif // _HEADERFILE_H    // Put this line at the end of your file.

//  0123456789012345
// ******************
// *MAN  xxC  hh:mm *
// *  A:OFF  B:OFF  *
// ******************

//  0123456789012345
// ******************
// *AUT  A:OFF B:OFF*
// *xxC hh:mm  xx/xx*   per/dur
// ******************

//  0123456789012345
// ******************
// *CONF: MAN       *
// *MAN AUT SETTING *
// ******************

//  0123456789012345
// ******************
// *TEM:  xxC       *
// *    (18-40)     *
// ******************

//  0123456789012345
// ******************
// *SET CURRENT TIME*
// *    >hh:mm<     *
// ******************

//  0123456789012345
// ******************
// *DURATION:  hh   *
// *    (00-23)     *
// ******************

// ---------------|_________________|-----------------|______________
// ____________________|------------------|________________|---------------