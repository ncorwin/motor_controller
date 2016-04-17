#ifndef ISENSE_H
#define ISENSE_H

void adc_init(void);
int adc_counts(int pin);
unsigned int adc_sample_convert(int pin);

#endif
