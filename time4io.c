#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw( void )
{
  int sw_status = (PORTD & 0xf00) >> 8; //extracts bits 8-11 and shifts them to the 4 lsb
  return sw_status;
}

int getbtns(void)
{
  int btns_status = ((PORTD & 0xe0) >> 4) | ((PORTF & 0x2) >> 1);     // 0xE0) >> 5; //extracts bits 5-7 and shifts them to the 3 lsb
  return btns_status;
}
