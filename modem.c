#include "common.h"
#include "device.h"
#include "task.h"
#include "modem.h"
#include "sms.h"


void _modem_packet_init(void);
void _modem_packet_process(void);


void _modem_packet_init()
{
    modem_status.padding_match = 0;
    modem_status.buff[0] = '\0';
    modem_status.index = 0;
}

void modem_init()
{
    modem_rst(1);
    _delay_ms(10);
    modem_rst(0);
    _delay_ms(200);
    modem_rst(1);
    _delay_ms(10);

    modem_powon(0);
    _delay_ms(10);
    modem_powon(1);
    _delay_ms(200);
    modem_powon(0);
    _delay_ms(10);

    UART_MODEM_UCSRB = _BV(RXCIE) | _BV(RXEN) | _BV(TXEN);
    UART_MODEM_UBRRH = 0;
    UART_MODEM_UBRRL = 5;       /* 115200bps @ 11.0592Mhz */

    modem_status.status = MS_IDLE;
    modem_status.new_msg = 0;
    modem_status.dump = 0;

    m_raw_queue.queue[0] = '\0';
    m_raw_queue.in = 0;
    m_raw_queue.out = 0;
    m_raw_queue.count = 0;

    modem_status.ring_count = 0;
    modem_status.last_incoming_no[0] = 0;
    modem_status.last_rev = -1;
    modem_status.last_csq = 0;
    
    _modem_packet_init();
}

void modem_powon(unsigned char hi_low)
{
    if (!hi_low)
    {
        IO_MODEM_POW_PORT |= _BV(IO_MODEM_POW_CH);
    }
    else
    {
        IO_MODEM_POW_PORT &= ~( _BV(IO_MODEM_POW_CH) );
    }
}

void modem_rst(unsigned char hi_low)
{
    if (hi_low)
    {
        IO_MODEM_RST_PORT |= _BV(IO_MODEM_RST_CH);
    }
    else
    {
        IO_MODEM_RST_PORT &= ~( _BV(IO_MODEM_RST_CH) );
    }
}

unsigned char modem_is_simcard_ready(void)
{
    return (IO_SIM_DETECT_PIN & _BV(IO_SIM_DETECT_CH));
}

void modem_sendc(char c)
{
    loop_until_bit_is_set(UART_MODEM_UCSRA, UDRE);
    UART_MODEM_UDR = c;
}

void modem_sends_np(char *data)
{
    unsigned int len = strlen(data);
    unsigned int i = 0;

    for (i=0; i<len; i++)
    {
        modem_sendc(data[i]);
    }
}

void modem_sends(char *data)
{
    modem_sends_np(data);

    modem_sendc('\r');
    modem_sendc('\n');
}



ISR (UART_MODEM_RX_VECT)
{
    static unsigned char recv;
    recv = UART_MODEM_UDR;

    if (m_raw_queue.count >= MODEM_RAW_BUFF_MAX)
    {                           /* buffer full */
        m_raw_queue.queue[0] = recv;
        m_raw_queue.in = 1;
        m_raw_queue.out = 0;
        m_raw_queue.count = 1;
    }
    else
    {                           /* enqueue normally */
        m_raw_queue.queue[m_raw_queue.in] = recv;
        m_raw_queue.in = (m_raw_queue.in + 1) % MODEM_RAW_BUFF_MAX;
        m_raw_queue.count++;
    }
}

signed char modem_dequeue()
{
    char rev;

    if (m_raw_queue.count == 0)
    {                           /* no data */
        rev = -1;
    }
    else
    {                           /* dequeue normally */
        rev = m_raw_queue.queue[m_raw_queue.out];
        m_raw_queue.out = (m_raw_queue.out + 1) % MODEM_RAW_BUFF_MAX;
        m_raw_queue.count--;
    }

    return rev;
}


void modem_tick(void)
{
    signed char recv;

    cli();
    recv = modem_dequeue();
    sei();

    if (recv == -1)
    {
        return;
    }
    else
    { /* */ }

    if (recv == '\r' || recv == '\n') /* padding characters */
    {
        if (modem_status.index == 0)
        {                       /* a null packet */
            
        }
        else
        {
            switch (modem_status.padding_match)
            {
            case 0:
                if (recv == '\r')
                {
                    modem_status.padding_match = 1;
                }
                else
                {               /* a illegal packet */
                    _modem_packet_init();
                }

                break;
            case 1:
                if (recv == '\n')
                {               /* a packet ready */
                    modem_status.buff[modem_status.index] = '\0';
                    _modem_packet_process();

                    _modem_packet_init();
                }
                else
                {               /* a illegal packet */
                    _modem_packet_init();
                }

                break;
            default:
                break;
            }
        }
    }
    else
    {                           /* normal character */
        modem_status.buff[modem_status.index] = recv;
        modem_status.index++;

        if (modem_status.index >= MODEM_BUFF_MAX) /* huge packet */
        {
            _modem_packet_init();
        }
    }
}

