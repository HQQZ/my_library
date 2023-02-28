#ifndef __DEV_SPI_FLASH_H__
#define __DEV_SPI_FLASH_H__

#include "mcu_spi.h"
#include "linux_list.h"
#include <stdbool.h>

/*SPI FLASH 信息*/
typedef struct
{
	char *name;
	uint32_t JID;
	uint32_t MID;
	/*容量，扇区，块大小等信息*/
    uint32_t pagenum;//页大小
	uint32_t sectornum;//总扇区
	uint32_t sectorsize;//扇区大小
	uint32_t structure;//总容量

}_strSpiFlash;


/*SPI FLASH设备定义*/
typedef struct
{
	char *name;//设备名称
	char *spich;//挂载在哪条SPI通道
	_strSpiFlash *pra;//设备信息
}DevSpiFlash;

typedef struct
{
	/**/
	int32_t gd;
	/*设备信息*/

	DevSpiFlash dev;

	/*spi 通道节点*/
	DevSpiChNode *spichnode;

	struct list_head list;
}DevSpiFlashNode;

bool dev_spiflash_read(DevSpiFlashNode *node, uint32_t addr, uint16_t rlen, uint8_t *dst);
bool dev_spiflash_write(DevSpiFlashNode *node,  uint32_t addr, uint16_t wlen, uint8_t* src);
bool dev_spiflash_page_write(DevSpiFlashNode *node, uint32_t addr, uint16_t wlen, uint8_t *pbuf);
bool dev_spiflash_sector_erase(DevSpiFlashNode *node, uint32_t sector);
bool dev_spiflash_sector_read(DevSpiFlashNode *node, uint32_t sector, uint8_t *dst);
bool dev_spiflash_sector_write(DevSpiFlashNode *node, uint32_t sector, uint8_t *src);
DevSpiFlashNode *dev_spiflash_open(char* name);
bool dev_spiflash_close(DevSpiFlashNode *node);
bool dev_spiflash_register(const DevSpiFlash *dev);

void flash_test(void);

#endif /* __DEV_SPI_FLASH_H__ */

