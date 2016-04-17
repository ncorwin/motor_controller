#include "utilities.h"                   
#include <xc.h>

static volatile int mode = 0;
//0 = IDLE
//1 = PWM
//2 = ITEST
//3 = HOLD
//4 = TRACK

int get_mode(void) {

  return mode;
}

void set_mode(int mnew) {

  mode = mnew;
  //if(mode == 0) {
  //      __builtin_disable_interrupts();
  //		dir = 0;
  //		pwm = 0;
  //		__builtin_enable_interrupts();
  //}
}
