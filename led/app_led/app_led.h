/**
 ******************************************************************************
 * @file    app_led.h
 * @brief   user led application
 * @author  h
 * @version 1.0
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#ifndef __APP_LED_H__
#define __APP_LED_H__

#include "main.h"
#include "lib/list.h"
#include "bsp_led/bsp_led.h"

typedef enum
{
    LED_OFF_LONG = 0,
    LED_ON_LONG,
    LED_SLOW_BLINK,
    LED_QUICK_BLINK,
    LED_SINGLE_BLINK,
    LED_DOUBLE_BLINK
}LED_STATUS_ENUM;

typedef union
{
    uint64_t ram[2];
    struct
    {
        uint64_t cycle : 8;          //led on/off半周期
        uint64_t cycle_conunt : 8;   //led周期计数
        uint64_t blink_sum : 8;      //闪烁次数
        uint64_t blink_conunt : 8;   //闪烁计数
        uint64_t dark_sum : 8;       //熄灯时间
        uint64_t dark_conunt : 8;    //熄灯计数
        uint64_t led_n : 8;          //第n个led
        uint64_t led_toggle : 1;     //led反显
        uint64_t current_status : 7; //当前led状态
    }para;
}led_ram_u;

void app_led_init(void);
void led_set_always_on(uint8_t led);
void led_set_always_off(uint8_t led);
void led_slow_blink(uint8_t led);
void led_quick_blink(uint8_t led);
void led_single_blink(uint8_t led);
void led_double_blink(uint8_t led);
void app_led_task(void);

#endif /* __APP_LED_H__ */
/************************ (C) COPYRIGHT H *****END OF FILE****/
