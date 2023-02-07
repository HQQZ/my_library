/**
 ******************************************************************************
 * @file    bsp_led.c
 * @brief   led driver
 * @author  h
 * @version 1.0
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include "bsp_led/bsp_led.h"

static uint8_t inited = 0;
static led_param_t led_param[LED_NUM] = {
    {LED0_GPIO_Port, LED0_Pin},
    {LED1_GPIO_Port, LED1_Pin},
};

/**
 * @brief  led init
 * @retval none
 */
void bsp_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOF, LED0_Pin | LED1_Pin, GPIO_PIN_SET);

    /*Configure GPIO pins : PFPin PFPin */
    GPIO_InitStruct.Pin = LED0_Pin | LED1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    inited = 1;
}

/**
 * @brief set led status
 * @param led_num : number of led
 * @param led_value : 0:led off, 1:led_on
 * @retval none
 */
void set_led_status(uint8_t led_num, uint8_t led_value)
{
    if(!inited)
        return;

    if(led_value)
    {
        LED_ON(led_param[led_num].led_port, led_param[led_num].led_pin);
    }
    else
    {
        LED_OFF(led_param[led_num].led_port, led_param[led_num].led_pin);
    }
}
/************************ (C) COPYRIGHT H *****END OF FILE****/

