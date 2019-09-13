#include "main.h"
#include "DS3231.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

#define ONE_WIRE_BUS 2

/*****************RTC ****************/
byte pre_load = 0;
/*************************************/

/*****************RTC ****************/
char Cenc = 0xDF; //*C
char nr =0x7E;// ->
char nl =0x7F;// <-
LiquidCrystal_I2C lcd(0x3f,20,4);
/*************************************/
char line1[16];
char line2[16];
/*****************RTC ****************/
// Init a Time-data structure
RTClib RTC;
DateTime now;
byte start_hh=0;
byte dur=0;
DS3231 Clock;
/*************************************/


/*****************temperature ****************/
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************/

extern Setting setting;

void setup() {
    int i;

    delay(1000);

    sensors.begin();            //init DS18B20
    Wire.begin();               //init DS3231

    lcd.init();                 //init LCD
    lcd.backlight();            //init backlight
    lcd.home();                 //cursor to home
    lcd.clear();                //clear screen

    key_hw_init();              //init button key_hw_init()

    pinMode(load[0], OUTPUT);   //set pin as output
    pinMode(load[1], OUTPUT);   //set pin as output
    digitalWrite(load[0],LOW);  //make sure two aircon is off at startup
    digitalWrite(load[1],LOW);  //make sure two aircon is off at startup

    //load configuration
    for(i=0;i<CONTROL_SIZE;i++)
        setting.dump[i]=EEPROM.read(EEPROM_ADDR+i);

    //for the first time read, we dont know the content of eeprom
    //fix the mode to be manual first
    if( (setting.control.mode != MAN) && (setting.control.mode != AUT) )
    {
        setting.control.mode = MAN;
    }
}

void loop()
{
    switch (setting.control.mode)
    {
        case MAN:
            scr_man();
        break;
        case AUT:
            scr_aut();
        break;
        case CONF:
            scr_conf();
        break;
        case TEM:
            scr_tem();
        break;
        case TIME:
            scr_time();
        break;
        case PER:
            scr_per();
        break;
        default:
        break;
    }
}

//Manual control screen
void scr_man(void)
{
    key_pressed_t key;
    byte t;
    setting.control.mode = MAN;
    int i;

    // setting.control.mode print mode
    lcd.clear();
    lcd.setCursor(0,0);
    if((setting.control.load[0] != ON)&&(setting.control.load[0] != OFF))
    {
        setting.control.load[0] = ON;
    }
    if((setting.control.load[1] != ON)&&(setting.control.load[1] != OFF))
    {
        setting.control.load[1] = OFF;
    }
    memset(line1,' ',sizeof(line1));
    memset(line2,' ',sizeof(line2));
    memcpy(line1,&modex[setting.control.mode][0],modex[setting.control.mode].length());
    memcpy(&line2[2],"A:",sizeof("A:")-1);
    memcpy(&line2[9],"B:",sizeof("B:")-1);

    memcpy(&line2[4],&state_str[setting.control.load[0]][0],state_str[setting.control.load[0]].length());
    digitalWrite(load[0],setting.control.load[0]);

    memcpy(&line2[11],&state_str[setting.control.load[1]][0],state_str[setting.control.load[1]].length());
    digitalWrite(load[1],setting.control.load[1]);

    // //write to eeprom the current mode and load status
    for(i=0;i<CONTROL_SIZE;i++)
        EEPROM.update(EEPROM_ADDR+i,setting.dump[i]);

    delay(1000);
    while(MAN == setting.control.mode)
    {
        //read current time from ds3231
        now = RTC.now();
        line1[11]=now.hour()/10+'0';
        line1[12]=now.hour()%10+'0';
        line1[13]=':';
        line1[14]=now.minute()/10+'0';
        line1[15]=now.minute()%10+'0';

        sensors.requestTemperatures();
        t = (byte)(sensors.getTempCByIndex(0)+0.5);
        line1[4] = t/10+'0';
        line1[5] = t%10+'0';
        line1[6] = Cenc;


        // scan key
        delay(1000);
        key = key_getcode();
        if(KEY_INVALID != key)
        {
            switch (key)
            {
                case KEY_MODE:
                    // switch to configuration screen
                    setting.control.mode = CONF;
                break;
                case KEY_NEXT:
                    if(setting.control.load[0] == ON)
                        setting.control.load[0] = OFF;
                    else
                        setting.control.load[0] = ON;
                    memcpy(&line2[4],&state_str[setting.control.load[0]][0],state_str[setting.control.load[0]].length());
                    digitalWrite(load[0], setting.control.load[0]);
                    for(i=0;i<LOAD_SIZE;i++)
                        EEPROM.update(LOAD1_ADDR+i,setting.dump[LOAD1_ADDR+i]);
                break;
                case KEY_BACK:
                    if(setting.control.load[1] == ON)
                        setting.control.load[1] = OFF;
                    else
                        setting.control.load[1] = ON;
                    memcpy(&line2[11],&state_str[setting.control.load[1]][0],state_str[setting.control.load[1]].length());
                    digitalWrite(load[1], setting.control.load[1]);
                    for(i=0;i<LOAD_SIZE;i++)
                        EEPROM.update(LOAD2_ADDR+i,setting.dump[LOAD2_ADDR+i]);
                break;
                case KEY_OK:
                    // switch to configuration screen
                    setting.control.mode = CONF;
                break;
                default:
                break;
            }
        }
        lcd.setCursor(0,0);
        lcd.print(line1);
        lcd.setCursor(0,1);
        lcd.print(line2);
    }
}

