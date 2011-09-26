#include "common.h"
#include "modem.h"
#include "sms.h"
#include "task.h"
#include "device.h"


const unsigned char hex_lookup[] = "0123456789ABCDEF";
const unsigned char mask[] = {1,3,7,15,31,63,127};
const unsigned char power[] = {128,64,32,16,8,4,2};


int sms_compress( unsigned char *in, unsigned char *discarded, unsigned char ret[] )
{
    int i,ii,iii;
    unsigned char encode_c, now_c, next_c;

    *discarded = 0;

    for( i = ii = iii = 0; ( in[i] != '\0' ) && ( i < SMS_TEXT_LENGTH ); )
    {
        now_c = in[ i++ ];
        next_c = in[ i ];

        //Last char?
        /*
        if( next_c == '\0' )
        {
            next_c = 0;
        }
        */

        encode_c = ( now_c >> ii )+ ( mask[ ii ] & next_c )*power[ ii ];
        ret[ iii++ ] = hex_lookup[ ( encode_c >> 4 ) & 0x0F ];
        ret[ iii++ ] = hex_lookup[ ( encode_c & 0x0F ) ];

        if( ii == 6 )
        {
            ii = 0;
            i++;
            (*discarded)++;
        }
        else
        {
            ii++;
        }
    }

    ret[iii] = '\0';

    return i;
}


int sms_decompress( unsigned char *compressed, unsigned char *decompressed )
{
    int i,ii,iii;
    unsigned char rest_c, ans_c ,dec_c , this_c, next_c;

    
    for( i = ii = iii = rest_c = 0; (this_c = compressed[i++]) != '\0'; )
    {
        next_c = compressed[i++];
        
        dec_c = 16 * sms_htoi( this_c ) + sms_htoi( next_c );
        ans_c = dec_c & mask[6 - ii];
        ans_c <<= ii;
        decompressed[iii++] = ans_c + rest_c;

        rest_c = (dec_c & ~mask[6 - ii]) >> ( 7 - ii );

        if( ii == 6)
        {
            ii = 0;
            decompressed[ iii++ ] = rest_c;
            rest_c = 0;
        }
        else
        {
            ii++;
        }
    }

    decompressed[ iii ] = '\0';
    return iii;
}

int sms_atoi( unsigned char *a )
{

    int i, n;

    n = 0;

    for( i=0; ( a[ i ] >= '0' ) && ( a[ i ] <= '9' ); ++i )
    {
        n = 10*n + ( a[ i ] - '0' );
    }

    return n;
}


int sms_htoi( unsigned char hex )
{

    if( ( hex >= 'A' ) && ( hex <= 'F' ) )
    {
        return hex - 'A' + 10;
    }
    else
    {
        return hex - '0';
    }
}


void sms_itoh( int n, unsigned char *ret )
{
    ret[ 0 ] = hex_lookup[ ( n >> 4 ) & 0x0F ];
    ret[ 1 ] = hex_lookup[ ( n & 0x0F ) ];
    ret[ 2 ] = '\0';
}


void sms_get(unsigned char index)
{
    unsigned char cmd[16];

    sprintf(cmd, "AT+CMGR=%c", index);
    modem_sends(cmd);
}

void sms_process()
{
    unsigned char index = 0;
    unsigned char sender_index = 0;
    unsigned char sender_len = 0;
    unsigned char sender[16];
    unsigned char sms_len = 0;
    unsigned char text[141];
    // unsigned char buf[80];

    sender[0] = 0;

    index = sms_htoi(modem_status.buff[0]) << 4;
    index += sms_htoi(modem_status.buff[1]);

    index += 1;                 /* length of smsc info */
    index *= 2;                 /* because of hex to text */
    index += 2;                 /* first octet of deliver */

    sender_index = index;
    
    sender_len = sms_htoi(modem_status.buff[index]) << 4;
    sender_len += sms_htoi(modem_status.buff[index+1]);

    index += 2;                 /* sender len itself */
    index += 2;                 /* type of sender */

    modem_str_to_no(&modem_status.buff[index], sender);
    
    if ( !!strcmp(sender, task_sm.lord_no) )
    {
        task_sm.piezo0_count = 200;
        return;
    }
    
    if (sender_len % 2 == 0)
    {
        index += sender_len;
    }
    else
    {
        index += sender_len + 1;
    }
    
    index += 2;                 /* TP-PID */
    index += 2;                 /* TP-DCS */
    index += 14;                /* TP-SCTS */
    
    sms_len = sms_htoi(modem_status.buff[index]) << 4;
    sms_len += sms_htoi(modem_status.buff[index]+1);

    index += 2;                 /* text len */

    sms_decompress(&modem_status.buff[index], text);

    if ( text[0] == '*' )
    {
        task_change_irled(0);
    }
    else if ( text[0] == '#' )
    {
        task_set_pump(1);
    }
    else if ( text[0] == '?' )
    {
        sprintf(text, "temp: %.4f\n batt: %.3f\n ldr: %d\n csq: %d\n",
                task_sm.temperature, task_sm.bat*(3.3/1024),
                task_sm.ldr, modem_status.last_csq);
        sms_send(task_sm.lord_no, text);

        // sms_send(task_sm.lord_no, "you ask..");
        task_sm.piezo0_count = 10;
    }
    else
    {
        task_sm.piezo0_count = 100;
    }

    // sprintf(tmp, "AT:%s", text);
    // modem_sends(tmp);
}


void sms_send(unsigned char *no, unsigned char *text)
{
    int payload_len = 0;
    int len = 0;

    unsigned char no_len;
    
    unsigned char payload_len_c[3];
    unsigned char jump = 0;

    unsigned char pdu_no[16];
    
    unsigned char pdu[256];
    unsigned char cmd[16];

    unsigned char last_modem_status;

    payload_len_c[0] = 0;

    payload_len = sms_compress(text, &jump, pdu);
    
    if (payload_len == 0)
    {
        return;
    }

    modem_no_to_str(no, pdu_no);
    no_len = strlen(pdu_no) / 2;

    sms_itoh(payload_len, &payload_len_c[0]);
    len = 8 + no_len + payload_len - jump;

    /*  */
    last_modem_status = modem_status.status;
    modem_status.status = MS_SMSING;
    
    sprintf(cmd, "AT+CMGS=%d", len);
    modem_sends(cmd);

    /*  */
    _delay_ms(15);              /* wait prompt, TODO use sm later */
    /*  */

    // modem_sends_np("0011000BA18168784017F60000AA");
    modem_sends_np("0011000BA1");
    modem_sends_np(pdu_no);
    modem_sends_np("0000AA");

    modem_sends_np(payload_len_c);

    modem_sends_np(pdu);

    modem_sendc(26);

    modem_status.status = last_modem_status;
}

void modem_str_to_no(const unsigned char *no, unsigned char *ret)
{
    unsigned char i = 0;
    unsigned char len = strlen(no);

    for (i=0; i<len; i+=2)
    {
        ret[i] = no[i+1];

        if (no[i] != 'F')
        {
            ret[i+1] = no[i];
        }
        else
        {
            ret[i+1] = 0;
            break;
        }
    }
}


void modem_no_to_str(const unsigned char *no, unsigned char *ret)
{
    ret[0] = 0;
    unsigned char len = strlen(no);
    unsigned char i;
    unsigned char padding = len % 2;

    for (i=0; i<len-padding; i+=2)
    {
        ret[i] = no[i+1];
        ret[i+1] = no[i];
    }

    if (padding)
    {
        ret[i] = 'F';
        ret[i+1] = no[len-1];
        ret[i+2] = 0;
    }
}
