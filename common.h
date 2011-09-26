/*
 * common.h include all common header file
 */

#ifndef _COMMON_H
#define _COMMON_H 1

/*
  The MACRO will send in ver command
*/
#define VERSION "+ok|0.1"

#define F_CPU 11059200

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <math.h>

#define DEBUG 0

#endif
/* .end of the file */
