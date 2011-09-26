#include "common.h"

void adc_init()
{

    ADCSRA |= _BV(ADEN);        /* global enable ADC */
    ADCSRA |= _BV(ADPS2) | _BV(ADPS0); /* 32preClk */
    ADMUX |= _BV(REFS0);               /* use AVCC as ref. */
}


unsigned int adc_read(unsigned char channel)
{

    ADCSRA |= _BV(ADEN);

    ADMUX &= 0xe0;              /* clear last channel */
    ADMUX |= channel;           /* reset channel */
    ADCSRA |= _BV(ADSC);        /* start ADChange */

    loop_until_bit_is_set(ADCSRA, ADIF); /* busy wait ADC done */
    ADCSRA |= _BV(ADIF);                 /* then clear the flag */

    return ADC;                 /* ADC -> ADCH:ADCL */
}
