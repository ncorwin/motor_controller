#include "NU32.h"          // config bits, constants, funcs for startup and UART
#include "encoder.h"
#include "utilities.h"
#include "isense.h"
#include "currentcont.h"
#include "positioncont.h"
#include "LCD.h"

#define BUF_SIZE 200
#define M (1.988)
#define B (502.424)
#define CENTER 32768

#define NUMSAMPS 50
#define PLOTPTS 100
#define MSG_LEN 20

#define REFLEN 1000

static volatile int pwm = 0;
static volatile int dir = 0;

static volatile float Trajectory[REFLEN];
static volatile float Path[REFLEN];
static volatile int TrajLen = 0;

static volatile int Waveform[NUMSAMPS];
static volatile int ADCarray[PLOTPTS];
static volatile int REFarray[PLOTPTS];
static volatile int StoringData = 0;
static volatile int eint = 0;

void __ISR(_TIMER_4_VECTOR, IPL6SOFT) Control_200(void) {
  //if(LATDbits.LATD9 == 0) {
  //	LATDbits.LATD9 = 1;
  //}
  //else{
  //	LATDbits.LATD9 = 0;
  //}

  //HOLD
  if(get_mode()==3) {
	  static float P_ref = 0;
	  static float P_e = 0;
	  static float P_eprev = 0;
	  static float P_edot= 0;
	  static float P_eint = 0;
	  static float P_u = 0;
	  static float P_unew = 0;
	  static float P_Kp = 0;
	  static float P_Ki = 0;
	  static float P_Kd = 0;
	  static float angle = 0;
	  static int P_count = 0;

	  P_Kp = get_Pkp();
	  P_Ki = get_Pki();
	  P_Kd = get_Pkd();
	  P_eint = get_Peint();

	  P_count = encoder_counts();
	  P_count = encoder_counts();
	  angle = (float) 360*(P_count-CENTER)/(448*4);

	  P_ref = get_angle();

	  P_e = P_ref - angle;

	  P_eint = P_eint + P_e;

	  P_edot = (P_e - P_eprev)/0.005;

	  P_u = P_Kp*P_e + P_Ki*P_eint + P_Kd*P_edot;

	  if(P_u > 600) {
		P_u = 600;
	  }
	  else if(P_u < -600) {
		P_u = -600;
	  }

	  P_eprev = P_e;

	  set_Peint(P_eint);

	  if(P_e > 0) {
	  	dir = 0;
	  }
	  else {
	  	dir = 1;
	  }

	  set_Iref(P_u);

  }
	 
  //TRACK
  if(get_mode()==4) {
	  static float P_ref = 0;
	  static float P_e = 0;
	  static float P_eprev = 0;
	  static float P_edot= 0;
	  static float P_eint = 0;
	  static float P_u = 0;
	  static float P_unew = 0;
	  static float P_Kp = 0;
	  static float P_Ki = 0;
	  static float P_Kd = 0;
	  static float angle = 0;
	  static int P_count = 0;

	  static int index = 0;

	  
	  set_angle(Trajectory[index]);

	  P_Kp = get_Pkp();
	  P_Ki = get_Pki();
	  P_Kd = get_Pkd();
	  P_eint = get_Peint();

	  P_count = encoder_counts();
	  P_count = encoder_counts();
	  angle = (float) 360*(P_count-CENTER)/(448*4);
	  P_ref = get_angle();
	  P_e = P_ref - angle;
	  P_eint = P_eint + P_e;
	  P_edot = (P_e - P_eprev)/0.005;
	  P_u = P_Kp*P_e + P_Ki*P_eint + P_Kd*P_edot;

	  if(P_u > 600) {
		P_u = 600;
	  }
	  else if(P_u < -600) {
		P_u = -600;
	  }

	  P_eprev = P_e;
	  set_Peint(P_eint);

	  if(P_e > 0) {
	  	dir = 0;
	  }
	  else {
	  	dir = 1;
	  }

	  set_Iref(P_u);

	  if(StoringData == 1) {
		Path[index] = angle;
	  }
	  
	  index++;
	  if(index >= TrajLen) {
		set_mode(3);
		index = 0;
		StoringData = 0;
	  }
		
  }
  
  IFS0bits.T4IF = 0;
}

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Control_5k(void) {
  //IDLE
  if(get_mode()==0) {
	  OC1RS = 0;
	  LATDbits.LATD8 = 0;
  }

  //PWM
  if(get_mode()==1) {
	  OC1RS = pwm;
	  if(dir == 1) {
		LATDbits.LATD8 = 1;
	  }
	  else {
		LATDbits.LATD8 = 0;
	  }
  }

  //ITEST
  if(get_mode()==2) {
	  static float adcval = 0;
	  static float ref = 0;
	  static float e = 0;
	  static float u = 0;
	  static float unew = 0;
	  static float duty = 0;
	  static float Kp = 0;
	  static float Ki = 0;
	  static int counter = 0;
	  static int plotind = 0;

	  Kp = get_Ikp();
	  Ki = get_Iki();
	  eint = get_Ieint();

	  adcval = (adc_sample_convert(0)-B)*M;
	  ref = Waveform[counter];
	  e = ref - adcval;
	  eint = eint + e;
	  u = Kp*e + Ki*eint;
  
	  set_Ieint(eint);

	  duty = abs(u/6);

	  if(e > 0) {
		dir = 0;
	  }
	  else {
		dir = 1;
	  }

	  if(duty > 100) {
		duty = 100;
	  }
	  else if(duty < 0) {
		duty = 0;
	  }

	  pwm = abs((duty * 4000)/100);

	  OC1RS = pwm;
	  LATDbits.LATD8 = dir;


	  if (StoringData == 1) {
		ADCarray[plotind] = adcval;
		REFarray[plotind] = Waveform[counter];
		plotind++;
		counter++;
		if (counter == NUMSAMPS) {
		  counter = 0;
		}
		if (plotind == PLOTPTS) {
		  plotind = 0;
		  StoringData = 0;
		  set_mode(0);
		}
	  }

  }

  //HOLD and TRACK
  if(get_mode()==3 || get_mode()==4) {
	  static float ref = 0;
	  static float adcval = 0;
	  static float e = 0;
	  static float u = 0;
	  static float duty = 0;
	  static float Kp = 0;
	  static float Ki = 0;

	  Kp = get_Ikp();
	  Ki = get_Iki();
	  eint = get_Ieint();

	  adcval = (adc_sample_convert(0)-B)*M;
	  ref = get_Iref();
	  e = ref - adcval;
	  eint = eint + e;
	  u = Kp*e + Ki*eint;
  
	  if(eint > 1000) {
		set_Ieint(0);
	  }
	  else {
		set_Ieint(eint);
	  }

	  if(duty > 100) {
		duty = 100;
	  }
	  else if(duty < 0) {
		duty = 0;
	  }

	  duty = abs(u/6);

	  if(e > 0) {
		dir = 0;
	  }
	  else {
		dir = 1;
	  }

	  pwm = abs((duty * 4000)/100);

	  OC1RS = pwm;
	  LATDbits.LATD8 = dir;

  }
	 
  IFS0bits.T2IF = 0;
}

