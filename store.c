#include "common.h"
#include "store.h"
#include "task.h"

void loadfrom_rom(void)
{
    cli();
    
    eeprom_busy_wait();
    task_sm.lord_no_len = eeprom_read_byte(LORD_NUMBER_LEN);

    if (task_sm.lord_no_len > 3 && task_sm.lord_no_len < 16)
    {
        eeprom_busy_wait();
        eeprom_read_block(task_sm.lord_no, LORD_NUMBER, task_sm.lord_no_len);
    }
    else
    {
        task_sm.lord_no_len = -1;
    }

    sei();
}

void store(signed char len, char *buf)
{
    cli();
    
    if (len > 3 && len < 16)
    {
        eeprom_busy_wait();
        eeprom_write_byte(LORD_NUMBER_LEN, len);

        eeprom_busy_wait();
        eeprom_write_block(buf, LORD_NUMBER, len);
    }

    sei();
}
