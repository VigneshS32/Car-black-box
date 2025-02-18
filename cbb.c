/* 
 * Name : Vignesh S
 * Date : 10:02:2025
 * project : Car black box
 * description : Black Boxes are typically used in any transportation system 
 * that are used for analysis post-crash and understand the root cause of accidents.
 * Continuous monitoring and logging of events is critical 
 * for effective usage of black box. The goal of this project is to implement 
 * core functionalities of a care black-box in a PIC based micro-controller 
 * supported by rich peripherals. Events will be logged in EEPROM in this project.
 * This project can be further extended to any vehicle.
 * 
 * 
 * default password : 1111 (press up key 4 times RB3 )
 */
#include <xc.h>
#include "ds1307.h"
#include "i2c.h"
#include "clcd.h"
#include "EEprom.h"
#include <string.h>
#include "cbb.h"
#include "digital_keypad.h"
#include "uart.h"
unsigned char clock_reg[3];
unsigned char log[12];
unsigned char pos = 0,pwd_flag = 1;
char time[7],s_time[3];  // "HH:MM:SS"
extern unsigned char return_time;
unsigned char sec = 60;
char *menu[]= { "View Log", "Clear Log", "Download Log","Set Time    ", "Change Pass   " };
static void get_time(unsigned char *clock_reg)
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    clcd_putch(time[0],LINE2(0));
    clcd_putch(time[1],LINE2(1));
    clcd_putch(':',LINE2(2));
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    clcd_putch(time[2],LINE2(3));
    clcd_putch(time[3],LINE2(4));
    
    clcd_putch(':',LINE2(5));
    
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    clcd_putch(time[4],LINE2(6));
    clcd_putch(time[5],LINE2(7));
    time[6] = '\0';
}

