#ifndef __MCU_SPI_H__
#define __MCU_SPI_H__

#include "spi.h"
#include "linux_list.h"
#include <stdlib.h>
#include <stdbool.h>

#define DEV_NAME_SIZE 16

#define MALLOC malloc
#define FREE   free

#define mcu_io_setbit(mcu_port, mcu_pin) \
        mcu_port->BSRR = mcu_pin;

#define mcu_io_resetbit(mcu_port, mcu_pin) \
        mcu_port->BSRR = (uint32_t)mcu_pin<<16U;

#define mcu_io_readbit(mcu_port, mcu_pin) \
        ((mcu_port->ODR & mcu_pin) ? 1U : 0U);

typedef enum{
	DEV_SPI_H = 1,//硬件SPI控制器
	DEV_SPI_V = 2,//IO模拟SPI
}DEV_SPI_TYPE;

/*

SPI模式

*/
typedef enum{
	SPI_MODE_0 =0,
	SPI_MODE_1,
	SPI_MODE_2,
	SPI_MODE_3,
	SPI_MODE_MAX
}SPI_MODE;

/*
    IO定义
*/
typedef struct
{
    GPIO_TypeDef * gpio_port;
    uint16_t gpio_pin;
}SPI_IO;

/*
	SPI 分两层，
	1层是SPI控制器，不包含CS
	2层是SPI通道，由控制器+CS组成

*/
/*

	SPI 设备定义

*/
typedef struct
{
	/*设备名称*/
	char name[DEV_NAME_SIZE];

	/*设备类型，IO模拟 or 硬件控制器*/
	DEV_SPI_TYPE type;

	SPI_IO clk;

	SPI_IO mosi;

	SPI_IO miso;

}DevSpi;

/*

	SPI控制器设备节点

*/
typedef struct
{
	/*句柄，空闲为-1，打开为0，spi控制器不能重复打开*/
	int32_t gd;
	/*控制器硬件信息，初始化控制器时拷贝设备树的信息到此*/
	DevSpi dev;

	/*模拟SPI的时钟分频设置*/
	uint16_t clk;
	/*链表*/
	struct list_head list;
}DevSpiNode;

/*
	SPI 通道定义
	一个SPI通道，有一个SPI控制器+一根CS引脚组成

*/
typedef struct
{
	/*通道名称，相当于设备名称*/
	char name[DEV_NAME_SIZE];
	/*SPI控制器名称*/
	char spi[DEV_NAME_SIZE];

	/*cs脚*/
	SPI_IO cs;
}DevSpiCh;

/*SPI通道节点*/
typedef struct
{
	/**/
	int32_t gd;

	DevSpiCh dev;

	DevSpiNode *spi;//控制器节点指针

	struct list_head list;
}DevSpiChNode;

extern bool mcu_spi_register(const DevSpi *dev);
extern bool mcu_spich_register(const DevSpiCh *dev);

extern DevSpiChNode *mcu_spi_open(char *name, SPI_MODE mode, uint16_t pre);
extern bool mcu_spi_close(DevSpiChNode * node);
extern bool mcu_spi_transfer(DevSpiChNode * node, uint8_t *snd, uint8_t *rsv, uint32_t len);
extern bool mcu_spi_cs(DevSpiChNode * node, uint8_t sta);
void spi_test(void);

#endif /* __MCU_SPI_H__ */
