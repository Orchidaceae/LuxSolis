/* mipslab.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

/* Declare display-related functions from mipslabfunc.c */
void display_init(void);
void display_update(void);
uint8_t spi_send_recv(uint8_t data);

int light_up_pxset(int y, int x);
void updateDisplay(void);
void seven_seg(int segm, int digitNumb);
void time2digit(int time);
int number2digit_segs(int number, int digit);
void clear_display(void);
void clear_digits(void);
void update_time(int time);
void light_colon(void);
void clear_colon(void);
void display_error_msg(void);

/* Declares clock mode specific functions*/
void set_time_mode(int * time_source);
void set_time(int * time, int digit);
void set_wakeup_mode(void);
void set_color_mode(void);
void clock_work(void);
void change_mode(int * mode);
void display_wakeup_min(void);
void display_wakeup_brighness(void);
void change_wakeup_min(char * min);
void colorAlarm(int mode_set);
void change_wakeup_brightness(char * min);

/* Declares I/O functions */
int getbtns(void);
int getsw(void);

/* Time related functions */
void quicksleep(int cyc);
void tick( unsigned int * timep );

/* Declare buffers */
extern char display_bits[4][128]; //devides the display into 4x4 pixel-blocks

/* important variables */
extern int disable_colon;
extern int mytime;
extern int alarm;
extern int color_mode;
extern char wakeup_min;
extern int brightness;
char enable_alarm;

/* neopixel specific declarations */
extern uint8_t colorBuffer[12];        // 4 rows led1, led2, led3, led4; 3 columns GRB

int GRBsetLEDColor(uint8_t green, uint8_t red, uint8_t blue);
void GRBclearLEDColor(void);

void GRB_delay_one(void);
void GRB_delay_two(void);
void GRB_delay_three(void);

void neopxl_portSet(void);
void neopxl_portClr(void);
void strobe(void);
