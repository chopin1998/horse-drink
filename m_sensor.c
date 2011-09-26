#include "common.h"
#include "m_sensor.h"
#include "device.h"
#include "task.h"


ISR (INT0_vect)
{   
    task_sm.piezo0_count = 10;    
    
    m_sensor_int_disable(0);

    m_sensor.m0_count += M_0_ADD;
    if (m_sensor.m0_count > M_0_THRESHOLD)
    {
        m_sensor.m0_count = M_0_THRESHOLD;
        m_sensor.m0_alart = 1;
    }
}

ISR (INT1_vect)
{
    // task_sm.piezo0_count = 10;

    m_sensor_int_disable(1);

    m_sensor.m1_count += M_1_ADD;
    if (m_sensor.m1_count > M_1_THRESHOLD)
    {
        m_sensor.m1_count = M_1_THRESHOLD;
        m_sensor.m1_alart = 1;
    }
}


void m_sensor_init(void)
{
    MCUCR |= _BV(ISC10) | _BV(ISC00); /* INT0/ INT1 for logical change */
    m_sensor_int_disable(0);
    m_sensor_int_disable(1);

    GIFR |= _BV(INT0) | _BV(INT1); /* clear flag */

    m_sensor.m0_count = 0;
    m_sensor.m1_count = 0;

    m_sensor.m0_alart = 0;
    m_sensor.m1_alart = 0;
}


void m_sensor_int_enable(unsigned char ch)
{
    /* enable INT0/ INT1 */
    
    if (ch == 0)
    {
        m_sensor._m0_delay = M_0_COUNT_TOP;
        GIFR |= _BV(INT0);
        GICR |= _BV(INT0);
    }
    else if (ch == 1)
    {
        m_sensor._m1_delay = M_1_COUNT_TOP;
        GIFR |= _BV(INT1);
        GICR |= _BV(INT1);
    }
}


void m_sensor_int_disable(unsigned char ch)
{
    if (ch == 0)
    {
        GICR &= ~( _BV(INT0) );
        GIFR |= _BV(INT0);
        m_sensor._m0_delay = M_0_COUNT_TOP;
    }
    else if (ch == 1)
    {
        GICR &= ~( _BV(INT1) );
        GIFR |= _BV(INT1);
        m_sensor._m1_delay = M_1_COUNT_TOP;
    }
}


inline void m_sensor_timer_tick(void)
{
    if (m_sensor._m0_delay > 0)
    {
        m_sensor._m0_delay--;

        if (m_sensor._m0_delay == 0)
        {
            m_sensor_int_enable(0);

            m_sensor.m0_count -= M_0_DEC;
            if (m_sensor.m0_count <= 0)
            {
                m_sensor.m0_count = 0;
                m_sensor.m0_alart = 0;
            }
        }
    }

    if (m_sensor._m1_delay > 0)
    {
        m_sensor._m1_delay--;

        if (m_sensor._m1_delay == 0)
        {
            m_sensor_int_enable(1);

            m_sensor.m1_count -= M_1_DEC;
            if (m_sensor.m1_count <= 0)
            {
                m_sensor.m1_count = 0;
                m_sensor.m1_alart = 0;
            }
        }
    }
}
