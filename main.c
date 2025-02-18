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
 * 
 */
#include <xc.h>
#include "adc.h"
#include "cbb.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include <string.h>
#include "timers.h"
#include "uart.h"
#pragma config WDTE = OFF
char *gear[] = {"GN","GR","G1","G2","G3","G4","G5"};
unsigned char return_time = 5;
void init_config(void)
{
/*initialize required pheripherals */
    init_clcd();
    init_digital_keypad();
//    init_ds1307();
    init_adc();
    init_i2c(100000);
    init_timer2();
    // init_ uart
    init_uart(9600);
    // ENABLE GLOBAL AND PHERIPERAL INT
    PEIE = 1;
    GIE = 1;
    
}

void main(void)
{
init_config();

unsigned char key,menu_pos,stat;
/*first event */
char event[3] = "ON";
/*initially gear index*/
int gr = 0;
unsigned char speed = 0;
/*flag to indicate the screen to start with screen is dhashboard*/
char control_flag = DASH_BOARD_FLAG;

char reset_flag = RESET_NOTHING ;

/*menu position index*/
//int menu_pos = 0;
//int wait_time = 0;
log_car_event(event,speed);


while (1)
{

        speed = (unsigned char)(read_adc(CHANNEL0) / 10); // 0-1023 to 0-99
        if(speed > 99)
            speed = 99;
        
        /*read the key pressed and check with control_flag based that display the screen*/
        key = read_digital_keypad(STATE);  
        for(unsigned int wait = 500;wait--;);
        /*if gear increment key is pressed */
        if(key == SW1)
        {
            strcpy(event, " C");
            log_car_event(event,speed);
        }
        else if(key == SW2)
        {
            if(gr < 5)
            {
                gr++;
            }
            strcpy(event, gear[gr]);
            log_car_event(event,speed);
        }
        else if(key == SW3)
        {
            if(gr > 0)
            {
                gr--;
            }
            strcpy(event, gear[gr]);
            log_car_event(event,speed);
        }
        // check if sw4 or sw5 is pressed then change the screen
        else if(((key == SW5 || key == SW4)) && control_flag == DASH_BOARD_FLAG)
        {
            control_flag = LOGIN_FLAG;
            reset_flag = RESET_PASSWORD;
            clear_screen();
            clcd_print("Enter password",LINE1(0));
            clcd_write(DISP_ON_AND_CURSOR_ON,INST_MODE);
            
            //set timer
            TMR2ON = 1;
        }
        // in menu screen detect long press, based on menu pos change the screen
        else if(control_flag == MAIN_MENU_FLAG && key == L_SW4)
        {
            switch(menu_pos)
            {
                case 0:
                    // view log
                    clear_screen();
                    control_flag = VEIW_LOG_FLAG;
                    reset_flag = RESET_VIEW_LOG_POS;
                    break;
                case 1:
                    //clear log
                    control_flag = CLEAR_LOG_FLAG;
                    break;
                case 2:
                    // download log
                    control_flag = DOWNLOAD_LOG_FLAG;
                    break;
                case 3:
                    // set time
                    control_flag = SET_TIME_FLAG;
                    break;
                case 4:
                    // change password
                  control_flag = CHANGE_PASSWORD_FLAG;
                    break;
            }
        }
        else if(control_flag == MAIN_MENU_FLAG && key == L_SW5)
        {
                control_flag = DASH_BOARD_FLAG;
        }
        
        
        
        
        
        switch(control_flag)
        {
            case DASH_BOARD_FLAG : 
                display_dash_board(event,speed);
                break;
            case LOGIN_FLAG :
                switch(login_screen(key, reset_flag))
                {
                    case RETURN_BACK:
                                // go back to dashboard
                                control_flag = DASH_BOARD_FLAG;

                                clear_screen();
                                clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
                                TMR2ON = 0;
                                break;
     
                    case LOGIN_SUCESS:
                                control_flag = MAIN_MENU_FLAG;
                                reset_flag = RESET_LOGIN_MENU;
                                clear_screen();
                                clcd_write(DISP_ON_AND_CURSOR_OFF,INST_MODE);
                                TMR2ON = 0;
                                break;
                        // go to menu screen
                        break;
                }
                break;
            case MAIN_MENU_FLAG :
//                 implement menu screen
                menu_pos = menu_screen(key,reset_flag);
                break;
            case VEIW_LOG_FLAG :
                view_log(key,reset_flag);
                if(key == L_SW5)
                {
                    clear_screen();
                    control_flag = MAIN_MENU_FLAG;
                }
                break;
            case CLEAR_LOG_FLAG :
                clear_log();
                strcpy(event, "CL");
                log_car_event(event,speed);
                clear_screen();
                control_flag = MAIN_MENU_FLAG;
                break;
            case DOWNLOAD_LOG_FLAG :
                strcpy(event, "DL");
                log_car_event(event,speed);
                download_log();
                clear_screen();
                control_flag = MAIN_MENU_FLAG;
                break;
            case SET_TIME_FLAG:
                stat = set_time();
                if(stat == TASK_SUCESS)
                {
                strcpy(event, "ST");
                log_car_event(event,speed);
                }
                clear_screen();
                control_flag = MAIN_MENU_FLAG;
                break;
            case CHANGE_PASSWORD_FLAG:
                stat = change_password();
                if(stat == TASK_SUCESS)
                {
                    strcpy(event, "CP");
                    log_car_event(event,speed); 
                }
                clear_screen();
                control_flag = MAIN_MENU_FLAG;
                break;
        }
        /*if control flag is DASHBOARD read the speed and time and display play on screen */
        
        reset_flag = RESET_NOTHING;
}

}
