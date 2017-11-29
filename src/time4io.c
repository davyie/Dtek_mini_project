#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"
//function prototype -> int getsw(void).
//return the value of the pointer.
int getsw(void) {
    int sw = (PORTD >> 8) & 0x000F;
    return sw;
} 
//function prototype -> int getbtns(void)
//return the value of pointer
int getbtns(void) {
   int btn = (PORTD >> 5) & 0x0007;
   return btn;
}

