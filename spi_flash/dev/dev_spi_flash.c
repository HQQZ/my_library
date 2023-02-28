/**
 * @file dev_spi_flash.c
 * @author h
 * @brief  spi flash驱动
 * @version 0.1
 * @date 2023-02-21
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "dev_spi_flash.h"
#include <string.h>

/*
	常用的SPI FLASH 参数信息
*/
_strSpiFlash SpiFlashPraList[]=
{
	{"MX25L3206E", 0xC22016, 0xC215, 256, 1024, 4096, 4194304},
	{"W25Q64JVSI", 0xef4017, 0xef16, 256, 2048, 4096, 8388608},
    {"W25Q128JVEI", 0xef4018, 0xef17, 256, 4096, 4096, 16777216},
    {"NM25Q128", 0x522118, 0x1752, 256, 4096, 4096, 16777216},
};

/* spi flash 命令*/
#define SPIFLASH_WRITE      0x02  /* Write to Memory instruction  Page Program */
#define SPIFLASH_WRSR       0x01  /* Write Status Register instruction */
#define SPIFLASH_WREN       0x06  /* Write enable instruction */
#define SPIFLASH_WRDIS      0x04  /* Write disable instruction */
#define SPIFLASH_READ       0x03  /* Read from Memory instruction */
#define SPIFLASH_FREAD      0x0B  /* Fast Read from Memory instruction */
#define SPIFLASH_RDSR       0x05  /* Read Status Register instruction  */
#define SPIFLASH_SE         0x20  /* Sector Erase instruction */
#define SPIFLASH_BE         0xD8  /* Bulk Erase instruction */
#define SPIFLASH_CE         0xC7  /* Chip Erase instruction */
#define SPIFLASH_PD         0xB9  /* Power down enable */
#define SPIFLASH_RPD        0xAB  /* Release from Power down mode */
#define SPIFLASH_RDMID      0x90  /* Read Device identification */
#define SPIFLASH_RDJID      0x9F  /* Read JEDEC identification */
#define SPIFLASH_DUMMY_BYTE 0xA5

#define FLASH_TIMEOUT       0x40000

LIST_HEAD(DevSpiFlashRoot);

/**
 * @brief FLASH写使能
 *
 * @param[in] node Flash设备节点
 */
static void dev_spiflash_writeen(DevSpiFlashNode *node)
{
    uint8_t command = SPIFLASH_WREN;
    mcu_spi_cs(node->spichnode, 0);
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);
    mcu_spi_cs(node->spichnode, 1);
}

/**
 * @brief 等待Flash写结束
 *
 * @param node Flash设备节点
 * @return bool 成功:true, 失败:false
 */
static bool dev_spiflash_wait_write_end(DevSpiFlashNode *node)
{
    uint8_t flash_status = 0;
    uint8_t command = SPIFLASH_RDSR;
    uint32_t time_out = 0;

	mcu_spi_cs(node->spichnode, 0);
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);
    do
    {
        mcu_spi_transfer(node->spichnode, NULL, &flash_status, 1);
        if(time_out++ > FLASH_TIMEOUT)
        {
            return false;
        }
    } while ((flash_status & 0x01) == 1);
	mcu_spi_cs(node->spichnode, 1);

    return true;
}

/**
 * @brief 按照字节地址擦除扇区
 *
 * @param node Flash设备节点
 * @param addr 要擦除的扇区地址
 * @return true 擦除成功
 * @return false 擦除失败
 */
bool dev_spiflash_erase(DevSpiFlashNode *node, uint32_t addr)
{
    uint8_t command[4];

    command[0] = SPIFLASH_SE;
    command[1] = (uint8_t)(addr>>16);
    command[2] = (uint8_t)(addr>>8);
    command[3] = (uint8_t)(addr);

    dev_spiflash_writeen(node);
    if(dev_spiflash_wait_write_end(node) != true)
        return false;

	mcu_spi_cs(node->spichnode, 0);
    mcu_spi_transfer(node->spichnode, command, NULL, 4);
	mcu_spi_cs(node->spichnode, 1);

    if(dev_spiflash_wait_write_end(node) != true)
        return false;

    return true;
}

