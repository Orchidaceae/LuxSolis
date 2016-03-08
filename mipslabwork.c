/* mipslabwork.c

   This file written 2015 by F Lundevall

   This file should be changed by YOU! So add something here:

   This file modified 2015-12-24 by Ture Teknolog

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

int mytime = 0x000000;
int alarm = 0x000100;
int sec_count = 0;
int min_count = 0;
int disable_colon = 0; //hides and shows the toggling mid-sceen colon

char textstring[] = "text, more text, and even more text!";

/* Interrupt Service Routine */
void user_isr( void )
{
  IFSCLR(0) = 0xfffffeff;
  if((IFS(0) & 0x100) == 0x100)
  {
    IFSCLR(0) = 0x00000100;   //clears the timer 2 interrupt flag (bit 8)

    ++sec_count;
    if(sec_count == 10)
    {
      sec_count = 0;
      tick( &mytime );
      ++min_count;
      if(!disable_colon)
        light_colon();
    }
  }
}

/* Lab-specific initialization goes here */
void labinit( void )
{
  TRISE =0xFFFFFF00;                   //initializes PORTEpoint (LEDs) to output on bits 0-7

  TRISDSET = 0x0fe0;          //0x0fe0         //initzializes PORTD to input on bits 5-11
  TRISFSET = 0x2;                      //initializes PORTF bit 1 (btn1)

  T2CON = 0x0;                         //stops and clears timer
  T2CON = 0x70;                        //sets prescalar to 256:1 (111 on bits 6:4) (0x10-0x70)
  TMR2 = 0x0;                          //clears the time value register
  PR2 = 0x7A12;                         //sets period to 31250(dec) (less than the maximum value of 16 bits!)
  IFSCLR(0) = 0x00000100;              //clears the timer interrupt flag (bit 8)

  IECCLR(0) = 0xffffffff;             //clears all interrupts
  IECSET(0) = 0x00000100;             //enables timer 2 interrupt (bit 8)
  IPCSET(2) = 31;                     //Sets both the priority( to 7 (111) ) and the subprio( to 3 (11)  ) to highest (11111 = 31)

  enable_interrupts();                //calls the assembly function with the same name in the file labworks.S

  T2CONSET = 0X8000;                  //starts timer

  return;
}

/* This function is called repetitively from the main program */
void clock_work( void )
{
  update_time(mytime);
  if(sec_count == 5)
    clear_colon();
  updateDisplay();

  if(alarm == mytime)
    PORTE = 0x1;

  if(getbtns() != 0)          //turn off the alarm?
    PORTE = 0x0;

}

/*
* The alarm and current time setting mode
* the user pushes the buttons on the I/O shield to change
* the variables alarm and mytime
*/
void set_time_mode(int * time_source)
{
  int t = *time_source;

  disable_colon = 1;           //hides the toggling colon
  light_colon();
  time2digit(t);
  updateDisplay();
  while(getsw() != 0)
  {
    switch (getbtns())
    {
      case 1:
      set_time(&t, 1);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;

      case 2:
      set_time(&t, 2);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;

      case 4:
      set_time(&t, 3);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;

      case 8:
      set_time(&t, 4);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;
    }
    *time_source = t;
  }
}

void set_wakeup_mode(void)
{}

void set_color_mode(void)
{}
