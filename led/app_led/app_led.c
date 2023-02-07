/**
 ******************************************************************************
 * @file    app_led.c
 * @brief   user led application
 * @author  h
 * @version 1.0
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include "app_led/app_led.h"
#include "string.h"

struct led_list
{
    struct led_list *next;
    led_ram_u led_buf;
};

static struct led_list led_buffer[LED_NUM];
LIST(LedList);

/**
 * @brief This function is used for apl init of led
 * @retval None
 */
void app_led_init(void)
{
    memset(led_buffer, 0, sizeof(led_buffer));
    list_init(LedList);
}

/**
 * @brief 判断led是否存在ledlist中，存在则移除
 * @param led:第n个led
 * @retval None
 */
static void led_list_exist(uint8_t led)
{
    struct led_list *i = NULL;
    for(i = list_head(LedList); i != NULL; i = i->next)
    {
        if(i->led_buf.para.led_n == led)
        {
            list_remove(LedList, i);
        }
    }
}

/**
 * @brief 打开/关闭led
 * @param led :第n个led
 * @param led_sta : led on/off
 * @retval None
 */
static void set_led_on_or_off(uint8_t led, uint8_t led_sta)
{
    led_list_exist(led);

    memset(&led_buffer[led].led_buf,0,sizeof(led_buffer[led].led_buf));
    led_buffer[led].led_buf.para.led_n = led;
    if(led_sta)
    {
        led_buffer[led].led_buf.para.current_status = LED_ON_LONG;
    }
    else
    {
        led_buffer[led].led_buf.para.current_status = LED_OFF_LONG;
    }
    list_add(LedList, &led_buffer[led]);
}

/**
 * @brief led常开
 * @param led :第n个led
 * @retval None
 */
void led_set_always_on(uint8_t led)
{
    set_led_on_or_off(led, 1);
}

/**
 * @brief led常关
 * @param led :第n个led
 * @retval None
 */
void led_set_always_off(uint8_t led)
{
    set_led_on_or_off(led ,0);
}

/**
 * @brief 设置led的闪烁状态
 * @param led :第n个led
 * @param cycle : 闪烁的半周期次数，根据app_led_task()调用周期设置
 * @param blink_sum : 闪烁次数
 * @param dark : 闪烁后的熄灯时间
 * @retval None
 */
static void led_blink_set(uint8_t led, uint8_t cycle, uint8_t blink_sum, uint8_t dark)
{
    led_list_exist(led);

    memset(&led_buffer[led].led_buf,0,sizeof(led_buffer[led].led_buf));
    led_buffer[led].led_buf.para.led_n = led;
    led_buffer[led].led_buf.para.cycle = cycle;
    led_buffer[led].led_buf.para.blink_sum = blink_sum;
    led_buffer[led].led_buf.para.dark_sum = dark;
}

/**
 * @brief 设置led慢闪
 * @param led :第n个led
 * @retval None
 */
void led_slow_blink(uint8_t led)
{
    led_blink_set(led, 100, 0, 0);
    led_buffer[led].led_buf.para.current_status = LED_SLOW_BLINK;

    list_add(LedList, &led_buffer[led]);
}

/**
 * @brief 设置led快闪
 * @param led :第n个led
 * @retval None
 */
void led_quick_blink(uint8_t led)
{
    led_blink_set(led, 10, 0, 0);
    led_buffer[led].led_buf.para.current_status = LED_QUICK_BLINK;

    list_add(LedList, &led_buffer[led]);
}

/**
 * @brief 设置led单闪
 * @param led :第n个led
 * @retval None
 */
void led_single_blink(uint8_t led)
{
    led_blink_set(led, 20, 1, 80);
    led_buffer[led].led_buf.para.current_status = LED_SINGLE_BLINK;

    list_add(LedList, &led_buffer[led]);
}

/**
 * @brief 设置led双闪
 * @param led :第n个led
 * @retval None
 */
void led_double_blink(uint8_t led)
{
    led_blink_set(led, 20, 2, 80);
    led_buffer[led].led_buf.para.current_status = LED_DOUBLE_BLINK;

    list_add(LedList, &led_buffer[led]);
}

/**
 * @brief led apl函数，周期性调用
 * @retval None
 */
void app_led_task(void)
{
    struct led_list *i;
    for(i = list_head(LedList); i != NULL; i = i->next)
    {
        switch (i->led_buf.para.current_status)
        {
        case LED_OFF_LONG:
            set_led_status(i->led_buf.para.led_n, 0);
            break;

        case LED_ON_LONG:
            set_led_status(i->led_buf.para.led_n, 1);
            break;

        case LED_SLOW_BLINK:
        case LED_QUICK_BLINK:
            if(0 == i->led_buf.para.cycle_conunt)
            {
                i->led_buf.para.cycle_conunt++;
                if(i->led_buf.para.led_toggle)
                {
                    i->led_buf.para.led_toggle = 0;
                }
                else
                {
                    i->led_buf.para.led_toggle = 1;
                }
                set_led_status(i->led_buf.para.led_n, i->led_buf.para.led_toggle);
            }
            else if(i->led_buf.para.cycle == i->led_buf.para.cycle_conunt++)
            {
                i->led_buf.para.cycle_conunt = 0;
            }
            break;

        case LED_SINGLE_BLINK:
        case LED_DOUBLE_BLINK:
            if(i->led_buf.para.blink_sum >= i->led_buf.para.blink_conunt)
            {
                if(0 == i->led_buf.para.cycle_conunt)
                {
                    i->led_buf.para.cycle_conunt++;
                    if(i->led_buf.para.led_toggle)
                    {
                        i->led_buf.para.led_toggle = 0;
                    }
                    else
                    {
                        i->led_buf.para.led_toggle = 1;
                    }
                    set_led_status(i->led_buf.para.led_n, i->led_buf.para.led_toggle);
                }
                else if(i->led_buf.para.cycle == i->led_buf.para.cycle_conunt++)
                {
                    i->led_buf.para.cycle_conunt = 0;
                    i->led_buf.para.blink_conunt++;
                }
            }
            else
            {
                if(i->led_buf.para.dark_sum > i->led_buf.para.dark_conunt++)
                {
                    set_led_status(i->led_buf.para.led_n, 0);
                }
                else
                {
                    i->led_buf.para.led_toggle =
                    i->led_buf.para.dark_conunt =
                    i->led_buf.para.blink_conunt = 0;
                }
            }
            break;

        default:
            set_led_status(i->led_buf.para.led_n, 0);
            break;
        }

    }
}

/************************ (C) COPYRIGHT H *****END OF FILE****/
