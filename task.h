#ifndef _TASK_TIMER_H
#define _TASK_TIMER_H

#define TASK_TIMER_PRECLOCK 1024
#define TASK_TIMER_OCR      54
#define TASK_TICK_FREQ (F_CPU / 1024 / TASK_TIMER_OCR)

enum { MOTOR_A, MOTOR_B };
#define MOTOR_PWM_TOP          3110 // 4146
#define MOTOR_PWM_BASE         18.0 /* accroding to datasheet */
#define MOTOR_MIN_POS_DUTY     0.75 /* according to datasheet */
#define MOTOR_MAX_POS_DUTY     2.25 /* according to datasheet*/
#define MOTOR_POS_STEP_DUTY    ((MOTOR_MAX_POS_DUTY - MOTOR_MIN_POS_DUTY)/100)

#define MINUTE_INTERVAL  (TASK_TICK_FREQ * 60)

#define KEY_DEBOUNCE_COUNT 16

void task_init(void);
void task_set_pos(unsigned char ch, float percent);
void task_change_irled(unsigned char val);
void task_set_pump(unsigned char sec);

enum{ TASK_IDLE, TASK_WAIT_LORD };


typedef struct
{
    unsigned char status;
    
    unsigned char motor_enabled;
    float motor_a_pos;
    float motor_b_pos;
    unsigned short motor_a_ocr;
    unsigned short motor_b_ocr;

    int piezo0_count;
    int piezo1_count;

    int  pump_count;

    unsigned int callback_delay_count;

    signed char lord_no_len;
    unsigned char lord_no[20];

    unsigned char irled_duty;   /* percent */
    unsigned char irled_duty_count;

    unsigned char csq_led_duty;
    unsigned char csq_led_duty_count;

    float temperature;
    short ldr;
    short bat;
    
} _task_sm;
volatile _task_sm task_sm;

#endif
