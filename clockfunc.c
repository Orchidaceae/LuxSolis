/* mipslabfunc.c
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "clock.h"  /* Declatations for these labs */

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int );
char hexasc(char hex);

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

/* tick:
   Add 1 to time in memory, at location pointed to by parameter.
   Time is stored as 4 pairs of 2 NBCD-digits.
   1st pair (most significant byte) counts days.
   2nd pair counts hours.
   3rd pair counts minutes.
   4th pair (least significant byte) counts seconds.
   In most labs, only the 3rd and 4th pairs are used. */
void tick( unsigned int * timep )
{
  /* Get current value, store locally */
  register unsigned int t = * timep;
  t += 1; /* Increment local copy */

  /* If result was not a valid BCD-coded time, adjust now */

  if( (t & 0x0000000f) >= 0x0000000a ) t += 0x00000006;
  if( (t & 0x000000f0) >= 0x00000060 ) t += 0x000000a0;
  /* Seconds are now OK */

  if( (t & 0x00000f00) >= 0x00000a00 ) t += 0x00000600;
  if( (t & 0x0000f000) >= 0x00006000 ) t += 0x0000a000;
  /* Minutes are now OK */

  if( (t & 0x000f0000) >= 0x000a0000 ) t += 0x00060000;
  if( (t & 0x00ff0000) >= 0x00240000 ) t += 0x00dc0000;
  /* Hours are now OK */

  if( (t & 0x0f000000) >= 0x0a000000 ) t += 0x06000000;
  if( (t & 0xf0000000) >= 0xa0000000 ) t = 0;
  /* Days are now OK */

  * timep = t; /* Store new value */
}

/*
* This function lets a user set the time or the alarm digit by digit
*/
void set_time(int * time, int digit)
{
	int t = *time;

	switch (digit)
	{
		case 1:					//M1
		if( ((t & 0xf00) >> 8) < 9)
			t += 0x100;
		else
			t &= 0xfffff0ff;
		break;

		case 2:					//M2
		if( ((t & 0xf000) >> 12) < 5)
			t += 0x1000;
		else
			t &= 0xffff0fff;
		break;

		case 3:					//H1
		if( ((t & 0xf00000) >> 20) != 2 )
		{
			if( ((t & 0xf0000) >> 16) < 9)
				t += 0x10000;
			else
				t &= 0xfff0ffff;
		}
		else
		{
			if( ((t & 0xf0000) >> 16) < 3)
				t += 0x10000;
			else
				t &= 0xfff0ffff;
		}
		break;

		case 4:					//H2
		if( ((t & 0xf00000) >> 20) < 2)
			t += 0x100000;
		else
			t &= 0xff0fffff;
		break;
	}

	//Sets time to 00:00 if an illegal combination is set   EVIL!!!
	if( (((t & 0xff0000) >> 16) > 0x23) || (((t & 0xff00) >> 8) > 0x59) )
		t = 0;

	*time = t;
}

/*
* updates the displayed time
*/
void update_time(int time)
{
	clear_digits();
	time2digit(time);
}

/*
* displays the mid-screen colon
*/
void light_colon(void)
{
	light_up_pxset(2, 15);
	light_up_pxset(2, 16);
	light_up_pxset(6, 15);
	light_up_pxset(6, 16);
}

/*
* Hides the mid-sceen colon
*/
void clear_colon(void)
{
	int y, x;
	for(y = 1; y < 4; y++)
	{
		x = 0;
		for(x = 58; x < 74; x++)
			display_bits[y][x] = 0;
	}
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));	//while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
  DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);

	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);

	spi_send_recv(0x8D);
	spi_send_recv(0x14);

	spi_send_recv(0xD9);
	spi_send_recv(0xF1);

	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);

	spi_send_recv(0xA1);
	spi_send_recv(0xC8);

	spi_send_recv(0xDA);
	spi_send_recv(0x20);

	spi_send_recv(0xAF);
}

/*
* Takes a 2 byte NBCD-code and displays as a 4-digit 7-seg clock
*/
void time2digit(int time)
{
	int M1 = (time & 0xf00) >> 8;
	number2digit_segs(M1, 5);

	int M2 = (time & 0xf000) >> 12;
	number2digit_segs(M2, 4);

  int H1 = (time & 0xf0000) >> 16;
	number2digit_segs(H1, 2);

	int H2 = (time & 0xf00000) >> 20;
	number2digit_segs(H2, 1);
}


