#include "NU32.h"    

#include "positioncont.h"                   
#include <xc.h>

static volatile float PKp = 0, PKi = 0, PKd = 0, Peint = 0;
static volatile int angle = 0;

void positioncont_init(void) {

  //__builtin_disable_interrupts(); // step 2: disable interrupts
                                  // step 3:
  T4CONbits.TCKPS = 0b100;            // Timer4 prescaler N=1 (1:1)
  PR4 = 12000;                     // period = (PR2+1) * N * 12.5 ns = 5000 us, 200 Hz
  TMR4 = 0;                       // initial TMR4 count is 0
  T4CONbits.ON = 1;               // turn on Timer 4

  IPC4bits.T4IP = 6;              // step 4: interrupt priority 5
  IFS0bits.T4IF = 0;              // step 5: clear the int flag
  IEC0bits.T4IE = 1;              // step 6: enable 
  //__builtin_enable_interrupts();  // step 7: enable interrupts

  //TRISDbits.TRISD9 = 0;           // Set pin D9 to digital output
  //LATDbits.LATD9 = 0;             // Set D9 low

}

void set_Pgains(float p, float i, float d) {
  __builtin_disable_interrupts();
  PKp = p;
  PKi = i;
  PKd = d;
  Peint = 0;
  __builtin_enable_interrupts();
}

float get_Pkp(void) {
  return PKp;
}

float get_Pki(void) {
  return PKi;
}

float get_Pkd(void) {
  return PKd;
}

float get_Peint(void) {
  return Peint;
}

void set_Peint(float e) {
  __builtin_disable_interrupts();
  Peint = e;
  __builtin_enable_interrupts();
}

void set_angle(int a) {
  __builtin_disable_interrupts();
  angle = a;
  __builtin_enable_interrupts();
}

int get_angle(void) {
  return angle;
}
