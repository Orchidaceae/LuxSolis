/* mipslabdata.c
   This file compiled 2015 by F Lundevall
   from original code written by Axel Isaksson

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "clock.h"  /* Declatations for these labs */

char textbuffer[4][16];
char display_bits[4][128];
uint8_t colorBuffer[12];