/*
* Clears the display buffer display_bits
*/
void clear_digits(void)
{
	int y, x;
	for(y = 0; y < 4; y++)
	{
		for(x = 0; x < 58; x++)
			display_bits[y][x] = 0;
		for(x = 74; x < 128; x++)
			display_bits[y][x] = 0;
	}
}

/*
* Takes a number between 0 and 9 and displays it as a 7-seg digit
* digit - decides whitch of the five 7-seg digits
*/
int number2digit_segs(int number, int digit)
{
	int i;
	if(number < 0 || number > 9 || digit < 1  || digit > 5)
		return 1; 								//invalid values

	if(number == 0)
		for(i = 1; i < 7; i++)
		seven_seg(i, digit);

	switch (number)
	{
		case 1:
		seven_seg(2, digit);
		seven_seg(3, digit);
		break;
		case 2:
		seven_seg(1, digit);
		seven_seg(2, digit);
		seven_seg(4, digit);
		seven_seg(5, digit);
		seven_seg(7, digit);
		break;
		case 3:
		seven_seg(1, digit);
		seven_seg(2, digit);
		seven_seg(3, digit);
		seven_seg(4, digit);
		seven_seg(7, digit);
		break;
		case 4:
		seven_seg(2, digit);
		seven_seg(3, digit);
		seven_seg(6, digit);
		seven_seg(7, digit);
		break;
		case 5:
		seven_seg(1, digit);
		seven_seg(3, digit);
		seven_seg(4, digit);
		seven_seg(6, digit);
		seven_seg(7, digit);
		break;
		case 6:
		seven_seg(1, digit);
		seven_seg(3, digit);
		seven_seg(4, digit);
		seven_seg(5, digit);
		seven_seg(6, digit);
		seven_seg(7, digit);
		break;
		case 7:
		seven_seg(1, digit);
		seven_seg(2, digit);
		seven_seg(3, digit);
		break;
		case 8:
		for(i = 1; i < 8; i++)
		seven_seg(i, digit);
		break;
		case 9:
		seven_seg(1, digit);
		seven_seg(2, digit);
		seven_seg(3, digit);
		seven_seg(4, digit);
		seven_seg(6, digit);
		seven_seg(7, digit);
		break;
	}
}

/*
* Displays the word error on the in the 7-seg display mode
*/
void display_error_msg(void)
{
	seven_seg(1, 1); // E in 7-seg
	seven_seg(4, 1);
	seven_seg(5, 1);
	seven_seg(6, 1);
	seven_seg(7, 1);

	seven_seg(5, 2); // r in 7-seg
	seven_seg(6, 2);
	seven_seg(7, 2);

	seven_seg(5, 3);
	seven_seg(6, 3);
	seven_seg(7, 3);

	seven_seg(3, 4); // o in 7-seg
	seven_seg(4, 4);
	seven_seg(5, 4);
	seven_seg(7, 4);

	seven_seg(5, 5);
	seven_seg(6, 5);
	seven_seg(7, 5);
}

/*
* Displays the word mode and the mode digit in the color setting mode
*/
void display_mode(void)
{
	number2digit_segs(color_mode, 1);

	light_up_pxset(2, 11);						//M
	light_up_pxset(2, 15);
	light_up_pxset(3, 11);
	light_up_pxset(3, 12);
	light_up_pxset(3, 14);
	light_up_pxset(3, 15);
	light_up_pxset(4, 11);
	light_up_pxset(4, 13);
	light_up_pxset(4, 15);
	light_up_pxset(5, 11);
	light_up_pxset(5, 15);
	light_up_pxset(6, 11);
	light_up_pxset(6, 15);

	light_up_pxset(3, 18);						//o
	light_up_pxset(4, 17);
	light_up_pxset(4, 19);
	light_up_pxset(5, 17);
	light_up_pxset(5, 19);
	light_up_pxset(6, 18);

	light_up_pxset(1, 23);						//d
	light_up_pxset(2, 23);
	light_up_pxset(3, 22);
	light_up_pxset(3, 23);
	light_up_pxset(4, 21);
	light_up_pxset(4, 23);
	light_up_pxset(5, 21);
	light_up_pxset(5, 23);
	light_up_pxset(6, 22);
	light_up_pxset(6, 23);

	light_up_pxset(2, 26);						//e
	light_up_pxset(2, 27);
	light_up_pxset(3, 25);
	light_up_pxset(3, 28);
	light_up_pxset(4, 25);
	light_up_pxset(4, 26);
	light_up_pxset(4, 27);
	light_up_pxset(5, 25);
	light_up_pxset(6, 26);
	light_up_pxset(6, 27);

	updateDisplay();
}

