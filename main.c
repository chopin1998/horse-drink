/*
 * 
 * 
 */

#define __DELAY_BACKWARD_COMPATIBLE__ 
#include "common.h"
#include "task.h"
#include "device.h"

#include "modem.h"
#include "m_sensor.h"
#include "sms.h"
#include "ds1721.h"
#include "adc.h"

void init_io(void);


void _collector()
{
    task_sm.temperature = ds1721_read();
    task_sm.ldr = task_sm.ldr * 0.9 + adc_read(ADC_LDR_CH) * 0.1;
    task_sm.bat = task_sm.bat * 0.85 + adc_read(ADC_BATT_CH) * ((4.3 + 1.5) / 1.5) * 0.15;
}

void _check_m_sensors()
{
    unsigned char buf[80];
    unsigned char i = 0;
    
    if (m_sensor.m0_alart)
    {
        m_sensor_init();
            
        sms_send(task_sm.lord_no, "somebody fucking me!");
        /*
        sprintf(buf, "somebody fucking me!\ntemp: %.4f\n batt: %.3f\n ldr: %d\n",
                task_sm.temperature, task_sm.bat*(3.3/1024), task_sm.ldr);
        sms_send(task_sm.lord_no, buf);
        */
            
        cli();
        for (i=0; i<9; i++)
        {
            IO_PIEZO_1_PORT |= _BV(IO_PIEZO_1_CH);
            IO_PIEZO_0_PORT |= _BV(IO_PIEZO_0_CH);
            IOS_LED_PORT |= _BV(IOS_LED_2);
            _delay_ms(200 + i*200);

            IO_PIEZO_1_PORT &= ~( _BV(IO_PIEZO_1_CH) );
            IO_PIEZO_0_PORT &= ~( _BV(IO_PIEZO_0_CH) );
            IOS_LED_PORT &= ~( _BV(IOS_LED_2) );
            _delay_ms(500 - i*50);
        }
        sei();
    }
        
    if (m_sensor.m1_alart)
    {
        m_sensor_init();

        sprintf(buf, "horse drinking...\ntemp: %.4f\n batt: %.3f\n ldr: %d\n",
                task_sm.temperature, task_sm.bat*(3.3/1024), task_sm.ldr);
        sms_send(task_sm.lord_no, buf);
        
        task_sm.piezo0_count = 20;
        task_set_pump(1);
    }
}


int main(void)
{
    init_io();

    modem_init();
    task_init();
    m_sensor_init();
    ds1721_init();
    adc_init();

    /*
    PGM_P string_1 = PSTR("String 1");
    if ( !strcmp_P("String 1", string_1) )
    {
        for (;;)
        {
            IOS_LED_PORT ^= _BV(IOS_LED_3);
            _delay_ms(100);
        }
    }
    */
    

    loadfrom_rom();
    if (task_sm.lord_no_len == -1)
    {
        task_sm.status = TASK_WAIT_LORD;
    }
    
    sei();

    for (unsigned char i=0; i<16; i++)
    {
        IOS_LED_PORT ^= _BV(IOS_LED_3);
        _delay_ms(50);
    }

    for(unsigned int i=0;;i++)
    {
        modem_tick();

        if (!i)
        {
            IOS_LED_PORT ^= _BV(IOS_LED_3);

            _collector();
        }

        _check_m_sensors();
    }

    cli();
    wdt_enable(WDTO_500MS);     /* should not run here */
    for(;;);
}

void init_io()
{
    /* input, no pull up */
    ADC_PORT &= ~( _BV(ADC_LDR_CH) | _BV(ADC_BATT_CH) );
    ADC_DDR &= ~( _BV(ADC_LDR_CH) | _BV(ADC_BATT_CH) );

    /* output */
    IO_PIEZO_0_PORT &= ~( _BV(IO_PIEZO_0_CH) ); /* set 0 */
    IO_PIEZO_0_DDR &= ~( _BV(IO_PIEZO_0_CH) ); /* set 0 */

    /* output */
    IO_PIEZO_1_PORT &= ~( _BV(IO_PIEZO_1_CH) );
    IO_PIEZO_1_DDR &= ~( _BV(IO_PIEZO_1_CH) );

    /* input */
    IO_SIM_DETECT_PORT &= ~( _BV(IO_SIM_DETECT_CH) );
    IO_SIM_DETECT_DDR &= ~( _BV(IO_SIM_DETECT_CH) );

    /* output */
    // IO_MODEM_POW_PORT &= ~( _BV(IO_MODEM_POW_CH) );
    IO_MODEM_POW_PORT |= _BV(IO_MODEM_POW_CH);
    IO_MODEM_POW_DDR |= _BV(IO_MODEM_POW_CH);

    /* output */
    // IO_MODEM_RST_PORT &= ~( _BV(IO_MODEM_RST_CH) );
    IO_MODEM_RST_PORT |= _BV(IO_MODEM_RST_CH);
    IO_MODEM_RST_DDR |= _BV(IO_MODEM_RST_CH);

    /* output */
    IO_IRLED_PORT &= ~( _BV(IO_IRLED_CH) );
    // IO_IRLED_PORT |= _BV(IO_IRLED_CH);
    IO_IRLED_DDR |= _BV(IO_IRLED_CH);

    /* output */
    IO_PUMP_PORT &= ~( _BV(IO_PUMP_CH) );
    IO_PUMP_DDR |= _BV(IO_PUMP_CH);

    /* input */
    IOS_KEY_PORT &= ~( IOS_KEY_MASK );
    IOS_KEY_DDR &= ~( IOS_KEY_MASK );

    /* output */
    IOS_LED_PORT &= ~( IOS_LED_MASK );
    IOS_LED_DDR |= IOS_LED_MASK;

    /* output */
    IO_MOTOR_PWM_A_PORT &= ~( _BV(IO_MOTOR_PWM_A_CH) );
    IO_MOTOR_PWM_A_DDR |= _BV(IO_MOTOR_PWM_A_CH);

    /* output */
    IO_MOTOR_PWM_B_PORT &= ~( _BV(IO_MOTOR_PWM_B_CH) );
    IO_MOTOR_PWM_B_DDR |= _BV(IO_MOTOR_PWM_B_CH);

    /* input */
    IO_MODEM_RTS_PORT &= ~( _BV(IO_MODEM_RTS_CH) );
    IO_MODEM_RTS_DDR &= ~( _BV(IO_MODEM_RTS_CH) );

    /* input */
    IO_MODEM_CTS_PORT &= ~( _BV(IO_MODEM_CTS_CH) );
    IO_MODEM_CTS_DDR &= ~( _BV(IO_MODEM_CTS_CH) );

    UART_DDR |= _BV(UART_TX_CH);
    UART_DDR &= ~( _BV(UART_RX_CH) );
    UART_PORT &= ~( _BV(UART_TX_CH) | _BV(UART_RX_CH) );

    /* input */
    IO_MOTION_0_PORT &= ~( _BV(IO_MOTION_0_CH) );
    IO_MOTION_0_DDR &= ~( _BV(IO_MOTION_0_CH) );
    IO_MOTION_1_PORT &= ~( _BV(IO_MOTION_1_CH) );
    IO_MOTION_1_DDR &= ~( _BV(IO_MOTION_1_CH) );
}
