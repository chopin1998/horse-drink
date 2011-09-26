#ifndef _DEVICE_H
#define _DEVICE_H

#include "common.h"


#define UART_MODEM_UCSRA  UCSRA
#define UART_MODEM_UCSRB  UCSRB
#define UART_MODEM_UCSRC  UCSRC
#define UART_MODEM_UBRRH  UBRRH
#define UART_MODEM_UBRRL  UBRRL
#define UART_MODEM_UDR    UDR
#define UART_MODEM_RX_VECT   USART_RXC_vect

#define UART_PORT        PORTD
#define UART_DDR         DDRD
#define UART_TX_CH       PD1
#define UART_RX_CH       PD0


#define ADC_PORT          PORTA
#define ADC_DDR           DDRA
#define ADC_LDR_CH        0
#define ADC_BATT_CH       1


#define IO_PIEZO_0_PORT      PORTD
#define IO_PIEZO_0_DDR       DDRD
#define IO_PIEZO_0_CH        PD7

#define IO_PIEZO_1_PORT      PORTD
#define IO_PIEZO_1_DDR       DDRD
#define IO_PIEZO_1_CH        PD6

#define IO_SIM_DETECT_PORT   PORTB
#define IO_SIM_DETECT_DDR    DDRB
#define IO_SIM_DETECT_PIN    PINB
#define IO_SIM_DETECT_CH     PB3

#define IO_MODEM_POW_PORT    PORTB
#define IO_MODEM_POW_DDR     DDRB
#define IO_MODEM_POW_CH      PB0

#define IO_MODEM_RST_PORT    PORTB
#define IO_MODEM_RST_DDR     DDRB
#define IO_MODEM_RST_CH      PB1

#define IO_MODEM_RTS_PORT    PORTC
#define IO_MODEM_RTS_DDR     DDRC
#define IO_MODEM_RTS_CH      PC2

#define IO_MODEM_CTS_PORT    PORTC
#define IO_MODEM_CTS_DDR     DDRC
#define IO_MODEM_CTS_PIN     PINC
#define IO_MODEM_CTS_CH      PC3

#define IO_IRLED_PORT        PORTB
#define IO_IRLED_DDR         DDRB
#define IO_IRLED_CH          PB4

#define IO_PUMP_PORT         PORTB
#define IO_PUMP_DDR          DDRB
#define IO_PUMP_CH           PB2

#define IO_MOTION_0_PORT     PORTD
#define IO_MOTION_0_DDR      DDRD
#define IO_MOTION_0_PIN      PIND
#define IO_MOTION_0_CH       PD2

#define IO_MOTION_1_PORT     PORTD
#define IO_MOTION_1_DDR      DDRD
#define IO_MOTION_1_PIN      PIND
#define IO_MOTION_1_CH       PD3


#define IOS_KEY_PORT         PORTA
#define IOS_KEY_DDR          DDRA
#define IOS_KEY_PIN          PINA
#define IOS_KEY_MASK         0x3c /* PA5 - PA2 */
// #define IOS_KEY_MASK         (PA2 | PA3 | PA4 | PA5)
#define IOS_KEY_0            PA2
#define IOS_KEY_1            PA3
#define IOS_KEY_2            PA4
#define IOS_KEY_3            PA5
#define IOS_KEY_SHIFT        2

#define IOS_LED_PORT         PORTC
#define IOS_LED_DDR          DDRC
#define IOS_LED_MASK         0xf0 /* PC7 - PC4 */
// #define IOS_LED_MASK         (PC7 | PC6 | PC5 | PC4)
#define IOS_LED_0            PC7
#define IOS_LED_1            PC6
#define IOS_LED_2            PC5
#define IOS_LED_3            PC4
#define IOS_LED_SHIFT        4

#define IO_MOTOR_PWM_A_PORT  PORTD
#define IO_MOTOR_PWM_A_DDR   DDRD
#define IO_MOTOR_PWM_A_CH    PD4

#define IO_MOTOR_PWM_B_PORT  PORTD
#define IO_MOTOR_PWM_B_DDR   DDRD
#define IO_MOTOR_PWM_B_CH    PD5


#endif