void _modem_packet_process()
{
    unsigned char len;
    unsigned char buf[80];
    
    
    if ( !strcmp_P(modem_status.buff, PSTR("OK")) )
    {
        modem_status.last_rev = 0;
    }
    else if ( !strcmp_P(modem_status.buff, PSTR("ERROR")) )
    {
        modem_status.last_rev = -1;
    }
    else if ( !strncmp_P(modem_status.buff, PSTR("Incoming number = "), 18) )
    {
        modem_status.status = MS_INCOMING_CALL;
        
        strncpy(modem_status.last_incoming_no, modem_status.buff+18, 16);

        if (task_sm.status == TASK_WAIT_LORD)
        {
            len = strlen(modem_status.last_incoming_no);
            if (len > 3 && len < 16)
            {
                task_sm.status = TASK_IDLE;
                store(len, modem_status.last_incoming_no);
                loadfrom_rom();
                
                task_sm.piezo0_count = 32;
            }
        }
    }
    else if ( !strncmp_P(modem_status.buff, PSTR("RING"), 4) )
    {
        task_sm.piezo0_count = 32;

        modem_status.ring_count++;
        if (modem_status.ring_count >= MODEM_HANG_AFTER_COUNT)
        {   
            modem_status.ring_count = 0;
            modem_sends("AT+CHUP"); // modem_sends("ATH");

            if ( !strcmp(modem_status.last_incoming_no, task_sm.lord_no) )
            {
                modem_status.last_call_type = VOICE_CALL;
                task_sm.callback_delay_count = MODEM_WAIT_CALLBACK_TIME * TASK_TICK_FREQ;
            }
            else
            {
                _delay_ms(10);
                sprintf(buf, "[!] a call from <%s>, i hung it up",
                        modem_status.last_incoming_no);
                sms_send(task_sm.lord_no, buf);
            }
        }
    }
    else if ( !strcmp_P(modem_status.buff, PSTR("VPRING")) )
    {
        task_sm.piezo0_count = 64;

        modem_status.ring_count++;
        if (modem_status.ring_count >= MODEM_HANG_AFTER_COUNT)
        {
            modem_status.ring_count = 0;
            modem_sends("AT+LSVPH");

            if ( !strcmp(modem_status.last_incoming_no, task_sm.lord_no) )
            {
                modem_status.last_call_type = VIDEO_CALL;
                task_sm.callback_delay_count = MODEM_WAIT_CALLBACK_TIME * TASK_TICK_FREQ;
            }
            else
            {
                _delay_ms(10);
                sprintf(buf, "[!] a video call from <%s>, i hung it up",
                        modem_status.last_incoming_no);
                sms_send(task_sm.lord_no, buf);
            }
        }
    }
    else if ( !strcmp_P(modem_status.buff, PSTR("Call end")) )
    {
        modem_status.status = MS_IDLE;
        modem_status.ring_count = 0;

        // sms_send(task_sm.lord_no, modem_status.last_incoming_no);
    }
    else if ( !strcmp_P(modem_status.buff, PSTR("Call connect")) )
    {
        modem_status.status = MS_CONNECTED;
    }
    else if ( !strcmp_P(modem_status.buff, PSTR("Call end")) )
    {
        modem_status.status = MS_IDLE;
    }
    else if ( !strncmp_P(modem_status.buff, PSTR("VOCDTMF = "), 10) ||
              !strncmp_P(modem_status.buff, PSTR("VPDTMF = "), 9) )
    {
        if (modem_status.buff[1] == 'O')
        {
            modem_status.last_dtmf = modem_status.buff[10];
        }
        else
        {
            modem_status.last_dtmf = modem_status.buff[9];
        }

        switch (modem_status.last_dtmf)
        {
        case '2':
            task_set_pos(MOTOR_A, task_sm.motor_a_pos+=5);
            
            break;
        case '8':
            task_set_pos(MOTOR_A, task_sm.motor_a_pos-=5);

            break;
        case '4':
            task_set_pos(MOTOR_B, task_sm.motor_b_pos+=5);

            break;
        case '6':
            task_set_pos(MOTOR_B, task_sm.motor_b_pos-=5);

            break;
        case '5':
            task_set_pos(MOTOR_A, 50.0);
            task_set_pos(MOTOR_B, 50.0);

            break;
        case '*':
            task_change_irled(0);
            
            break;

        case '#':
            task_set_pump(1);

            break;
        case '1':
            task_sm.piezo0_count = TASK_TICK_FREQ;
            task_sm.piezo1_count = TASK_TICK_FREQ;

            break;
        default:
            task_sm.piezo0_count = 16;
            
            break;
        }
    }
    else if ( !strncmp_P(modem_status.buff, PSTR("+CSQ:"), 5) )
    {
        modem_status.last_csq = atoi(&modem_status.buff[6]);
    }
    else if ( !strncmp_P(modem_status.buff, PSTR("+CMTI: "), 7) )
    {
        sms_get(modem_status.buff[12]);
    }
    else if ( !strncmp_P(modem_status.buff, PSTR("+CMGR: "), 7) )
    {
        task_sm.piezo0_count = 25; /* beep.. you got a new msg */
        
        modem_status.new_msg = 1;
    }
    else if ( modem_status.new_msg ) /* maybe add other condition */
    {
        modem_status.new_msg = 0;

        sms_process();
    }
}
