#ifndef __STORE_H
#define __STORE_H 1

#include <avr/eeprom.h>

void loadfrom_rom(void);
void store(signed char len, char *buf);


// #define BOOTLOAD_DONE 0x00

#define LORD_NUMBER_LEN  0x10
#define LORD_NUMBER      0x11



#endif
