/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog 

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

int prime = 1234678;
int mytime = 0x5957;
int timeoutcount = 0;
char textstring[] = "text, more text, and even more text!";

/* Interrupt Service Routine */
// call when interrupt flag is set. 
void user_isr( void )
{
  IFS(0) = 0x000;
  timeoutcount++;
  if(timeoutcount == 10){
    time2string( textstring, mytime );
    display_string( 3, textstring );
    display_update();
    tick( &mytime );
    timeoutcount = 0;
  }
  // add code to acknowledge interrupts from timer2. 
  
}

/* Lab-specific initialization goes here */
void labinit( void )
{ 
  //create a pointer to the TRISE address. make sure to cast use volatile a casting so that 
  //the compiler does not optimize.
  volatile int* lamp = (volatile int*) 0xbf886100;
 // deference the pointer so that the value on the memory address changes. 
 // zeros indicates outputs and ones indicates input. 
  *lamp = *lamp & 0xFF00;
  //We need to initilize the inputs for port D. 
  // Bits from 11 to 5 have to be ones. 0000 1111 1110 0000. 
  // Index starts at 0. 
  TRISD = TRISD & 0x0FE0;
  // Timerstuff. 
  T2CONSET = 0x70; // prescale the timer
  TMR2 = 0; // reset the timer
  T2CONSET = 0x8000;
  PR2 = ((80000000/256)/10); //period time. 

  //Interrupt priority Control Register 
  //This sets the priority of different interrupts
  IPC(2) = 0x1F;
  IEC(0) = 0x100;//some initialization to the Interrupt Enable Center
  // add .global enable_interrupt to be able to call the function here in the C-file.  
  enable_interrupt();
  return;
}

/* This function is called repetitively from the main program */
void labwork( void )
{
    int sw;
    int button;
       
    //delay( 1000 );
    button = getbtns();
    sw = getsw();
    // If button is pressed 
    switch(button) { 
    case 1: // 3  
      mytime = mytime & 0xFF0F;
      mytime = (sw << 4 ) | mytime;
      //*pointerE = *pointerE + 1;
      break;
    case 2: // button nr 2 on the chipkit. 
      mytime = mytime & 0xF0FF;
      mytime = (sw << 8) | mytime;
      //*pointerE = *pointerE + 2;
      break;
    case 4: // button nr 1 on the chipkit
      mytime = mytime & 0x0FFF;
      mytime = (sw << 12) | mytime;
      //*pointerE = *pointerE + 3;
      break;
    }

    prime = nextprime(prime);
    display_string(0, itoaconv(prime));
    display_update();
}