//this function can only be called in aut mode
void control_task(byte t)
{
    int i;
    //make sure we are in aut mode
    if( AUT != setting.control.mode ) return;
    now = RTC.now();
    dur = (now.hour() >= start_hh) ? (now.hour() - start_hh) : (now.hour() + 24 - start_hh);
    line2[14] = dur/10 + '0';
    line2[15] = dur%10 + '0'; 

    if(t > setting.control.tem)
    {
        // turn on both aircons
        // no need to follow schedule because this is priority
        if( OFF == setting.control.load[0] )
        {
            pre_load = 0;
            setting.control.load[0] = ON;
            digitalWrite(load[pre_load],HIGH);
            for(i=0;i<LOAD_SIZE;i++)
                EEPROM.update(LOAD1_ADDR+i,setting.dump[LOAD1_ADDR+i]);
        }
        if( OFF == setting.control.load[1] )
        {
            pre_load = 1;
            setting.control.load[1] = ON;
            digitalWrite(load[pre_load],HIGH);
            for(i=0;i<LOAD_SIZE;i++)
                EEPROM.update(LOAD2_ADDR+i,setting.dump[LOAD2_ADDR+i]);
        }
    }
    else
    {
        //if it's time to do switching, do switching (dur >= per)
        //if not yet, make sure only one aircon running
        if(dur >= setting.control.per)
        {
            start_hh = now.hour();
            dur = 0;
            if( (ON == setting.control.load[0]) && (ON == setting.control.load[1]) )
            {
                // turn off one aircon
                setting.control.load[pre_load] = OFF;
                digitalWrite(load[pre_load],LOW);
                for(i=0;i<LOAD_SIZE;i++)
                    EEPROM.update(pre_load*LOAD1_ADDR+i,setting.dump[pre_load*LOAD1_ADDR+i]);
            }
            else if( (OFF == setting.control.load[0]) && (OFF == setting.control.load[1]) )
            {
                // no, for this design, we can never get here
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("OPERATION ERROR");
                lcd.setCursor(0, 1);
                lcd.print("CALL SUPPORT !!!");
                while(1);
            }
            else
            {
                //switch 2 aircons
                if(setting.control.load[0] == ON)
                    setting.control.load[0] = OFF;
                else
                    setting.control.load[0] = ON;
                if(setting.control.load[1] == ON)
                    setting.control.load[1] = OFF;
                else
                    setting.control.load[1] = ON;
                digitalWrite(load[0],setting.control.load[0]);
                for(i=0;i<LOAD_SIZE;i++)
                    EEPROM.update(LOAD1_ADDR+i,setting.dump[LOAD1_ADDR+i]);
                digitalWrite(load[1],setting.control.load[1]);
                for(i=0;i<LOAD_SIZE;i++)
                    EEPROM.update(LOAD2_ADDR+i,setting.dump[LOAD2_ADDR+i]);
            }
        }
        else
        {
            //if two aircon are both previously auto turn on
            //and the temperature get back to normal
            if( (ON == setting.control.load[0]) && (ON == setting.control.load[1]) && (t < (setting.control.tem-1)))
            {
                // turn off one aircon
                setting.control.load[pre_load] = OFF;
                digitalWrite(load[pre_load],LOW);
                for(i=0;i<LOAD_SIZE;i++)
                    EEPROM.update(pre_load*LOAD1_ADDR+i,setting.dump[pre_load*LOAD1_ADDR+i]);
            }
        }
    }
}