/*
* changes the color mode and shows it on the display
*/
void change_mode(int * mode)
{
	int modeNumb = *mode;
	if(modeNumb >= 4)
		modeNumb = 1;
	else
		modeNumb += 1;

	int y, x;
	for(y = 0; y < 4; y++)
	{
		for(x = 0; x < 40; x++)
			display_bits[y][x] = 0;
	}

	number2digit_segs(modeNumb, 1);
	updateDisplay();

	*mode = modeNumb;
}

/*
* displays the set wakeup minute settings
* is enabled in colormodes 1-3
*/
void display_wakeup_min(void)
{
	number2digit_segs(wakeup_min, 1);

	light_up_pxset(2, 11);						//M
	light_up_pxset(2, 15);
	light_up_pxset(3, 11);
	light_up_pxset(3, 12);
	light_up_pxset(3, 14);
	light_up_pxset(3, 15);
	light_up_pxset(4, 11);
	light_up_pxset(4, 13);
	light_up_pxset(4, 15);
	light_up_pxset(5, 11);
	light_up_pxset(5, 15);
	light_up_pxset(6, 11);
	light_up_pxset(6, 15);

	light_up_pxset(3, 17);						//I
	light_up_pxset(4, 17);
	light_up_pxset(5, 17);
	light_up_pxset(6, 17);

	light_up_pxset(3, 19);
	light_up_pxset(4, 19);
	light_up_pxset(4, 20);
	light_up_pxset(4, 21);
	light_up_pxset(5, 19);
	light_up_pxset(5, 21);
	light_up_pxset(6, 19);
	light_up_pxset(6, 21);


	updateDisplay();
}

/*
* increases the wakeup minutes
*/
void change_wakeup_min(char * min)
{
	char m = *min;
	if(m < 5)
		m += 2;
	else
		m = 1;

	int y, x;
	for(y = 0; y < 4; y++)
	{
		for(x = 0; x < 40; x++)
			display_bits[y][x] = 0;
	}
	updateDisplay();
	*min = m;
}

/*
* Displays the brighness setting mode
* enabled in color mode 4 (strobe)
*/
void display_wakeup_brighness(void)
{
	number2digit_segs(color_mode, 1);

	light_up_pxset(1, 11);
	light_up_pxset(1, 12);
	light_up_pxset(1, 13);
	light_up_pxset(2, 11);
	light_up_pxset(2, 14);
	light_up_pxset(3, 11);
	light_up_pxset(3, 14);
	light_up_pxset(4, 11);
	light_up_pxset(4, 12);
	light_up_pxset(4, 13);
	light_up_pxset(5, 11);
	light_up_pxset(5, 14);
	light_up_pxset(6, 11);
	light_up_pxset(6, 14);
	light_up_pxset(7, 11);
	light_up_pxset(7, 12);
	light_up_pxset(7, 13);

	updateDisplay();
}

/*
*	changes the brightness of the strobe wakeup mode
*/
void change_wakeup_brightness(char * brightness)
{
	char b = * brightness;
	if(b < 2)
		++b;
	else
		b = 1;

	int y, x;
	for(y = 0; y < 4; y++)
	{
		for(x = 0; x < 40; x++)
			display_bits[y][x] = 0;
	}
	updateDisplay();
	*brightness = b;
}


