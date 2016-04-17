#ifndef POSITIONCONT_H
#define POSITIONCONT_H

void positioncont_init(void);
void set_Pgains(float p, float i, float d);
float get_Pkp(void);
float get_Pki(void);
float get_Pkd(void);
float get_Peint(void);
void set_Peint(float e);
void set_angle(int a);
int get_angle(void);

#endif
