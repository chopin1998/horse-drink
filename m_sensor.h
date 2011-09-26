#ifndef __M_SENSOR_H
#define __M_SENSOR_H 1

void m_sensor_init(void);

void m_sensor_int_enable(unsigned char ch);
void m_sensor_int_disable(unsigned char ch);
void m_sensor_timer_tick(void);

#define M_0_COUNT_TOP (TASK_TICK_FREQ >> 2) /* one shot int delay */
#define M_0_ADD       6
#define M_0_DEC       2
#define M_0_THRESHOLD 20

#define M_1_COUNT_TOP (TASK_TICK_FREQ >> 1)
#define M_1_ADD       4
#define M_1_DEC       1
#define M_1_THRESHOLD 20


typedef struct
{
    signed char m0_count;
    signed char m1_count;

    int _m0_delay;
    int _m1_delay;

    unsigned char m0_alart : 4;
    unsigned char m1_alart : 4;
    
} _m_sensor;
volatile _m_sensor m_sensor;


#endif