//Auto control screen
void scr_aut(void)
{
    int i;
    key_pressed_t key;
    byte t;
    byte tem;
    // setting.control.mode print mode

    setting.control.mode = AUT;
    lcd.clear();
    lcd.setCursor(0,0);
    if((setting.control.per > MAX_PER)||(setting.control.per < MIN_PER))
    {
        lcd.print("PERIOD = ???");
        delay(3000);
        setting.control.mode = CONF;
        return;
    }
    if((setting.control.tem > MAX_TEM)||(setting.control.tem < MIN_TEM))
    {
        lcd.print("TEMPERATURE = ???");
        delay(3000);
        setting.control.mode = CONF;
        return;
    }
    tem = setting.control.tem;
    if((setting.control.load[0] != ON)&&(setting.control.load[0] != OFF))
    {
        setting.control.load[0] = ON;
    }
    if((setting.control.load[1] != ON)&&(setting.control.load[1] != OFF))
    {
        setting.control.load[1] = OFF;
    }

    memset(line1,' ',sizeof(line1));
    memset(line2,' ',sizeof(line2));
    memcpy(line1,&modex[setting.control.mode][0],modex[setting.control.mode].length());
    memcpy(&line1[5],"A:",sizeof("A:")-1);
    memcpy(&line1[11],"B:",sizeof("B:")-1);
    now = RTC.now();
    start_hh = now.hour();

    //load status
    memcpy(&line1[7],&state_str[setting.control.load[0]][0],state_str[setting.control.load[0]].length());
    memcpy(&line1[13],&state_str[setting.control.load[1]][0],state_str[setting.control.load[1]].length());
    digitalWrite(load[0],setting.control.load[0]);
    digitalWrite(load[1],setting.control.load[1]);
    //write to eeprom the current mode and load status
    for(i=0;i<CONTROL_SIZE;i++)
        EEPROM.update(EEPROM_ADDR+i,setting.dump[i]);

    delay(1000);
    while(AUT == setting.control.mode)
    {
        //read current time from ds3231
        now = RTC.now();
        line2[4]=now.hour()/10+'0';
        line2[5]=now.hour()%10+'0';
        line2[6]=':';
        line2[7]=now.minute()/10+'0';
        line2[8]=now.minute()%10+'0';

        //read current temperature
        sensors.requestTemperatures();
        t = (byte)(sensors.getTempCByIndex(0)+0.5);
        line2[0] = t/10+'0';
        line2[1] = t%10+'0';
        line2[2] = Cenc;

        lcd.setCursor(11,1);
        line2[11] = setting.control.per/10 + '0';
        line2[12] = setting.control.per%10 + '0';
        line2[13] = '/';
        line2[14] = dur/10 + '0';
        line2[15] = dur%10 + '0';

        //control load1/load2 base on temperature and scheduler
        control_task(t);

        //load status
        memcpy(&line1[7],&state_str[setting.control.load[0]][0],state_str[setting.control.load[0]].length());
        memcpy(&line1[13],&state_str[setting.control.load[1]][0],state_str[setting.control.load[1]].length());

        //scan key
        delay(1000);
        key = key_getcode();
        if(KEY_INVALID != key)
        {
            switch (key)
            {
                case KEY_MODE:
                    // switch to configuration screen
                    setting.control.mode = CONF;
                break;
                case KEY_OK:
                    // switch to configuration screen
                    setting.control.mode = CONF;
                break;

                default:
                break;
            }
        }
        lcd.setCursor(0,0);
        lcd.print(line1);
        lcd.setCursor(0,1);
        lcd.print(line2);
    }
}

