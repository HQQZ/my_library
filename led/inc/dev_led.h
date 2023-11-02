/**
 * @file dev_led.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-10-31
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __DEV_LED_H__
#define __DEV_LED_H__

#include <stdio.h>
#include "linux_list.h"

typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;

typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;

#define DEV_NAME_SIZE 16

typedef struct {
    uint16_t flash_light_time;
    uint16_t recorde_flash_light_time;
    uint16_t flash_dark_time;
    uint16_t recorde_flash_dark_time;
    uint16_t flash_times;
    uint16_t recorde_flash_times;
    uint16_t dark_time;
    uint16_t recorde_dark_time;
}led_param, *led_param_t;

typedef enum {
    LED_ALWAY_ON = 0,
    LED_ALWAY_OFF,
    LED_CYCLE_FLASH,
    LED_FLICKER,
}led_state;

typedef struct {
    char name[DEV_NAME_SIZE];
    led_state led_sta;
    led_param led_time;
    void (* set_led_state)(int state);
}dev_led;

typedef struct {
    int32_t gd;
    dev_led dev;

    struct list_head list;
}dev_led_node;

struct list_head *get_led_head(void);
dev_led_node *dev_led_open(char *name);
int dev_led_close(char *name);
int dev_led_add(dev_led *dev);

#endif /* __DEV_LED_H__ */

