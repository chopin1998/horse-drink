#ifndef _DS1721_H
#define _DS1721_H

#define DS1721_ADDR 0x90
#define DS1721_READ 0x01
#define DS1721_WRITE 0x00

#define DS1721_CMD_INIT 0x51
#define DS1721_CMD_READ 0xaa

void ds1721_init(void);
float ds1721_read(void);

#endif
