#include "NU32.h"    

#include "currentcont.h"                   
#include <xc.h>

static volatile float Kp = 0, Ki = 0, eint = 0, Iref = 0;

void currentcont_init(void) {

  //__builtin_disable_interrupts(); // step 2: disable interrupts
                                  // step 3:
  T2CONbits.TCKPS = 2;            // Timer2 prescaler N=4 (1:4)
  PR2 = 3999;                     // period = (PR2+1) * N * 12.5 ns = 200 us, 5 kHz
  TMR2 = 0;                       // initial TMR2 count is 0             
  T2CONbits.ON = 1;               // turn on Timer2

  IPC2bits.T2IP = 5;              // step 4: interrupt priority 5
  IFS0bits.T2IF = 0;              // step 5: clear the int flag
  IEC0bits.T2IE = 1;              // step 6: enable 
  //__builtin_enable_interrupts();  // step 7: enable interrupts

  T3CONbits.TCKPS = 0;            // Timer3 prescaler N=1 (1:1)
  PR3 = 3999;                     // period = (PR3+1) * N * 12.5 ns = 500 us, 20 kHz
  TMR3 = 0;                       // initial TMR3 count is 0

  OC1CONbits.OC32 = 0;
  OC1CONbits.OCTSEL = 1;

  OC1CONbits.OCM = 0b110;         // PWM mode without fault pin; other OC1CON bits are defaults
  OC1RS = 1000;                   // duty cycle = OC1RS/(PR3+1) = 25%
  OC1R = 1000;                    // initialize before turning OC1 on; afterward it is read-only

  T3CONbits.ON = 1;               // turn on Timer3
  OC1CONbits.ON = 1;              // turn on OC1

  TRISDbits.TRISD8 = 0;           // Set pin D8 to digital output
  LATDbits.LATD8 = 0;             // Set D8 low

}

void set_Igains(float p, float i) {
  __builtin_disable_interrupts();
  Kp = p;
  Ki = i;
  eint = 0;
  __builtin_enable_interrupts();
}

float get_Ikp(void) {
  return Kp;
}

float get_Iki(void) {
  return Ki;
}

float get_Ieint(void) {
  return eint;
}

void set_Ieint(float e) {
  __builtin_disable_interrupts();
  eint = e;
  __builtin_enable_interrupts();
}

void set_Iref(float I) {
  __builtin_disable_interrupts();
  Iref = I;
  __builtin_enable_interrupts();
}

float get_Iref(void) {
  return Iref;
}
