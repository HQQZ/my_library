/**
 * @file app_led.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "app_led.h"


#define APP_LED_CYCLE   5 /* cycle ms */
#define MAX_LED_SUM     2


/* temp */
dev_led led_buff[MAX_LED_SUM] = {
    {
        "led0",
		LED_ALWAY_OFF,
		{
			.flash_light_time = 200/APP_LED_CYCLE,
            .flash_dark_time = 200/APP_LED_CYCLE,
            .flash_times = 1,
            .dark_time = 800/APP_LED_CYCLE,
		},
    },
};

/**
 * @brief init app led
 *
 */
void app_led_init(void)
{
    memset(led_buff, 0, sizeof(led_buff));
    for(int i = 0; i < sizeof(led_buff)/sizeof(dev_led); i++)
    {
        dev_led_add(&led_buff[i]);
    }
}

/**
 * @brief set led sta and cycle
 *
 * @param node  led node(get using the dev_led_open() fun)
 * @param led_sta   led state
 * @param led_time  led cycle time
 *                  (取决于APP_LED_CYCLE，比如app_led_thread()5ms调用一次，APP_LED_CYCLE == 5
 *                  根据需要的led亮灭时间，除以APP_LED_CYCLE)
 * @return int   0:ok   -1:faild
 */
int app_led_set(dev_led_node *node, led_state led_sta, led_param led_time)
{
    if(node->gd == 0)
        return -1;

    node->gd = 0;
    node->dev.led_sta = led_sta;
    memset((uint8_t *)&node->dev.led_time, 0, sizeof(node->dev.led_time));
    node->dev.led_time = led_time;
    node->gd = -1;

    return 0;
}

/**
 * @brief   app led thred(周期性调用)
 *
 */
void app_led_thread(void)
{
    dev_led_node *node, *pos;

    list_for_each_entry(pos, get_led_head(), list)
    {
        if(pos->gd == -1)
        {
            switch(pos->dev.led_sta)
            {
                case LED_ALWAY_ON:
                    pos->dev.set_led_state(led_on);
                    break;

                case LED_ALWAY_OFF:
                    pos->dev.set_led_state(led_off);
                    break;

                case LED_CYCLE_FLASH:
                    if(pos->dev.led_time.recorde_flash_light_time++ < pos->dev.led_time.flash_light_time)
                    {
                        pos->dev.set_led_state(led_on);
                        pos->dev.led_time.recorde_flash_dark_time = 0;
                    }
                    else
                    {
                        if(pos->dev.led_time.recorde_flash_dark_time++ < pos->dev.led_time.flash_dark_time)
                        {
                            pos->dev.set_led_state(led_off);
                        }
                        else
                        {
                            pos->dev.led_time.recorde_flash_light_time = 0;
                        }
                    }
                    break;

                case LED_FLICKER:
                    if(pos->dev.led_time.recorde_flash_times <= pos->dev.led_time.flash_times)
                    {
                        if(pos->dev.led_time.recorde_flash_light_time++ < pos->dev.led_time.flash_light_time)
                        {
                            pos->dev.set_led_state(led_on);
                            pos->dev.led_time.recorde_flash_dark_time = 0;
                        }
                        else
                        {
                            if(pos->dev.led_time.recorde_flash_dark_time++ < pos->dev.led_time.flash_dark_time - 1)
                            {
                                pos->dev.set_led_state(led_off);
                            }
                            else
                            {
                                pos->dev.led_time.recorde_flash_light_time = 0;
                                pos->dev.led_time.recorde_flash_times++;
                            }
                        }
                    }
                    else
                    {
                        if(pos->dev.led_time.recorde_dark_time++ < pos->dev.led_time.dark_time - 1)
                        {
                            pos->dev.set_led_state(led_off);
                        }
                        else
                        {
                            pos->dev.led_time.recorde_dark_time = 0;
                            pos->dev.led_time.recorde_flash_times = 0;
                        }
                    }
                    break;
            }
        }
    }
}