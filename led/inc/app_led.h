/**
 * @file app_led.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-11-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __APP_LED_H__
#define __APP_LED_H__

#include "dev_led.h"

typedef enum {
    led_off = 0,
    led_on,
}app_led_state_e;

void app_led_init(void);
int app_led_set(dev_led_node *node, led_state led_sta, led_param led_time);
void app_led_thread(void);

#endif /* __APP_LED_H__ */
