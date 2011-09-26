#include <util/twi.h>

#include "common.h"
#include "ds1721.h"

#include "task.h"


void i2c_init()
{
    TWSR = 0x00;
    // TWBR = 1;                  // (F_CPU / 100000UL - 16) / 2;
    TWBR = 47;
    TWCR = _BV(TWEN);
}

void i2c_wait_finish(void)
{
    // loop_until_bit_is_set(TWCR, TWINT);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_send_start(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    i2c_wait_finish();
}

void i2c_send_stop(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
    // loop_until_bit_is_clear(TWCR, TWSTO);
    while (TWCR & (1 << TWSTO));
}

void i2c_send_byte(unsigned char c)
{
    TWDR = c;
    TWCR = _BV(TWINT) | _BV(TWEN);
    i2c_wait_finish();
}

unsigned char i2c_read_byte(unsigned char ACK)
{
    TWCR = _BV(TWINT) | _BV(TWEN) | (ACK << TWEA);
    loop_until_bit_is_set(TWCR, TWINT);
    TWCR = _BV(TWINT) | _BV(TWEN);

    return TWDR;
}


void ds1721_init()
{
    i2c_init();

    i2c_send_start();
    i2c_send_byte(DS1721_ADDR | DS1721_WRITE);
    i2c_send_byte(DS1721_CMD_INIT);
    i2c_send_stop();

    _delay_us(10);
}

float ds1721_read()
{
    unsigned char hi, lo;
    unsigned char i;
    short tmp = 1;
    float rev;
    float frac[] = { 0.0625, 0.125, 0.25, 0.5 };
    
    i2c_send_start();
    i2c_send_byte(DS1721_ADDR | DS1721_WRITE);
    i2c_send_byte(DS1721_CMD_READ);

    i2c_send_start();           /* restart */
    i2c_send_byte(DS1721_ADDR | DS1721_READ);
    hi = i2c_read_byte(1);
    lo = i2c_read_byte(1);
    // _delay_us(70);              /* WHAT ?! */
    // i2c_send_stop();
    // task_sm.piezo0_count = 5;

    ////////////////
    ////////////////
    
    if (hi & 0x80)
    {                           /* negative */
        tmp = (hi << 8) + lo;
        tmp = ~tmp;
        tmp++;

        hi = tmp >> 8;
        lo = tmp & 0x00ff;

        tmp = -1;
    }
    
    rev = hi;
    lo >>= 4;

    for (i=0; i<4; i++)
    {
        if (lo & _BV(i))
        {
            rev += frac[i];
        }
    }

    return rev * tmp;
}
