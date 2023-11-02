/**
 * @file ymodem_port.h
 * @author h
 * @brief
 * @version 0.1
 * @date 2023-10-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __YMODEM_PORT_H__
#define __YMODEM_PORT_H__

#include <stdio.h>
#include "main.h"



typedef struct
{
    void (*ymodem_send_data)(uint8_t *buf, uint16_t length);//发送数据函数
    void (*ymodem_receive_data)(uint8_t *buf, uint16_t length);//接收数据函数
    uint8_t (*ymodem_wait)(uint16_t wait_time);//等待函数

    uint8_t (*ymodem_rx_header_callback)(char *file_name, uint16_t file_size);//首包接收成功后对文件名和大小的处理，成功返回1，失败0
    uint8_t (*ymodem_packet_callback)(uint8_t *buf, uint16_t size);//数据包校验成功后的处理函数
	uint8_t (*ymodem_end_callback)(void);
}Ymodem_drive_t;

void set_ymodem_rx_enable_flag(void);
void clear_ymodem_rx_enable_flag(void);
uint8_t get_ymodem_rx_enable_status(void);
void set_ymodem_receive_end_flag(void);
void clear_ymodem_receive_end_flag(void);
uint8_t get_ymodem_receive_end_status(void);
uint8_t *get_ymodem_rx_buff(void);
uint16_t get_ymodem_rx_len(void);
uint8_t register_rx_ymodem(uint8_t *rx_buf, uint16_t *rx_len);

extern Ymodem_drive_t ymodem_drive;


#endif /* __YMODEM_PORT_H__ */

