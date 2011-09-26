#include "common.h"
#include "task.h"
#include "device.h"
#include "modem.h"
#include "m_sensor.h"

static void task_timer_init(void);
static void task_motor_init(void);


static void task_timer_init(void)
{
    /* use timer0 as task timer */

    /* set timer 0 as CTC mode
       1024 pre_clock
       100 Hz
    */
    TCCR0 = _BV(WGM01);
    
#if (TASK_TIMER_PRECLOCK == 1024)
    TCCR0 |= _BV(CS02) | _BV(CS00);
#endif
    
    OCR0 = TASK_TIMER_OCR - 1;

    TIMSK = _BV(OCIE0);
}

static void task_motor_init(void)
{
    /* use timer1 as motor pwm */

    /* set timer1 as fast pwm mode
       64 pre_clock
       icr1 set as 3110
       so,  base-freq is 11059200 / 64 / 3100
    */

    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10);

    ICR1 = MOTOR_PWM_TOP;
    OCR1A = 0;
    OCR1B = 0;
}



inline void _key_process(unsigned char keys)
{
    static unsigned int count = 0;
    
    
    if ( ! ( keys & ( _BV(IOS_KEY_0) | _BV(IOS_KEY_1) ) ) )
    {
        count++;

        if (count > TASK_TICK_FREQ * 4 / KEY_DEBOUNCE_COUNT)
        {
            count = 0;
        }
        else if (count > TASK_TICK_FREQ * 2 / KEY_DEBOUNCE_COUNT)
        {
            task_sm.piezo0_count = 1;
                
            if (! (count % 2) )
                IOS_LED_PORT ^= _BV(IOS_LED_0);

            task_sm.status = TASK_WAIT_LORD;
        }
        else if (count > TASK_TICK_FREQ / KEY_DEBOUNCE_COUNT)
        {
            task_sm.piezo0_count = 5;
        }
    }
    else
    {
        count = 0;
        // task_sm.piezo0_count = 1;
            
        IOS_LED_PORT &= ~(_BV(IOS_LED_0));
    }

    if ( ! (keys & (_BV(IOS_KEY_2))) )
    {
        modem_sends("AT+CSQ");
        // modem_sends("AT+CMGF=0");
        // task_sm.temperature = ds1721_read();
    }
}


inline void _check_csq(void)
{
    unsigned char buf[16];
    
    if (modem_status.status == MS_IDLE)
    {
        if (modem_status.last_csq < 12)
        {
            task_sm.csq_led_duty = 1;
        }
        else if (modem_status.last_csq > 24)
        {
            task_sm.csq_led_duty = 255;
        }
        else
        {
            task_sm.csq_led_duty = ( modem_status.last_csq - 10) * 15;
        }
        
        modem_status.last_csq = 0;
        modem_sends("AT+CSQ");
    }
}


inline void _minute_timer(void)
{
    _check_csq();
}

