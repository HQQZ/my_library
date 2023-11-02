/**
 * @file ymodem.c
 * @author h
 * @brief
 * @version 0.1
 * @date 2023-10-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "ymodem.h"
#include <string.h>

// static uint8_t ymodem_rx_buffer[PACKET_HEADER_SIZE+PACKET_1K_SIZE+PACKET_TRAILER_SIZE];
static ym_rx_sta_e ymodem_rx_sta = YM_RX_IDLE;
static char rx_file_name[FILE_NAME_LENGTH];
static char rx_file_size[FILE_SIZE_LENGTH];


/**
 * @brief crc16
 *
 * @param buf
 * @param count
 * @return uint16_t
 */
static uint16_t crc16(const uint8_t *buf, uint32_t count)
{
    uint16_t crc = 0;
    int i;

    while(count--) {
    crc = crc ^ *buf++ << 8;

    for (i=0; i<8; i++) {
        if (crc & 0x8000) {
            crc = crc << 1 ^ 0x1021;
        } else {
            crc = crc << 1;
        }
    }
    }
    return crc;
}

/**
 * @brief 数字转字符
 *
 * @param val
 * @return const char*
 */
static const char *num_to_str(uint32_t val)
{
    /* Maximum number of decimal digits in uint32_t is 10 */
    static char num_str[FILE_NAME_LENGTH];
    int  pos = FILE_NAME_LENGTH - 1;
    num_str[FILE_NAME_LENGTH - 1] = 0;

    if (val == 0) {
    /* If already zero then just return zero */
        return "0";
    }

    while ((val != 0) && (pos > 0)) {
        num_str[--pos] = (val % 10) + '0';
        val /= 10;
    }

    return &num_str[pos];
}

/**
 * @brief 字符转数字
 *
 * @param str
 * @return uint32_t
 */
static uint32_t str_to_num(char* str)
{
    const char *s = str;
    uint32_t acc;
    int c;

    /* strip leading spaces if any */
    do {
        c = *s++;
    } while (c == ' ');

    for (acc = 0; (c >= '0') && (c <= '9'); c = *s++) {
        c -= '0';
        acc *= 10;
        acc += c;
    }
    return acc;
}


/**
 * @brief 解析并返回包命令
 *
 * @param buf
 * @param sz
 * @return uint8_t
 */
uint8_t ymodem_rx_pac_check(uint8_t* buf, uint16_t sz)
{
    uint8_t ch;
    ch = buf[0];
    if(sz < 128) //是个指令包
    {
        if(ch==EOT || ch==ACK || ch==NAK || ch==CAN || ch==CNC)
            return ch;
        else
            return 0xff;      //错误的指令码
    }
    else
    {
        if(ch==SOH || ch==STX)
        {
            uint16_t crc1 = crc16((uint8_t*)(buf+PACKET_HEADER_SIZE), sz-PACKET_OVERHEAD_SIZE);
            uint16_t crc2 = ((uint16_t)(buf[sz-2]))*256+buf[sz-1];
            if(crc1 == crc2 && (0xff == (uint8_t)buf[1]+(uint8_t)buf[2]))
                return ch;
            else
                return 0xff;      //数据包校验为错
        }
        else
            return 0xff;      //错误的指令码
    }
}

/**
 * @brief 尾包校验
 *
 * @param buf
 * @param sz
 * @return uint8_t
 */
uint8_t ymodem_rx_tail_check(uint8_t* buf, uint16_t sz)
{
    uint8_t ch;
    ch = buf[0];
	if(ch == SOH && (0xff == (uint8_t)buf[1]+(uint8_t)buf[2]))
		return ch;
	else
		return 0xff;
}

/**
 * @brief 解析出头包中的文件名和大小
 *
 * @param buf
 * @param sz
 * @return uint8_t
 */
