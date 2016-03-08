/* mipslabfunc.c
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

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

void time2string (char *str, int ti) {
	str[5] = '\0'; // adds ASCII NULL

	char M1 = (char) ((ti & 0xf00) >> 8);
	str[4] = hexasc(M1);
	char M2 = (char) ((ti & 0xf000) >> 12);
	str[3] = hexasc(M2);

	str[2] = ':';

  char H1 = (char) ((ti & 0xf0000) >> 16);
	str[1] = hexasc(H1);
	char H2 = (char) ((ti & 0xf00000) >> 20);
	str[0] = hexasc(H2);
}

char hexasc(char hex)
{
	if(hex < 10)
		hex += 0x30;
	else
		hex += 0x37;
	return hex;
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

/* display_debug
   A function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr )
{
  display_string( 1, "Addr" );
  display_string( 2, "Data" );
  num32asc( &textbuffer[1][6], (int) addr );
  num32asc( &textbuffer[2][6], *addr );
  display_update();
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

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;

	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

void display_image(int x, const uint8_t *data) {
	int i, j;

	for(i = 0; i < 4; i++) {					//i < 4
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22); //0x22
		spi_send_recv(i);

		spi_send_recv(x & 0xF);			//x & 0xF
		spi_send_recv(0x10 | ((x >> 4) & 0xF));  // 0x10 | ((x >> 4) & 0xF)

		DISPLAY_CHANGE_TO_DATA_MODE;

		for(j = 0; j < 32; j++)						//j < 32
			spi_send_recv(data[i*32 + j]);	//i*32 + j
	}
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


void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(0x0);
		spi_send_recv(0x10);

		DISPLAY_CHANGE_TO_DATA_MODE;

		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;

			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n )
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

/*
 * itoa
 *
 * Simple conversion routine
 * Converts binary to decimal numbers
 * Returns pointer to (static) char array
 *
 * The integer argument is converted to a string
 * of digits representing the integer in decimal format.
 * The integer is considered signed, and a minus-sign
 * precedes the string of digits if the number is
 * negative.
 *
 * This routine will return a varying number of digits, from
 * one digit (for integers in the range 0 through 9) and up to
 * 10 digits and a leading minus-sign (for the largest negative
 * 32-bit integers).
 *
 * If the integer has the special value
 * 100000...0 (that's 31 zeros), the number cannot be
 * negated. We check for this, and treat this as a special case.
 * If the integer has any other value, the sign is saved separately.
 *
 * If the integer is negative, it is then converted to
 * its positive counterpart. We then use the positive
 * absolute value for conversion.
 *
 * Conversion produces the least-significant digits first,
 * which is the reverse of the order in which we wish to
 * print the digits. We therefore store all digits in a buffer,
 * in ASCII form.
 *
 * To avoid a separate step for reversing the contents of the buffer,
 * the buffer is initialized with an end-of-string marker at the
 * very end of the buffer. The digits produced by conversion are then
 * stored right-to-left in the buffer: starting with the position
 * immediately before the end-of-string marker and proceeding towards
 * the beginning of the buffer.
 *
 * For this to work, the buffer size must of course be big enough
 * to hold the decimal representation of the largest possible integer,
 * and the minus sign, and the trailing end-of-string marker.
 * The value 24 for ITOA_BUFSIZ was selected to allow conversion of
 * 64-bit quantities; however, the size of an int on your current compiler
 * may not allow this straight away.
 */
#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";

  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}