//configuration screen
void scr_conf(void)
{
    screen sel = MAN;
    key_pressed_t key;
    setting.control.mode = CONF;

    lcd.clear();
    memset(line1,' ',sizeof(line1));
    memset(line2,' ',sizeof(line2));
    memcpy(&line1[0],"MODE:",sizeof("MODE:")-1);
    memcpy(&line2[0],"MAN AUT CONF ",sizeof("MAN AUT CONF ")-1);
    memcpy(&line1[6],&modex[sel][0],modex[sel].length());
    lcd.setCursor(0,0);
    lcd.print(line1);
    lcd.setCursor(0,1);
    lcd.print(line2);
    delay(1000);
    while(CONF == setting.control.mode)
    {
        delay(1000);
        key = key_getcode();
        if(KEY_INVALID != key)
        {
            switch (key)
            {
                case KEY_NEXT:
                    switch (sel)
                    {
                        case MAN:
                            sel = AUT;
                        break;
                        case AUT:
                            sel = CONF;
                        break;
                        case CONF:
                            sel = MAN;
                        break;
                        default:
                        break;
                    }
                break;
                case KEY_BACK:
                    switch (sel)
                    {
                        case MAN:
                            sel = CONF;
                        break;
                        case AUT:
                            sel = MAN;
                        break;
                        case CONF:
                            sel = AUT;
                        break;
                        default:
                        break;
                    }
                break;
                case KEY_OK:
                    setting.control.mode = (CONF == sel) ? (TEM):(sel);
                break;
                default:
                break;
            }
        }
        memcpy(&line1[6],"    ",4);
        memcpy(&line1[6],&modex[sel][0],modex[sel].length());
        lcd.setCursor(0,0);
        lcd.print(line1);
        lcd.setCursor(0,1);
        lcd.print(line2);
    }
}

//configuration temperature screen
void scr_tem(void)
{
    int i;
    key_pressed_t key;
    byte temp = setting.control.tem;
    setting.control.mode = TEM;

    lcd.clear();
    memset(line1,' ',sizeof(line1));
    memset(line2,' ',sizeof(line2));
    memcpy(line1,&modex[setting.control.mode][0],modex[setting.control.mode].length());
    memcpy(line2,"    (18-40)     ",16);
    line1[6] = temp/10 + '0';
    line1[7] = temp%10 + '0';

    delay(1000);
    while(TEM == setting.control.mode)
    {
        delay(1000);
        key = key_getcode();
        if(KEY_INVALID != key)
        {
            switch (key)
            {
                case KEY_NEXT:
                    if( temp < MAX_TEM )    temp += 1;
                break;
                case KEY_BACK:
                    if( temp > MIN_TEM )    temp -= 1;
                break;
                case KEY_OK:
                    if( (temp <= MAX_TEM) && (temp >= MIN_TEM) )
                    {
                        setting.control.tem = temp;
                        //write to eeprom the configured temperature
                        for(i=0;i<TEM_SIZE;i++)
                            EEPROM.update(TEM_ADDR+i,setting.dump[TEM_ADDR+i]);
                        setting.control.mode = TIME;
                    }
                break;
                default:
                break;
            }
            memcpy(&line1[6],"    ",4);
            line1[6] = temp/10 + '0';
            line1[7] = temp%10 + '0';
        }
        lcd.setCursor(0,0);
        lcd.print(line1);
        lcd.setCursor(0,1);
        lcd.print(line2);
    }
}

//show selection
void show_sel(byte sel)
{
    switch (sel)
    {
        case 0:
            line2[4] = nr;
            line2[10] = ' ';
        break;
        case 1:
            line2[10] = nl;
            line2[4] = ' ';
        break;
        default:
        break;
    }
}