void makeWaveform(void);

int main() 
{
  char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init

  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;        

  __builtin_disable_interrupts();
  // in future, initialize modules or peripherals here
  LCD_Setup();
  encoder_init();
  adc_init();
  currentcont_init();
  positioncont_init();
  __builtin_enable_interrupts();

  set_mode(0);
  makeWaveform();

  while(1)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;                   // clear the error LED
    switch (buffer[0]) {
      case 'a': 
      {
		sprintf(buffer,"%d\r\n", adc_sample_convert(0));
        NU32_WriteUART3(buffer);
        break;
      }

      case 'b': 
      {
		sprintf(buffer,"%5.3f\r\n", (adc_sample_convert(0)-B)*M);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'c': 
      {
        sprintf(buffer,"%d\r\n", encoder_counts());
		sprintf(buffer,"%d\r\n", encoder_counts());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'd':                      // dummy command for demonstration purposes
      {
		double deg = 0.0;
        int count = 0;

		count = encoder_counts();
		count = encoder_counts();
		deg = (double) 360*(count-CENTER)/(448*4);

		sprintf(buffer,"%f\r\n", deg);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'e': 
      {
        sprintf(buffer,"%d\r\n", encoder_reset());
		sprintf(buffer,"%d\r\n", encoder_counts());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'f': 
      {
		int duty = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
		

		__builtin_disable_interrupts();
		sscanf(buffer, "%d", &duty);

		if(duty > 0) {
		  dir = 0;
		}
		else {
		  dir = 1;
		}
		pwm = abs((duty * 4000)/100);
		set_mode(1);

		__builtin_enable_interrupts();

		sprintf(buffer,"%d\r\n", get_mode());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'g': 
      {
        float kptemp = 0;
		float kitemp = 0;

        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%f", &kptemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%f", &kitemp);
		
		set_Igains(kptemp, kitemp);
		
        sprintf(buffer,"%d\r\n", 1);
        NU32_WriteUART3(buffer);

        break;
      }

      case 'h': 
      {
        sprintf(buffer,"%f\r\n", get_Ikp());
        NU32_WriteUART3(buffer);
		sprintf(buffer,"%f\r\n", get_Iki());
        NU32_WriteUART3(buffer);

        break;
      }

	  case 'i': 
      {
        float kptemp = 0;
		float kitemp = 0;
		float kdtemp = 0;

        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%f", &kptemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%f", &kitemp);
		NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%f", &kdtemp);
		
		set_Pgains(kptemp, kitemp, kdtemp);
		
        sprintf(buffer,"%d\r\n", 1);
        NU32_WriteUART3(buffer);

        break;
      }

      case 'j': 
      {
        sprintf(buffer,"%f\r\n", get_Pkp());
        NU32_WriteUART3(buffer);
		sprintf(buffer,"%f\r\n", get_Pki());
        NU32_WriteUART3(buffer);
		sprintf(buffer,"%f\r\n", get_Pkd());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'k': 
      {
		int i = 0;
        set_mode(2);
		set_Ieint(0);
		StoringData = 1;

		while (StoringData) {
		  ;
		}

		sprintf(buffer,"%d\r\n", PLOTPTS);
		NU32_WriteUART3(buffer);

		for (i=0; i<PLOTPTS; i++) {
		  sprintf(buffer, "%d %d\r\n", ADCarray[i], REFarray[i]);
		  NU32_WriteUART3(buffer);
		}
        break;
      }

	case 'l': 
      {
        int ang_temp = 0;

        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%d", &ang_temp);

		set_angle(ang_temp);
		set_mode(3);
		
        //sprintf(buffer,"%d\r\n", 1);
        //NU32_WriteUART3(buffer);
        break;
      }

      case 'r': 
      {
		sprintf(buffer,"%d\r\n", get_mode());
        NU32_WriteUART3(buffer);
        break;
      }

	  case 'm': 
      {
		int i = 0;
		int N = 0;
		float reftest = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%d", &N);
		TrajLen = N;

		for (i=0; i<N; i++) {

		  NU32_ReadUART3(buffer,BUF_SIZE);
		  sscanf(buffer, "%f", &reftest);
		  
		  if(i < REFLEN) {
			Trajectory[i] = reftest;
		  }
		  else {
			TrajLen = REFLEN;
		  }
		}

		sprintf(buffer,"%d\r\n", 1);
        NU32_WriteUART3(buffer);
        break;
      }


	  case 'n': 
      {
		int i = 0;
		int N = 0;
		float reftest = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%d", &N);
		TrajLen = N;

		for (i=0; i<N; i++) {

		  NU32_ReadUART3(buffer,BUF_SIZE);
		  sscanf(buffer, "%f", &reftest);
		  
		  if(i < REFLEN) {
			Trajectory[i] = reftest;
		  }
		  else {
			TrajLen = REFLEN;
		  }
		}

		sprintf(buffer,"%d\r\n", 1);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'o':
      {
		int i = 0;

		set_mode(4);
		StoringData = 1;

		while (StoringData) {
		  ;
		}

		sprintf(buffer,"%d\r\n", TrajLen);
		NU32_WriteUART3(buffer);

		for (i=0; i<TrajLen; i++) {
		  sprintf(buffer, "%f %f\r\n", Trajectory[i], Path[i]);
		  NU32_WriteUART3(buffer);
		}

        break;
	  }

      case 'p':
      { 
        __builtin_disable_interrupts();
		dir = 0;
		pwm = 0;

		set_mode(0);
		__builtin_enable_interrupts();

		sprintf(buffer,"%d\r\n", get_mode());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'q':
      {
        __builtin_disable_interrupts();
		dir = 0;
		pwm = 0;

		set_mode(0);
		__builtin_enable_interrupts();
        break;
      }

      case 'x':   
      {
        int n = 0;
		int m = 0;
		int x= 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%d", &n);
        NU32_ReadUART3(buffer,BUF_SIZE);
		sscanf(buffer, "%d", &m);

		x = n + m;
        sprintf(buffer,"%d\r\n", x);
        NU32_WriteUART3(buffer);
        break;
      }

      default:
      {
        NU32_LED2 = 0;  // turn on LED2 to indicate an error
        break;
      }
    }
  }
  return 0;
}

void makeWaveform(void) {
  int i = 0;
  int center = 0, A = 200;
  for (i = 0; i < NUMSAMPS; ++i) {
    if ((i < NUMSAMPS/2)) {
	  Waveform[i] = center + A;
	}
	else {
	  Waveform[i] = center - A;
	}
  }
}