uint8_t ymodem_rx_prepare(char *buf, uint16_t sz)
{
    uint8_t ans = 1;
    char *fil_nm;
    uint8_t fil_nm_len;
    uint16_t fil_sz;

    fil_nm = buf;
    fil_nm_len = strlen(fil_nm);
    fil_sz = (uint16_t)str_to_num(buf+fil_nm_len+1);
    ans = ymodem_drive.ymodem_rx_header_callback(fil_nm, fil_sz);
    return ans;
}

void ymodem_rx_task(void)
{
    uint8_t ans[2];
    uint16_t file_size = 0;

    switch(ymodem_rx_sta)
    {
        case YM_RX_IDLE:
            if(get_ymodem_rx_enable_status())
                ymodem_rx_sta = YM_RX_HANDLE;
            break;

        case YM_RX_HANDLE:
            ans[0] = CNC;
            ymodem_drive.ymodem_send_data(ans, 1);
            ymodem_drive.ymodem_wait(500);
            if(get_ymodem_receive_end_status())
            {
                clear_ymodem_receive_end_flag();
                ans[0] = ymodem_rx_pac_check(get_ymodem_rx_buff(), get_ymodem_rx_len());
                if(ans[0] == SOH)
                {
                    ans[0] = ACK;
                    ans[1] = CNC;
                    ymodem_rx_prepare((char *)get_ymodem_rx_buff(), get_ymodem_rx_len());
                    ymodem_rx_sta = YM_RX_DATA;
                    ymodem_drive.ymodem_send_data(ans, 2);
                }
                else
                {
                    ans[0] = ABORT1;
                    ymodem_rx_sta = YM_RX_IDLE;
                    clear_ymodem_rx_enable_flag();
                    clear_ymodem_receive_end_flag();
                    ymodem_drive.ymodem_send_data(ans, 1);
                }
            }
            break;

        case YM_RX_DATA:
            if(get_ymodem_receive_end_status())
            {
                clear_ymodem_receive_end_flag();
                ans[0] = ymodem_rx_pac_check(get_ymodem_rx_buff(), get_ymodem_rx_len());
                switch(ans[0])
                {
                    case SOH:
                    case STX:
                        ans[0] = ACK;
                        ymodem_drive.ymodem_packet_callback(get_ymodem_rx_buff(), get_ymodem_rx_len());
                        break;

                    case EOT:
                        ans[0] = NAK;
                        ymodem_rx_sta = YM_RX_END1;
                        break;

                    default:
                        ans[0] = ABORT1;
                        clear_ymodem_rx_enable_flag();
                        clear_ymodem_receive_end_flag();
                        ymodem_rx_sta = YM_RX_IDLE;
                        break;
                }
                ymodem_drive.ymodem_send_data(ans, 1);
            }
            break;

        case YM_RX_END1:
            if(get_ymodem_receive_end_status())
            {
                clear_ymodem_receive_end_flag();
                ans[0] = ymodem_rx_pac_check(get_ymodem_rx_buff(), get_ymodem_rx_len());
                if(ans[0] == EOT)
				{
					ans[0] = ACK;
					ans[0] = CNC;
					ymodem_rx_sta = YM_RX_END2;
					ymodem_drive.ymodem_send_data(ans, 2);
				}
                else
				{
					ans[0] = ABORT1;
					ymodem_drive.ymodem_send_data(ans, 1);
					clear_ymodem_rx_enable_flag();
					clear_ymodem_receive_end_flag();
					ymodem_rx_sta = YM_RX_IDLE;
				}
                
            }
            break;
			
		case YM_RX_END2:
			if(get_ymodem_receive_end_status())
            {
				clear_ymodem_receive_end_flag();
				ans[0] = ymodem_rx_tail_check(get_ymodem_rx_buff(), get_ymodem_rx_len());
				if(ans[0] == SOH)
					ans[0] = ACK;
                else
					ans[0] = ABORT1;
				clear_ymodem_rx_enable_flag();
				clear_ymodem_receive_end_flag();
				ymodem_rx_sta = YM_RX_IDLE;
				ymodem_drive.ymodem_send_data(ans, 1);
			}
			break;
    }
}