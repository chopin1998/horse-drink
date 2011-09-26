#ifndef _SMS_H
#define _SMS_H 1

#define SMS_ENCODED_SIZE  70                          
#define SMS_TEXT_LENGTH   80


void sms_get(unsigned char index);
void sms_process(void);
void sms_send(unsigned char *no, unsigned char *buf);

int sms_compress( unsigned char *in, unsigned char *discarded, unsigned char ret[] );
int sms_decompress( unsigned char *compressed, unsigned char *decompressed );

int sms_atoi( unsigned char *a );
int sms_htoi( unsigned char hex );
void sms_itoh( int n, unsigned char *ret );

void modem_str_to_no(const unsigned char *no, unsigned char *ret);
void modem_no_to_str(const unsigned char *no, unsigned char *ret);

#endif