bool dev_spiflash_erase_fullchip(DevSpiFlashNode *node)
{
    uint8_t command = SPIFLASH_CE;

    dev_spiflash_writeen(node);
    if(dev_spiflash_wait_write_end(node) != true)
        return false;

	mcu_spi_cs(node->spichnode, 0);
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);
	mcu_spi_cs(node->spichnode, 1);

    if(dev_spiflash_wait_write_end(node) != true)
        return false;

    return true;
}

bool dev_spiflash_read(DevSpiFlashNode *node, uint32_t addr, uint16_t rlen, uint8_t *dst)
{
    uint8_t command[4];

    if(rlen == 0) return false;

    command[0] = SPIFLASH_READ;
    command[1] = (uint8_t)(addr>>16);
    command[2] = (uint8_t)(addr>>8);
    command[3] = (uint8_t)(addr);

	mcu_spi_cs(node->spichnode, 0);
    mcu_spi_transfer(node->spichnode, command, NULL, 4);
    mcu_spi_transfer(node->spichnode, NULL, dst, rlen);
	mcu_spi_cs(node->spichnode, 1);
    return true;
}

bool dev_spiflash_page_write(DevSpiFlashNode *node, uint32_t addr, uint16_t wlen, uint8_t *pbuf)
{
    uint8_t command[4];
    command[0] = SPIFLASH_WRITE;
    command[1] = (uint8_t)(addr>>16);
    command[2] = (uint8_t)(addr>>8);
    command[3] = (uint8_t)(addr);

    if(wlen >= node->dev.pra->structure)
    {
        return false;
    }

    dev_spiflash_writeen(node);
    if(dev_spiflash_wait_write_end(node) != true)
        return false;

    mcu_spi_cs(node->spichnode, 0);
    if(mcu_spi_transfer(node->spichnode, command, NULL, 4) != true)
        return false;
    if(mcu_spi_transfer(node->spichnode, pbuf, NULL, wlen) != true)
        return false;
    mcu_spi_cs(node->spichnode, 1);

    if(dev_spiflash_wait_write_end(node) != true)
        return false;

    return true;
}

bool dev_spiflash_write(DevSpiFlashNode *node, uint32_t addr, uint16_t wlen, uint8_t* src)
{

    uint8_t num_of_page = 0, num_of_single = 0, waddr = 0, count = 0, temp = 0;

    // 写入地址是否对齐页地址
    waddr = addr % node->dev.pra->pagenum;
    // 差多少字节对齐页地址
    count = node->dev.pra->pagenum - waddr;
    // 需要写入的整数页
    num_of_page = wlen / node->dev.pra->sectorsize;
    // 剩余不满一页字节数
    num_of_single = wlen % node->dev.pra->sectorsize;

    // 页地址对齐
    if(waddr == 0)
    {
        // 写入字节小于页大小
        if(num_of_page == 0)
        {
            if(dev_spiflash_page_write(node, addr, wlen, src) != true)
                return false;
        }
        else
        {
            // 写整数页
            while(num_of_page--)
            {
                if(dev_spiflash_page_write(node, addr, node->dev.pra->pagenum, src) != true)
                    return false;
                addr += node->dev.pra->pagenum;
                src += node->dev.pra->pagenum;
            }
            // 写剩余字节
            if(dev_spiflash_page_write(node, addr, num_of_single, src) != true)
                return false;
        }
    }
    else
    {
        // 写入字节不足一页
        if(num_of_page == 0)
        {
            // 当前页剩余空间不足以写下single字节
            if(num_of_single > count)
            {
                temp = num_of_single - count;
                // 先写满当前页
                if(dev_spiflash_page_write(node, addr, count, src) != true)
                    return false;
                addr += count;
                src += count;
                // 写剩余字节
                if(dev_spiflash_page_write(node, addr, temp, src) != true)
                    return false;
            }
            else
            {
                if(dev_spiflash_page_write(node, addr, wlen, src) != true)
                    return false;
            }
        }
        else
        {
            // 先写未对齐字节
            wlen -= count;
            num_of_page = wlen / node->dev.pra->sectorsize;
            num_of_single = wlen % node->dev.pra->sectorsize;

            if(dev_spiflash_page_write(node, addr, count, src) != true)
                return false;
            addr += count;
            src += count;

            while(num_of_page--)
            {
                if(dev_spiflash_page_write(node, addr, node->dev.pra->pagenum, src) != true)
                    return false;
                addr += node->dev.pra->pagenum;
                src += node->dev.pra->pagenum;
            }
            if(num_of_single != 0)
            {
                if(dev_spiflash_page_write(node, addr, num_of_single, src) != true)
                    return false;
            }
        }
    }

	return true;
}

