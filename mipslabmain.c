/* mipsclock_work.c

   This file written 2015 by Axel Isaksson,
   modified 2015 by F Lundevall

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

int main(void) {
	/* Set up peripheral bus clock */
        /* OSCCONbits.PBDIV = 1; */
        OSCCONCLR = 0x100000; /* clear PBDIV bit 1 */
	OSCCONSET = 0x080000; /* set PBDIV bit 0 */

	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;

	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;

	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);

	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
        SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;

	display_init();
//	display_string(3, "Welcome!");
//	display_update();

  display_update();

	labinit(); /* Do any lab-specific initialization */

  // initializes the seven segment display
  time2digit(0);


	while( 1 )
	{
    disable_colon = 0;   //shows the toggling colon

    if(getsw() == 0)
      clock_work(); /* Do lab-specific things again and again */

    switch (getsw())
    {
      case 1:                 //Puts the clock into the alarm setting mode
      clear_digits();
      set_time_mode(&alarm);
      clear_digits();
      update_time(mytime);
      break;

      case 2:                 //Puts the clock into the current time setting mode
      clear_digits();
      set_time_mode(&mytime);
      clear_digits();
      update_time(mytime);
      break;

      case 4:
      clear_digits();
      while(getsw() == 4)
      {
        display_string(1, "abc");
      }
      clear_digits();
      update_time(mytime);
      break;

      case 8:
      break;

      default:
      clear_digits();
      clear_colon();
      display_error_msg();
      updateDisplay();
      while(getsw() != 0) {}
      clear_digits();
      update_time(mytime);
      break;
    }
	}
	return 0;
}
