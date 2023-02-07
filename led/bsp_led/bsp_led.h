/**
 ******************************************************************************
 * @file    bsp_led.h
 * @brief   led driver
 * @author  h
 * @version 1.0
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include "main.h"
#include "gpio.h"

#define LED_NUM    2

#define LED_OFF(gpiox, gpio_pin) {gpiox->BSRR = gpio_pin;}
#define LED_ON(gpiox, gpio_pin) {gpiox->BSRR = (uint32_t)gpio_pin << 16U;}

typedef struct
{
    GPIO_TypeDef *led_port;
    uint16_t led_pin;
}led_param_t;

void bsp_led_init(void);
void set_led_status(uint8_t led_num, uint8_t led_value);

#endif /* __BSP_LED_H__ */
/************************ (C) COPYRIGHT H *****END OF FILE****/





