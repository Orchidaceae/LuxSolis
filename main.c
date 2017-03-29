/* mipsclock_work.c

   This file written 2015 by Axel Isaksson,
   modified 2015 by F Lundevall

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "clock.h"  /* Declatations for these labs */

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

  updateDisplay();

	clockinit();  //calls all initializations

  // initializes the seven segment display
  time2digit(0);

	while( 1 )
	{
    disable_colon = 0;        //shows the toggling colon

    if(getsw() == 0)
      clock_work();           //Show the clock mode when no switch is pulled up

    switch (getsw())
    {
      case 1:                 //Puts the clock into the alarm setting mode; switch 1
      clear_digits();
      enable_alarm = 1;
      set_time_mode(&alarm);
      clear_digits();
      update_time(mytime);
      break;

      case 2:                 //Puts the clock into the current time setting mode; switch 2
      clear_digits();
      set_time_mode(&mytime);
      clear_digits();
      update_time(mytime);
      break;

      case 4:                 //Puts the clock into the wakeup setting mode; switch 3
      disable_colon = 1;
      clear_digits();
      clear_colon();
      updateDisplay();
      set_wakeup_mode();
      clear_digits();
      update_time(mytime);
      break;

      case 8:                 //Puts the clock into the wakeup color setting mode; switch 4
      disable_colon = 1;
      clear_digits();
      clear_colon();
      updateDisplay();
      set_color_mode();
      clear_digits();
      update_time(mytime);
      break;

      //case 4:
      case 3:
      case 5:
      case 6:
      case 7:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      clear_digits();           //More than one switch has been set by the user, display error message
      clear_colon();
      display_error_msg();
      updateDisplay();
      while(getsw() != 0)
      {}
      clear_digits();
      update_time(mytime);
      break;



    }

	}
	return 0;
}