/*
* Makes the display into a 5 digit 7-seg display
* seg - decides whitch segment to light up
* digitNumb - decides whitch of the 5 digits
*/
void seven_seg(int seg, int digitNumb)
{
	int a = 6;						//offset for the digit
	switch (digitNumb)
	{
		case 1:
		a *= 0;
		break;

		case 2:
		a *= 1;
		break;

		case 3:
		a *= 2;
		break;

		case 4:
		a *= 3;
		break;

		case 5:
		a *= 4;
		break;
	}
	switch (seg)
	{
		case 1:
		light_up_pxset(1, 3 + a);
		light_up_pxset(1, 4 + a);
		break;
		case 2:
		light_up_pxset(2, 5 + a);
		light_up_pxset(3, 5 + a);
		break;
		case 3:
		light_up_pxset(5, 5 + a);
		light_up_pxset(6, 5 + a);
		break;
		case 4:
		light_up_pxset(7, 3 + a);
		light_up_pxset(7, 4 + a);
		break;
		case 5:
		light_up_pxset(5, 2 + a);
		light_up_pxset(6, 2 + a);
		break;
		case 6:
		light_up_pxset(2, 2 + a);
		light_up_pxset(3, 2 + a);
		break;
		case 7:
		light_up_pxset(4, 3 + a);
		light_up_pxset(4, 4 + a);
		break;
	}
}

/*
* Devides the display-buffer display_bits into a 8x32 grid
* with coordinates 0 ≤ y ≤ 8 (row) and 0 ≤ x ≤ 31 (column)
* lights up a 4x4 pixel block with the corresponding coordinates
*/
int light_up_pxset(int y, int x)
{
	int i;
	if ((y >= 8) || (x >= 32 )||(y < 0)||(x < 0))
		return 1; //Incorrect coordinates

	for(i = 0; i < 4; i++)
	{
		if(y%2)
			display_bits[y/2][x * 4 + i] = display_bits[y/2][x * 4 + i] | 0xf0;
		else
			display_bits[y/2][x * 4 + i] = display_bits[y/2][x * 4 + i] | 0x0f;
	}
}

/*
* Updates the whole display
*/
void updateDisplay(void)
{
    int row, col;
  	for(row = 0; row < 4; row++)
		{
  		DISPLAY_CHANGE_TO_COMMAND_MODE;
  		spi_send_recv(0x22);  //Tell SPI that we want to set the row
  		spi_send_recv(row);

  		spi_send_recv(0x0);    //Set the 4 LSB in the offset to 0
  		spi_send_recv(0x10);   //Set the next 4 bits in the offset to 0

  		DISPLAY_CHANGE_TO_DATA_MODE;

  			for(col = 0; col < 128; col++)
					spi_send_recv(display_bits[row][col]); //pushes the bit-map buffer display_bits to the display
  	}

}

inline void neopxl_portSet()
{
	PORTESET = 0x2;
}

inline void neopxl_portClr()
{
	PORTECLR = 0x2;
}

//GRB delay one
#define GRB_delay_one(); {asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n  ");}

//GRB delay two
#define GRB_delay_two(); {asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n ");}

//GRB delay three
#define GRB_delay_three(); {asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n ");}

/*  Modifies the color matrix with the colors presented one 32-bit
*  value. color is a 32-bit unsigned int that is organized into four
*	bytes:
*	bits[31 - 24][23 - 16][15 - 8][7 - 0]
*      ( blank )(  red  )(green )(blue )
*
*	Each color is scaled by the brightness value and stored in the
* color array.
*/
int GRBsetLEDColor(uint8_t green, uint8_t red, uint8_t blue)
{

}

void GRBclearLEDColor(void)
{
	int i;
	for(i = 0; i < 12; i++)
		colorBuffer[i] = 0;
}