bool dev_spiflash_sector_erase(DevSpiFlashNode *node, uint32_t sector)
{
	uint32_t addr;

	if(sector >= node->dev.pra->sectornum)
		return false;

	addr = sector*(node->dev.pra->sectorsize);

	dev_spiflash_erase(node, addr);

	return true;
}

bool dev_spiflash_sector_read(DevSpiFlashNode *node, uint32_t sector, uint8_t *dst)
{
	if(sector >= node->dev.pra->sectornum)
		return false;

	return dev_spiflash_read(node, sector*(node->dev.pra->sectorsize), node->dev.pra->sectorsize, dst);
}

bool dev_spiflash_sector_write(DevSpiFlashNode *node, uint32_t sector, uint8_t *src)
{
	uint16_t sector_size;

	if(sector >= node->dev.pra->sectornum)
		return false;

	sector_size = node->dev.pra->sectorsize;
	dev_spiflash_page_write(node, sector*sector_size, sector_size, src);
	return true;
}

static uint32_t dev_spiflash_readMTD(DevSpiFlashNode *node)
{
    uint32_t MID;
    uint8_t command = SPIFLASH_RDMID;
    uint8_t data[2];

	mcu_spi_cs(node->spichnode, 0);
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);

    command = 0xff;
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);
    mcu_spi_transfer(node->spichnode, NULL, data, 2);
	mcu_spi_cs(node->spichnode, 1);

    MID = data[0];
    MID = (MID<<8) + data[1];

    return MID;
}

static uint32_t dev_spiflash_readJTD(DevSpiFlashNode *node)
{
    uint32_t JID;
    uint8_t command = SPIFLASH_RDJID;
    uint8_t data[3];

	mcu_spi_cs(node->spichnode, 0);
    mcu_spi_transfer(node->spichnode, &command, NULL, 1);
    mcu_spi_transfer(node->spichnode, NULL, data, 3);
	mcu_spi_cs(node->spichnode, 1);

    JID = data[0];
    JID = (JID<<8) + data[1];
    JID = (JID<<8) + data[2];

    return JID;
}

DevSpiFlashNode *dev_spiflash_open(char* name)
{

	DevSpiFlashNode *node;
	struct list_head *listp;

	// SPIFLASH_DEBUG(LOG_INFO, "spi flash open:%s!\r\n", name);

	listp = DevSpiFlashRoot.next;
	node = NULL;

	while(1)
	{
		if(listp == &DevSpiFlashRoot)
			break;

		node = list_entry(listp, DevSpiFlashNode, list);
		// SPIFLASH_DEBUG(LOG_INFO, "spi ch name%s!\r\n", node->dev.name);

		if(strcmp(name, node->dev.name) == 0)
		{
			// SPIFLASH_DEBUG(LOG_INFO, "spi ch dev get ok!\r\n");
			break;
		}
		else
		{
			node = NULL;
		}

		listp = listp->next;
	}

	if(node != NULL)
	{

		if(node->gd == 0)
		{
			// SPIFLASH_DEBUG(LOG_INFO, "spi flash open err:using!\r\n");
			node = NULL;
		}
		else
		{
			node->spichnode = mcu_spi_open(node->dev.spich, SPI_MODE_3, SPI_BAUDRATEPRESCALER_4);
			if(node->spichnode == NULL)
				node = NULL;
			else
				node->gd = 0;
		}
	}

	return node;
}