ISR(TIMER0_COMP_vect)
{
    /*
     * task timer
     */

    unsigned char buf[32];
    
    static unsigned char key_debounce = 0;
    static unsigned char last_keys;
    unsigned char keys;

    static unsigned int minute_timer_count = 0;

    if (1)
    {
        if (minute_timer_count >= MINUTE_INTERVAL)
        {
            minute_timer_count = 0;
            _minute_timer();
        }
        else
        {
            minute_timer_count++;
        }
    }

    if (task_sm.motor_enabled)
    {
        OCR1A = task_sm.motor_a_ocr;
        OCR1B = task_sm.motor_b_ocr;
    }

    if (task_sm.piezo0_count)
    {
        task_sm.piezo0_count--;
        if (task_sm.piezo0_count <= 0)
        {
            IO_PIEZO_0_PORT &= ~( _BV(IO_PIEZO_0_CH) );
        }
        else
        {
            IO_PIEZO_0_PORT |= _BV(IO_PIEZO_0_CH);
        }
    }

    if (task_sm.piezo1_count)
    {
        task_sm.piezo1_count--;
        if (task_sm.piezo1_count <= 0)
        {
            IO_PIEZO_1_PORT &= ~( _BV(IO_PIEZO_1_CH) );
        }
        else
        {
            IO_PIEZO_1_PORT |= _BV(IO_PIEZO_1_CH);
        }
    }

    if (task_sm.pump_count)
    {
        task_sm.pump_count--;
        if (task_sm.pump_count <= 0)
        {
            IO_PUMP_PORT &= ~( _BV(IO_PUMP_CH) );
        }
        else
        {
            IO_PUMP_PORT |= _BV(IO_PUMP_CH);
        }
    }

    if (task_sm.callback_delay_count)
    {
        task_sm.callback_delay_count--;
        if (task_sm.callback_delay_count == 0)
        {
            // snprintf(buf, 32, "ATD%s;", task_sm.lord_no);
            // modem_sends(buf);
            if (modem_status.last_call_type == VOICE_CALL)
            {
                modem_sends_np("ATD");
                modem_sends_np(task_sm.lord_no);
                modem_sends(";");
            }
            else if (modem_status.last_call_type == VIDEO_CALL)
            {
                modem_sends_np("AT+LSVPD=");
                modem_sends(task_sm.lord_no);
            }
        }
    }

    if (1)                      /* key processing */
    {
        keys = (IOS_KEY_PIN & IOS_KEY_MASK);
        if (keys == last_keys)
        {
            key_debounce++;
            if (key_debounce > KEY_DEBOUNCE_COUNT)
            {
                key_debounce = 0;
                
                _key_process(keys);
            }
        }
        else
        {
            last_keys = keys;
            key_debounce = 0;
        }
    }

    if (task_sm.status == TASK_WAIT_LORD)
    {
        IOS_LED_PORT |= _BV(IOS_LED_1);
    }
    else
    {
        IOS_LED_PORT &= ~(_BV(IOS_LED_1));
    }

    
    if (1)
    {
        task_sm.irled_duty_count++;
        
        if (task_sm.irled_duty_count > task_sm.irled_duty)
        {
            IO_IRLED_PORT &= ~( _BV(IO_IRLED_CH) );
        }
        else
        {
            IO_IRLED_PORT |= _BV(IO_IRLED_CH);
        }
        
        if (task_sm.irled_duty_count >= 4)
        {
            task_sm.irled_duty_count = 0;
        }
    }

    if (1)
    {
        task_sm.csq_led_duty_count++;

        if (task_sm.csq_led_duty_count > task_sm.csq_led_duty)
        {
            IOS_LED_PORT |= _BV(IOS_LED_2);
        }
        else
        {
            IOS_LED_PORT &= ~( _BV(IOS_LED_2) );
        }
    }

    
    if (1)
    {
        m_sensor_timer_tick();
    }
}


void task_init(void)
{
    task_timer_init();
    task_motor_init();

    task_sm.status = TASK_IDLE;
    
    task_set_pos(MOTOR_A, 50.0);
    task_set_pos(MOTOR_B, 50.0);

    task_sm.motor_enabled = 1;

    task_sm.piezo0_count = 100;
    task_sm.piezo1_count = 0;

    task_sm.pump_count = 0;

    task_sm.irled_duty = 0;
    task_sm.irled_duty_count = 0;

    task_sm.csq_led_duty = 10;
    task_sm.csq_led_duty_count = 0;
}

void task_change_irled(unsigned char val)
{   
    if (task_sm.irled_duty >= 4)
    {
        task_sm.irled_duty = 0;
    }
    else
    {
        task_sm.irled_duty++;
    }
}

void task_set_pump(unsigned char sec)
{
    if (task_sm.pump_count > 0)
    {
        return;                 /*  */
    }
    else
    {
        task_sm.pump_count = TASK_TICK_FREQ * sec;
    }
}

void task_set_pos(unsigned char ch, float percent)
{
    float ocr;

    if (percent < 0.0)
    {
        percent = 0.0;
    }
    else if (percent > 99.999999)
    {
        percent = 99.9999;
    }
    
    ocr = (MOTOR_MIN_POS_DUTY + percent * MOTOR_POS_STEP_DUTY) / MOTOR_PWM_BASE * MOTOR_PWM_TOP;

    if (ch == MOTOR_A)
    {
        task_sm.motor_a_pos = percent;
        task_sm.motor_a_ocr = ocr;
    }
    else
    {
        task_sm.motor_b_pos = percent;
        task_sm.motor_b_ocr = ocr;
    }
}
