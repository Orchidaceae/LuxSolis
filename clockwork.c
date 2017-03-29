/* mipslabwork.c

   This file written 2015 by F Lundevall

   This file should be changed by YOU! So add something here:

   This file modified 2015-12-24 by Ture Teknolog

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "clock.h"  /* Declatations for these labs */

int mytime = 0x000000;
int alarm = 0x000000;
int sec_count = 0;
char min_count = 0;
int disable_colon = 0; //hides and shows the toggling mid-sceen colon
int color_mode = 1;
char wakeup_min = 1;
int brightness = 1;
char enable_alarm = 0;
char enable_strobe = 0;
char enable_sunrise = 0;
char enable_magentaM = 0;
char enable_ocean = 0;
char dimmer = 0;


/* Interrupt Service Routine */
void user_isr( void )
{
  IFSCLR(0) = 0xfffffeff;
  if((IFS(0) & 0x100) == 0x100)
  {
    IFSCLR(0) = 0x00000100;   //clears the timer 2 interrupt flag (bit 8)

    ++sec_count;
    if(sec_count == 10)       //scales the timeing to seconds
    {
      sec_count = 0;
      tick( &mytime );        //increase time
      if(!disable_colon)      //toggling colon
        light_colon();
      ++min_count;

      // sunrise alarm
      if(enable_sunrise == 1)                    //increase brighness color alarm 1
      {
        if((min_count % (4 * wakeup_min) == 0) && (dimmer != 15)) //wakeup_min increases the brightness riseing time
        {
          colorBuffer[2] += 10;
          colorBuffer[5] += 10;
          colorBuffer[8] += 10;
          colorBuffer[11] += 10;
          ++dimmer;
          GRBrefreshLEDs();
        }
      }

      if(enable_magentaM == 1)                    //increase brighness color alarm 2
      {
        if((min_count % (4 * wakeup_min) == 0) && (dimmer != 15))
        {
          colorBuffer[1] += 10;
          colorBuffer[2] += 10;
          colorBuffer[4] += 2;
          colorBuffer[7] += 2;
          colorBuffer[10] += 10;
          colorBuffer[11] += 10;
          ++dimmer;
          GRBrefreshLEDs();
        }
      }

      if(enable_ocean == 1)                    //increase brighness color alarm 3
      {
        if((min_count % (4 * wakeup_min) == 0) && (dimmer != 15))
        {
          colorBuffer[2] += 10;
          colorBuffer[3] += 10;
          colorBuffer[6] += 10;
          colorBuffer[11] += 10;
          ++dimmer;
          GRBrefreshLEDs();
        }
      }
    }
  }

  if(min_count == 60)
    min_count = 0;
}

/* initialization of ports and timer */
void clockinit( void )
{
  TRISECLR = 0x2;                      //initzializes pin 27 to data output for the neopixels

  TRISDSET = 0x0fe0;                   //initzializes PORTD to input on bits 5-11
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
}


/* This function is called repetitively from the main program */
void clock_work( void )
{
  //show time and toggling colon
  update_time(mytime);
  if(sec_count == 5)
    clear_colon();
  updateDisplay();

  //turn on the alarm
  if((alarm == mytime) && (enable_alarm == 1))
    {
      switch (color_mode)
      {
        case 1:
        enable_sunrise = 1;
        colorAlarm(1);
        GRBrefreshLEDs();
        break;
        case 2:
        enable_magentaM = 1;
        colorAlarm(2);
        GRBrefreshLEDs();
        break;
        case 3:
        enable_ocean = 1;
        colorAlarm(3);
        GRBrefreshLEDs();
        break;
        case 4:
        enable_strobe = 1;
        break;
      }
    }

  //strobe alarm
  if(enable_strobe == 1)
  {
    if(!(sec_count % 3))
      strobe();
    else
    {
      GRBclearLEDColor();
      GRBrefreshLEDs();
    }
  }

  //turn off the alarm
  if((getbtns() != 0) || (getsw() != 0))
    {
      GRBclearLEDColor();
      GRBrefreshLEDs();
      dimmer = 0;
      switch (color_mode)
      {
        case 1:
        enable_sunrise = 0;
        break;
        case 2:
        enable_magentaM = 0;
        break;
        case 3:
        enable_ocean = 0;
        break;
        case 4:
        enable_strobe = 0;
        break;
      }
    }
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
      case 1:                 //btn 1 changes M1
      set_time(&t, 1);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;

      case 2:                 //btn 2 changes M2
      set_time(&t, 2);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;

      case 4:                 //btn 3 changes H1
      set_time(&t, 3);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;

      case 8:                 //btn 4 changes H2
      set_time(&t, 4);
      update_time(t);
      updateDisplay();
      quicksleep(0x003fffff);
      break;
    }
    *time_source = t;
  }
}

/*
* Lets the user decide the wake up time in mode 1-3
* or the brightness in the strobe mode (#4)
*/
void set_wakeup_mode(void)
{

  if(color_mode != 4)
  {
    display_wakeup_min();
    while (getsw() != 0)
    {
      if(getbtns() == 1)
      {
        change_wakeup_min(&wakeup_min);
        number2digit_segs(wakeup_min, 1);
        updateDisplay();
        quicksleep(0x003fffff);
      }
    }
  }
  else
  {
    while (getsw() != 0)
    {
      display_wakeup_brighness();
      if(getbtns() == 1)
      {

      }
    }
  }
}

/*
* Lets the user set a preprogrammed color wakeup mode:
* 1: Sunrise
* 2: Magenta morning
* 3: Ocean
* 4: strobe
*/
void set_color_mode(void)
{
  while (getsw() != 0)
  {
    display_mode();
    if(getbtns() == 1)
    {
      change_mode(&color_mode);
      quicksleep(0x003fffff);
    }
  }
}