//calib system time screen
void scr_time(void)
{
    byte sel = 0;   //0-hh,1-mm
    key_pressed_t key;
    byte temp_hh=0;
    byte temp_mm=0;
    setting.control.mode = TIME;
    //print setting.control.mode4
    lcd.clear();
    memset(line1,' ',sizeof(line1));
    memset(line2,' ',sizeof(line2));
    memcpy(line1,&modex[setting.control.mode][0],modex[setting.control.mode].length());
    now = RTC.now();
    temp_hh = now.hour();
    temp_mm = now.minute();
    line2[4] = nr;
    line2[5] = temp_hh/10 + '0';
    line2[6] = temp_hh%10 + '0';
    line2[7] = ':';
    line2[8] = temp_mm/10 + '0';
    line2[9] = temp_mm%10 + '0';

    delay(1000);
    while(TIME == setting.control.mode)
    {
        delay(1000);
        key = key_getcode();
        if(KEY_INVALID != key)
        {
            switch (key)
            {
                case KEY_MODE:
                    if(0 == sel){
                        sel = 1;    //mm
                    }
                    else{
                        sel = 0;    //hh
                    }
                break;
                case KEY_NEXT:
                    switch (sel)
                    {
                        case 0: //hh
                            temp_hh = (temp_hh < 23)?(temp_hh += 1):(temp_hh=0);
                        break;
                        case 1: //mm
                            temp_mm = (temp_mm < 59)?(temp_mm += 1):(temp_mm=0);
                        break;
                        default:
                        break;
                    }
                break;
                case KEY_BACK:
                    switch (sel)
                    {
                        case 0: //hh
                            temp_hh = (temp_hh > 0)?(temp_hh -= 1):(temp_hh=23);
                        break;
                        case 1: //mm
                            temp_mm = (temp_mm > 0)?(temp_mm -= 1):(temp_mm=59);
                        break;
                        default:
                        break;
                    }
                break;
                case KEY_OK:
                    // write temp_hh, temp_mm to ds3231
                    Clock.setHour(temp_hh);
		            Clock.setMinute(temp_mm);
                    setting.control.mode = PER;
                break;
                default:
                break;
            }
            show_sel(sel);
            line2[5] = temp_hh/10 + '0';
            line2[6] = temp_hh%10 + '0';
            line2[7] = ':';
            line2[8] = temp_mm/10 + '0';
            line2[9] = temp_mm%10 + '0';
        }
        lcd.setCursor(0,0);
        lcd.print(line1);
        lcd.setCursor(0,1);
        lcd.print(line2);
    }
}


//configuration schedule screen
void scr_per()
{
    int i;
    byte tper = setting.control.per;
    key_pressed_t key;
    screen tmode = setting.control.mode;

    //print setting.control.mode2
    lcd.clear();
    memset(line1,' ',sizeof(line1));
    memset(line2,' ',sizeof(line2));
    memcpy(line1,&modex[setting.control.mode][0],modex[setting.control.mode].length());
    memcpy(line2,"    (01-24)     ",16);
    line1[11] = tper/10 + '0';
    line1[12] = tper%10 + '0';

    delay(1000);
    while( tmode == setting.control.mode )
    {
        delay(1000);
        key = key_getcode();
        if(KEY_INVALID != key)
        {
            switch (key)
            {
                case KEY_NEXT:
                    tper = (tper < MAX_PER) ? (tper += 1) : (tper=1);
                break;
                case KEY_BACK:
                    tper = (tper > MIN_PER) ? (tper -= 1) : (tper=24);
                break;
                case KEY_OK:
                    if( (tper <= MAX_PER) && (tper >= MIN_PER) )
                    {
                        setting.control.per = tper;
                        for(i=0;i<PER_SIZE;i++)
                            EEPROM.update(PER_ADDR+i,setting.dump[PER_ADDR+i]);
                        setting.control.mode = CONF;
                    }
                break;
                default:
                break;
            }
            line1[11] = tper/10 + '0';
            line1[12] = tper%10 + '0';
        }
        lcd.setCursor(0,0);
        lcd.print(line1);
        lcd.setCursor(0,1);
        lcd.print(line2);
    }
}