/*  Generate the datastream to refresh the LEDs using the RGB color 	*/
/*  mode.  This function utilizes MIPS assembly no-op commands to 		*/
/*  ensure that the specific timing is met.								*/
/* 																		*/
/*	In each frame of the bitstream there are three components that 		*/
/*	allow for easier manipulation:										*/
/*	- high sector - the first 350ns of the bitframe that is high 		*/
/*		regardless of the data bit being a 1 or 0						*/
/*	- variable sector - the second 350ns of the bitframe that is high 	*/
/*		if a 1 and low if a zero										*/
/*	- low sector - third and final 550ns of the bitframe that is low	*/
/*		regardless of the data bit being a 1 or 0						*/
/*																		*/
/*	There are preprocessor defined macros to control the variable 		*/
/*	number of no-ops required to meeting timing as easily as possible.	*/
/*	preprocessor defined macros will allow for easier porting to other 	*/
/*	ChipKIT boards.														*/
void GRBrefreshLEDs(void)
{
		//do not allow bitstream to be interrupted
	//noInterrupts();
	uint8_t* colorArrayPtr = colorBuffer;
	int j;
	for(j = 0; j < 12; j++){
////////////////////
//Bit 0
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b10000000)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	neopxl_portClr();

	//wait forever
	GRB_delay_three();


////////////////////
//Bit 1
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b01000000)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	neopxl_portClr();

	//wait forever
	GRB_delay_three();

////////////////////
//Bit 2
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b00100000)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	neopxl_portClr();

	//wait forever
	GRB_delay_three();

////////////////////
//Bit 3
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b00010000)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	//*port = lowBitMask;
	neopxl_portClr();

	//wait forever
	GRB_delay_three();

////////////////////
//Bit 4
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b00001000)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	neopxl_portClr();

	//wait forever
	GRB_delay_three();

////////////////////
//Bit 5
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b00000100)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	neopxl_portClr();

	//wait forever
	GRB_delay_three();

////////////////////
//Bit 6
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b00000010)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	neopxl_portClr();

	//wait forever
	GRB_delay_three();

////////////////////
//Bit 7
////////////////////
	//set port to high to start
	neopxl_portSet();

	//wait forever
	GRB_delay_one();

	//set variable bitmask
	if(*colorArrayPtr & 0b00000001)
		neopxl_portSet();
	else
		neopxl_portClr();

	//wait forever
	GRB_delay_two();

	//set port to low
	neopxl_portClr();

	//wait forever
	GRB_delay_three();

	colorArrayPtr += 1;
	}

	neopxl_portClr();


	//bitstream done, enable interrupts
	//interrupts();
}

/*
* Initialzes the color on the alarm
*/
void colorAlarm(int mode_set)
{
	switch (mode_set)
	{
		case 1:												//sunrise
		GRBclearLEDColor();
		colorBuffer[0] = 250; //g
		colorBuffer[1] = 250; //r

		colorBuffer[4] = 250;

		colorBuffer[7] = 250;

		colorBuffer[9] = 250;
		colorBuffer[10] = 250;
		break;
		case 2:												//magenta morning
		GRBclearLEDColor();
		colorBuffer[1] = 100; //r
		colorBuffer[2] = 100; //b

		colorBuffer[4] = 200;

		colorBuffer[7] = 200;

		colorBuffer[10] = 100;
		colorBuffer[11] = 100;
		break;
		case 3:												//ocean
		GRBclearLEDColor();
		colorBuffer[0] = 100; //g
		colorBuffer[2] = 100; //b

		colorBuffer[3] = 100;

		colorBuffer[6] = 100;

		colorBuffer[9] = 100;
		colorBuffer[11] = 100;
		break;
		case 4:												//strobe
		GRBclearLEDColor();
		colorBuffer[0] = 100; //g
		colorBuffer[1] = 100; //r
		colorBuffer[2] = 100; //b

		colorBuffer[3] = 100;
		colorBuffer[4] = 100;
		colorBuffer[5] = 100;

		colorBuffer[6] = 100;
		colorBuffer[7] = 100;
		colorBuffer[8] = 100;

		colorBuffer[9] = 100;
		colorBuffer[10] = 100;
		colorBuffer[11] = 100;
		break;
	}
}

void strobe(void)
{
	colorBuffer[0] = 200; //g
	colorBuffer[1] = 200; //r
	colorBuffer[2] = 200; //b

	colorBuffer[3] = 200;
	colorBuffer[4] = 200;
	colorBuffer[5] = 200;

	colorBuffer[6] = 200;
	colorBuffer[7] = 200;
	colorBuffer[8] = 200;

	colorBuffer[9] = 200;
	colorBuffer[10] = 200;
	colorBuffer[11] = 200;
	GRBrefreshLEDs();
}
