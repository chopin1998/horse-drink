#ifndef _MODEM_H
#define _MODEM_H 1


void modem_init(void);
void modem_sendc(char);
void modem_sends_np(char *);
void modem_sends(char *);

void modem_powon(unsigned char);
void modem_rst(unsigned char);
signed char modem_dequeue(void);
void modem_tick(void);

unsigned char modem_is_simcard_ready(void);


#define MODEM_HANG_AFTER_COUNT 3
#define MODEM_WAIT_CALLBACK_TIME 20 /* unit: second */

enum { VOICE_CALL, VIDEO_CALL };

enum{ MS_IDLE, MS_INCOMING_CALL, MS_CONNECTED, MS_WAIT_CALLBACK, MS_SMSING};

#define MODEM_RAW_BUFF_MAX 512
typedef struct
{
    unsigned char queue[MODEM_RAW_BUFF_MAX];
    unsigned char in, out;
    unsigned char count;
} _m_raw_queue;
volatile _m_raw_queue m_raw_queue;


#define MODEM_BUFF_MAX 384
typedef struct
{
    unsigned char status;
    unsigned char new_msg;
    
    unsigned char padding_match;
    unsigned char buff[MODEM_BUFF_MAX];
    unsigned char index;

    unsigned char ring_count;
    unsigned char last_incoming_no[16];
    unsigned char last_call_type;
    signed char last_rev;
    unsigned char last_dtmf;

    unsigned char last_csq;

    unsigned char dump;
    
} _modem_status;
volatile _modem_status modem_status;


#endif
