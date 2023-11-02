/**
 * @file ymodem_port.c
 * @author h
 * @brief
 * @version 0.1
 * @date 2023-10-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "ymodem_port.h"
#include "usart.h"

Ymodem_drive_t ymodem_drive;

static uint8_t *ymodem_rx_buff = NULL;
static uint16_t *ymodem_rx_len = NULL;
static uint8_t ymodem_rx_enable_flag = 0;
static uint8_t ymodem_receive_end_flag = 0;


static void _ymodem_send_data(uint8_t *buf, uint16_t length)
{
	DMA_UART_Send(buf, length);
}

static void _ymodem_receive_data(uint8_t *buf, uint16_t length)
{
	DMA_UART_Receive(buf, length);
}

static uint8_t _ymodem_wait(uint16_t wait_time)
{
	HAL_Delay(wait_time);
    return 1;
}

static uint8_t _ymodem_rx_header_callback(char *file_name, uint16_t file_size)
{
    return 1;
}

static uint8_t _ymodem_packet_callback(uint8_t *buf, uint16_t size)
{
    return 1;
}

static uint8_t _ymodem_end_callback(void)
{
	clear_ymodem_rx_enable_flag();
	clear_ymodem_receive_end_flag();
	return 1;
}

void set_ymodem_rx_enable_flag(void)
{
    ymodem_rx_enable_flag = 1;
}

void clear_ymodem_rx_enable_flag(void)
{
    ymodem_rx_enable_flag = 0;
}

uint8_t get_ymodem_rx_enable_status(void)
{
    return ymodem_rx_enable_flag;
}

void set_ymodem_receive_end_flag(void)
{
    ymodem_receive_end_flag = 1;
}

void clear_ymodem_receive_end_flag(void)
{
    ymodem_receive_end_flag = 0;
}

uint8_t get_ymodem_receive_end_status(void)
{
    return ymodem_receive_end_flag;
}

uint8_t *get_ymodem_rx_buff(void)
{
	return ymodem_rx_buff;
}

uint16_t get_ymodem_rx_len(void)
{
	return (*ymodem_rx_len);
}

uint8_t register_rx_ymodem(uint8_t *rx_buf, uint16_t *rx_len)
{
	ymodem_rx_buff = rx_buf;
	ymodem_rx_len = rx_len;
	
	ymodem_drive.ymodem_send_data = _ymodem_send_data;
    ymodem_drive.ymodem_receive_data = _ymodem_receive_data;
    ymodem_drive.ymodem_wait = _ymodem_wait;

    ymodem_drive.ymodem_rx_header_callback = _ymodem_rx_header_callback;
    ymodem_drive.ymodem_packet_callback = _ymodem_packet_callback;
	ymodem_drive.ymodem_end_callback = _ymodem_end_callback;
}
