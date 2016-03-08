/* mipslab.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

/* Declare display-related functions from mipslabfunc.c */
void display_image(int x, const uint8_t *data);
void display_init(void);
void display_string(int line, char *s);
void display_update(void);
uint8_t spi_send_recv(uint8_t data);

int light_up_pxset(int y, int x);
void updateDisplay(void);
void seven_seg(int segm, int digitNumb);
void time2digit(int time);
int number2digit_segs(int number, int digit);
void clear_display(void);
void update_time(int time);
void light_colon(void);
void clear_colon(void);
void display_error_msg(void);

void set_time_mode(int * time_source);
void set_time(int * time, int digit);
void set_wakeup_mode(void);
void set_color_mode(void);



/* Declare lab-related functions from mipslabfunc.c */
char * itoaconv( int num );
void clock_work(void);

void quicksleep(int cyc);
void tick( unsigned int * timep );

/* Declare display_debug - a function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr );

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare bitmap array containing icon */
extern const uint8_t const icon[128];

extern const uint8_t const test[24*4];

extern const uint8_t const test5[16*4];

/* Declare text buffer for display output */
extern char textbuffer[4][16];

extern char display_bits[4][128]; //devides the display into 4x4 pixel-blocks

extern int disable_colon;
extern int mytime;
extern int alarm;

/* Declare functions written by students.
   Note: Since we declare these functions here,
   students must define their functions with the exact types
   specified in the laboratory instructions. */
/* Written as part of asm lab: delay, time2string */
void delay(int);
void time2string( char *, int );
/* Written as part of i/o lab: getbtns, getsw */
int getbtns(void);
int getsw(void);