bool dev_spiflash_close(DevSpiFlashNode *node)
{
	if(node->gd != 0)
		return false;

	mcu_spi_close(node->spichnode);

	node->gd = -1;
	return true;
}

bool dev_spiflash_register(const DevSpiFlash *dev)
{
	struct list_head *listp;
	DevSpiFlashNode *node;

	uint32_t JID = 0;
	uint32_t MID = 0;
	uint8_t index = 0;

	// wjq_log(LOG_INFO, "[register] spi flash :%s!\r\n", dev->name);

	/*
		先要查询当前，防止重名
	*/
	listp = DevSpiFlashRoot.next;
	while(1)
	{
		if(listp == &DevSpiFlashRoot)
			break;

		node = list_entry(listp, DevSpiFlashNode, list);

		if(strcmp(dev->name, node->dev.name) == 0)
		{
			// wjq_log(LOG_INFO, "spi flash dev name err!\r\n");
			return false;
		}

		listp = listp->next;
	}

	/*
		申请一个节点空间

	*/
	node = (DevSpiFlashNode *)MALLOC(sizeof(DevSpiFlashNode));
	list_add(&(node->list), &DevSpiFlashRoot);
	memcpy((uint8_t *)&node->dev, (uint8_t *)dev, sizeof(DevSpiFlash));
	node->gd = -1;

	/*读 ID，超找FLASH信息*/
	node->spichnode = mcu_spi_open(dev->spich, SPI_MODE_3, SPI_BAUDRATEPRESCALER_4); //打开spi

	if(node->spichnode != NULL)
	{
		JID = dev_spiflash_readJTD(node);
		// wjq_log(LOG_DEBUG, "%s jid:0x%x\r\n", dev->name, JID);

		MID  = dev_spiflash_readMTD(node);
		// wjq_log(LOG_DEBUG, "%s mid:0x%x\r\n", dev->name, MID);

		/*根据JID查找设备信息*/
		for(index = 0; index<(sizeof(SpiFlashPraList)/sizeof(_strSpiFlash));index++)
		{
			if((SpiFlashPraList[index].JID == JID)
				&&(SpiFlashPraList[index].MID == MID))
			{
				node->dev.pra = &(SpiFlashPraList[index]);
				break;
			}
		}
		mcu_spi_close(node->spichnode);
	}
    else
    {
        return false;
    }
	return true;
}

char spiname1[10];
char spiname2[10];
uint8_t test[256] = {0};
uint8_t rtest[256];
uint8_t rtest2[256];
void flash_test(void)
{
    struct list_head *listp;
    DevSpiFlashNode *p;
    uint8_t command[4];

    DevSpiFlash spifalsh = {
        "nm25q128",
        "WQ128",
    };

    if(dev_spiflash_register(&spifalsh) == true)
    {
        // listp = DevSpiFlashRoot.next;
        // p = list_entry(listp, DevSpiFlashNode, list);
        // memcpy((char *)spiname1, (char *)p->dev.name, 10);
        // memcpy((char *)spiname2, (char *)p->dev.spich, 10);

        p = dev_spiflash_open("nm25q128");
        for(int i = 0; i < 256; i++) {
            test[i] = i;
        }
        dev_spiflash_erase(p, 0);
        // dev_spiflash_page_write(p, 0, 256, test);
        dev_spiflash_write(p, 0, 256, test);
        // dev_spiflash_write(p, 0, 256, test);
        dev_spiflash_read(p, 0, 256, rtest);
        for(int i = 0; i < 256; i++) {
            test[i] = 255-i;
        }
        dev_spiflash_erase(p, 0);
        // dev_spiflash_page_write(p, 0, 256, test);
        dev_spiflash_write(p, 0, 256, test);
        // dev_spiflash_write(p, 0, 256, test);
        dev_spiflash_read(p, 0, 256, rtest2);
    }
}

