/* 
 * File:   cbb.h
 * Author: hp
 *
 * Created on 7 January, 2025, 7:53 PM
 */

#ifndef CBB_H
#define CBB_H

#define RETURN_BACK 0x02
#define LOGIN_SUCESS 0x01
#define TASK_SUCESS 0x11
#define TASK_FAIL 0x22

#define DASH_BOARD_FLAG 0x01
#define LOGIN_FLAG 0x02
#define MAIN_MENU_FLAG 0x04
#define VEIW_LOG_FLAG 0x08
#define CLEAR_LOG_FLAG 0x10
#define DOWNLOAD_LOG_FLAG 0x20
#define SET_TIME_FLAG 0x40
#define CHANGE_PASSWORD_FLAG 0x80
#define RESET_NOTHING 0x00
#define RESET_PASSWORD 0x01
#define RESET_LOGIN_MENU 0x02
#define RESET_MEMORY 0x04
#define RESET_VIEW_LOG_POS 0x08
#define REST_TIME 0x10


void display_dash_board(char event[], unsigned short speed);
void log_car_event (char *event ,unsigned short speed);
void clear_screen(void);
unsigned char login_screen(unsigned char key,unsigned char reset_flag); 
unsigned char menu_screen(unsigned char key,unsigned char reset_flag);
void view_log(unsigned char key,unsigned char reset_flag);
void clear_log(void);
void log_doc_event (char *event);
void download_log(void);
unsigned char set_time(void);
unsigned char change_password(void);
unsigned char compare_pwd(unsigned char reset_flag);
#endif /* CBB_H */