void display_dash_board(char event[], unsigned short speed)
{
    clcd_print(" TIME     EV  SP",LINE1(0));
    get_time(clock_reg);
    clcd_print(event,LINE2(10));
    clcd_putch((speed / 10) + '0',LINE2(14));
    clcd_putch((speed % 10) + '0',LINE2(15));
    
}
void log_event(void)
{
    unsigned char addr;
    addr = pos * 11 + 5;
    if(++pos == 10)
        pos = 0;
    ext_eeprom_24C02_str_write(addr, &log);
}
void log_car_event (char *event ,unsigned short speed)
{
    //store time , event and speed
    get_time(clock_reg);
    log[0] = pos + '0';
    strncpy(&log[1],time,6);
    strncpy(&log[7],event,2);
    
    log[9] = speed/10 + '0';    
    log[10] = speed %10 + '0';
    log[11] = '\0';

    log_event();
}
unsigned char login_screen(unsigned char key,unsigned char reset_flag)
{
    static unsigned char user_password[4],saved_password[5];
    static unsigned char i, attempt = 3;
    unsigned char add=0;
    
    // default password
    if(pwd_flag)
    {
    if(reset_flag == RESET_PASSWORD)
    {
        i=0;
        user_password[0] = '\0';
        user_password[1] = '\0';
        user_password[2] = '\0';
        user_password[3] = '\0';
        saved_password[0] = '1';
        saved_password[1] = '1';
        saved_password[2] = '1';
        saved_password[3] = '1';
        return_time = 5;
        key = 0;
    }
    if(return_time == 0)
    {
        return RETURN_BACK;
    }
    }
    else
    {
    // eeprom password
    if(reset_flag == RESET_PASSWORD)
    {
        i=0;
        user_password[0] = '\0';
        user_password[1] = '\0';
        user_password[2] = '\0';
        user_password[3] = '\0';
        for(unsigned char j = 0; j<4; j++)
        {
        saved_password[j] = ext_eeprom_24C02_read(add+j);
        }
        return_time = 5;
        key = 0;
    }
    if(return_time == 0)
    {
        return RETURN_BACK;
    }
    }
    // read password
    if(key == SW4 && i<4)
    {
        user_password[i++] = '1';
        clcd_putch('*',LINE2(i + 4));
        return_time = 5;
    }
    else if (key == SW5 && i<4)
    {
        user_password[i++] = '0';
        clcd_putch('*',LINE2(i + 4));
    }
    //compare password
    if(i == 4)
    {
        if(strncmp(user_password,saved_password,4) == 0)
        {
           clear_screen();
            // turn off the cursor print success message, off timer, attempt 3
           clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
           __delay_us(100);
           clcd_print("Login Success",LINE1(0));
           __delay_ms(1500);
           
           TMR2ON = 0;
           return LOGIN_SUCESS; 
        }
        else
        {
            //reduce attempt
            attempt--;
            if(attempt == 0)
            {
                // no attempt // block code for 60 sec
                  clear_screen();
                  clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
                  __delay_us(100);
                  clcd_print("You are blocked",LINE1(0));
                  clcd_print("Wait for",LINE2(0));
                  sec = 60;
                  while(sec != 0)
                  {
                       clcd_putch((sec / 10) + '0',LINE2(9));
                       clcd_putch((sec % 10) + '0',LINE2(10));
                       clcd_print("sec", LINE2(12));
                  }
                  
                  attempt = 3;
                
            }
            else
            {
                // if attempt are left , display no of attempt left and read password
                  clear_screen();
                  clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
                  __delay_us(100);
                  clcd_print("Wrong password",LINE1(0));
                  clcd_print("Attempts left:",LINE2(0));
                  clcd_putch(attempt + '0',LINE2(14));
                  __delay_ms(1500);
                  
            }
            clear_screen();
            clcd_print("Enter password",LINE1(0));
            clcd_write(DISP_ON_AND_CURSOR_ON,INST_MODE);
            i = 0;
            return_time = 5;
            //return RETURN_BACK;
        }
    }
}
// implementing menu screen
unsigned char menu_screen(unsigned char key,unsigned char reset_flag)
{
    static unsigned char menu_pos = 0; //menu[0],menu[1]
    static unsigned char select_pos = 1;
    if(reset_flag == RESET_LOGIN_MENU)
    {
        menu_pos = 0;
        select_pos = 1;
    }
    // based on key press scroll up and down the menu
    if(key == SW4 && menu_pos < 4)
    {
        menu_pos++;
        // move to line2    
        if(select_pos < 2)
            clear_screen();
            select_pos++;  
    }
    else if (key == SW5 && menu_pos > 0)
    {
        menu_pos--;
        //move to line1
        if(select_pos > 1)
            clear_screen();
            select_pos--;
    }
    if(select_pos == 1)
    {
     //print star on first line 
        clcd_putch('*',LINE1(0));
        clcd_putch(' ',LINE2(0));
        clcd_print(menu[menu_pos],LINE1(2));//   *view log 
        clcd_print(menu[menu_pos+1],LINE2(2));// clear log
    }
    else 
    {
     //print star on first line 
        clcd_putch(' ',LINE1(0));
        clcd_putch('*',LINE2(0));
        clcd_print(menu[menu_pos-1],LINE1(2));// view log 
        clcd_print(menu[menu_pos],LINE2(2));// *clear log
    }
    return menu_pos;
    
}
void view_log(unsigned char key,unsigned char reset_flag)
{
    static unsigned char shift = 0;
    char rlog[12];
    unsigned char add;
    //read the log from eeprom and displayed in clcd
    if(reset_flag == RESET_VIEW_LOG_POS)
    {
        shift = 0;
    }
    else if(key == SW4 && shift <10)
        shift++;
    else if(key == SW5 && shift > 0)
        shift--;
    //read data from eeprom 
    add = shift * 11 + 5;
    for(unsigned char i = 0; i<11; i++)
    {
        rlog[i] = ext_eeprom_24C02_read(add+i);
    }
    rlog[11] = '\0'; 
    //display it on clcd
    if(rlog[0] != 0x00)
    {
    clcd_print("# TIME     E  SP", LINE1(0));
    clcd_putch(rlog[0], LINE2(0));
    clcd_putch(' ', LINE2(1));
    clcd_putch(rlog[1], LINE2(2));
    clcd_putch(rlog[2], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(rlog[3], LINE2(5));
    clcd_putch(rlog[4], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(rlog[5], LINE2(8));
    clcd_putch(rlog[6], LINE2(9));
    clcd_putch(' ', LINE2(10));
    clcd_putch(rlog[7], LINE2(11));
    clcd_putch(rlog[8], LINE2(12));
    clcd_putch(' ', LINE2(13));
    clcd_putch(rlog[9], LINE2(14));
    clcd_putch(rlog[10], LINE2(15));
    }
    
}
void clear_log(void)
{
    clear_screen();
    clcd_print("CLEAR LOG", LINE1(0));
    clcd_print("SUCCESSFULL", LINE2(0));
    unsigned char clear = 0x00;
    for(unsigned char i = 5; i<=120; i++)
    {
        ext_eeprom_24C02_byte_write(i,clear);
    }
    pos = 0;
    __delay_ms(1500);
    clear_screen();
}
void download_log(void)
{
    puts("# TIME     E  SP\n\r");
    char rlog[12];
    unsigned char add = 5;
    for(unsigned char j=0;j<10;j++)
    {
    for(unsigned char i = 0; i<11; i++)
    {
        rlog[i] = ext_eeprom_24C02_read(add+i);
    }
    add += 11;
    rlog[11] = '\0'; 
    if(rlog[0] == 0x00)
        break;
    putchar(rlog[0]);
    putchar(' ');
    putchar(rlog[1]);
    putchar(rlog[2]);
    putchar(':');
    putchar(rlog[3]);
    putchar(rlog[4]);
    putchar(':');
    putchar(rlog[5]);
    putchar(rlog[6]);
    putchar(' ');
    putchar(rlog[7]);
    putchar(rlog[8]);
    putchar(' ');
    putchar(rlog[9]);
    putchar(rlog[10]);
    putchar('\n');
    putchar('\r');
    
    }
    clear_screen();
    clcd_print("DOWNLOAD LOG", LINE1(0));
    clcd_print("SUCCESSFULLY", LINE2(0));
    __delay_ms(1500);
}
unsigned char set_time(void)
{
    static unsigned char shift = 0,key;
    unsigned char sec,min,hr;
    clear_screen();
    while(1)
    {
        key = read_digital_keypad(STATE);
        clcd_print("TIME (HH:MM:SS)", LINE1(0));
        clcd_putch(time[0], LINE2(0));
        clcd_putch(time[1], LINE2(1));
        clcd_putch(':', LINE2(3));
        clcd_putch(time[2], LINE2(5));
        clcd_putch(time[3], LINE2(6));
        clcd_putch(':', LINE2(8));
        clcd_putch(time[4], LINE2(10));
        clcd_putch(time[5], LINE2(11));
        if(key == SW5)
        {
            if(++shift == 3)
                shift = 0;
        }
        switch(shift)
        {
            case 2:
                    clcd_putch(' ', LINE2(10)); 
                    clcd_putch(' ', LINE2(11)); 
                    sec = (time[4] - '0') * 10;
                    sec += (time[5]-'0');
                    if(key == SW4)
                    {
                        if(++sec > 59)
                            sec = 0;
                    }
                    time[4] = (sec /10) + '0';
                    time[5]= (sec % 10) + '0';
                break;
            case 1:
                    clcd_putch(' ', LINE2(5)); 
                    clcd_putch(' ', LINE2(6));  
                    min = (time[2] - '0') * 10;
                    min += (time[3]-'0');
                    if(key == SW4)
                    {
                        if(++min > 59)
                            min = 0;
                    }
                    time[2] = (min /10) + '0';
                    time[3] = (min % 10) + '0';
                break;
            case 0:
                    clcd_putch(' ', LINE2(0)); 
                    clcd_putch(' ', LINE2(1));  
                    hr = (time[0] - '0') * 10;
                    hr += (time[1]-'0');
                    if(key == SW4)
                    {
                        if(++hr > 23)
                            hr = 0;
                    }
                    time[0] = (hr /10) + '0';
                    time[1] = (hr % 10) + '0';
                break;      
        }
        clcd_print("TIME (HH:MM:SS)", LINE1(0));
        clcd_putch(time[0], LINE2(0));
        clcd_putch(time[1], LINE2(1));
        clcd_putch(':', LINE2(3));
        clcd_putch(time[2], LINE2(5));
        clcd_putch(time[3], LINE2(6));
        clcd_putch(':', LINE2(8));
        clcd_putch(time[4], LINE2(10));
        clcd_putch(time[5], LINE2(11));
        if(key == L_SW4)
        {
            clear_screen();
            clcd_print("TIME SET", LINE1(0));
            clcd_print("SUCCESSFULLY", LINE2(0));
            write_ds1307(SEC_ADDR,sec);
            write_ds1307(MIN_ADDR,min);
            write_ds1307(HOUR_ADDR,hr);
            __delay_ms(1500);
            clear_screen();
            return TASK_SUCESS;
        }
        else if(key == L_SW5)
        {
            clear_screen();
            break;
        }
    }
}
unsigned char change_password(void)
{
    unsigned char pwd_arr[5],res;
    clear_screen();
    clcd_print("ENTER OLD PWD", LINE1(0));
    res = compare_pwd(RESET_PASSWORD);
    if(res == TASK_SUCESS)
    {
    clear_screen();
    clcd_print("ENTER NEW PWD", LINE1(0));
    __delay_ms(1500);
    clear_screen();
    clcd_print("4 DIGIT TO SET", LINE1(0));
    clcd_print("OR CANCEL", LINE2(0));
    __delay_ms(1500);
    clear_screen();
    clcd_print("ENTER NEW PWRD", LINE1(0));
    unsigned char key = 0x00,i = 0,flag = 0,add=0;
    clcd_write(DISP_ON_AND_CURSOR_ON,INST_MODE);
    while(1)
    {
        key = read_digital_keypad(STATE);
        for(unsigned int wait = 1000;wait--;);
        if(key == SW4 && i<4)
        {
            pwd_arr[i++] = '1';
            clcd_putch('*', LINE2(i+4));
            if(i == 4)
                flag =1;
        }
        else if(key == SW5 && i<4)
        {
            pwd_arr[i++] = '0';
            clcd_putch('*', LINE2(i+4));
            if(i == 4)
                flag =1;
        }
        pwd_arr[i] = '\0';
        if(key == L_SW4 && flag)
        {
            clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
            if(i == 4)
            {
            ext_eeprom_24C02_str_write(add,&pwd_arr);
            for(unsigned char wait = 100;wait--;);
            clear_screen();
            clcd_print("PASSWORD CHANGED", LINE1(0));
            clcd_print("SUCCESSFULLY", LINE2(0));
            pwd_flag = 0;
            __delay_ms(1500);
            clear_screen();
            return TASK_SUCESS;
            }
        }
        else if(key == L_SW5 && flag)
        {
            clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
            clear_screen();
            break;
        }
    }
    }
    else
    {
        return TASK_FAIL;
    }
}
unsigned char compare_pwd(unsigned char reset_flag)
{
    static unsigned char user_pwd[4],saved_pwd[5];
    static unsigned char i;
    unsigned char add=0,key;
    // eeprom password
    if(pwd_flag)
    {
        if(reset_flag == RESET_PASSWORD)
        {
            i=0;
            user_pwd[0] = '\0';
            user_pwd[1] = '\0';
            user_pwd[2] = '\0';
            user_pwd[3] = '\0';
            saved_pwd[0] = '1';
            saved_pwd[1] = '1';
            saved_pwd[2] = '1';
            saved_pwd[3] = '1';
            return_time = 5;
            key = 0;
        }
        if(return_time == 0)
        {
            return RETURN_BACK;
        }
    }
    else
    {
    if(reset_flag == RESET_PASSWORD)
    {
        i=0;
        user_pwd[0] = '\0';
        user_pwd[1] = '\0';
        user_pwd[2] = '\0';
        user_pwd[3] = '\0';
        for(unsigned char j = 0; j<4; j++)
        {
        saved_pwd[j] = ext_eeprom_24C02_read(add+j);
        }
        return_time = 5;
        key = 0;
    }
    if(return_time == 0)
    {
        return RETURN_BACK;
    }
    }
    // read password
    while(1)
    {
    key = read_digital_keypad(STATE);
    for(unsigned int wait = 500;wait--;);
    if(i == 4)
        break;
    if(key == SW4 && i<4)
    {
        user_pwd[i++] = '1';
        clcd_putch('*',LINE2(i + 4));
        return_time = 5;
    }
    else if (key == SW5 && i<4)
    {
        user_pwd[i++] = '0';
        clcd_putch('*',LINE2(i + 4));
    }
    }
    //compare password
    if(i == 4)
    {
        if(strncmp(user_pwd,saved_pwd,4) == 0)
        {
           clear_screen();
            // turn off the cursor print success message, off timer, attempt 3
           clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
           __delay_us(100);
           clcd_print("GRANTED",LINE1(0));
           __delay_ms(1500);
           
           TMR2ON = 0;
           return TASK_SUCESS; 
        }
        else
        {
            clear_screen();
            clcd_print("INVALID",LINE1(0));
           __delay_ms(1500);
            return TASK_FAIL;  
        }
    }
}
void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
}