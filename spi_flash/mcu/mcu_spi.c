/**
 * @file mcu_spi.c
 * @author h
 * @brief spi驱动
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "mcu_spi.h"
#include <string.h>

#define MCU_SPI_WAIT_TIMEOUT 0x40000

/*
	相位配置，一共四种模式
*/
typedef struct
{
	uint16_t CPOL;
	uint16_t CPHA;
}_strSpiModeSet;

const _strSpiModeSet SpiModeSet[SPI_MODE_MAX]=
{
    {SPI_POLARITY_LOW, SPI_PHASE_1EDGE},
    {SPI_POLARITY_LOW, SPI_PHASE_2EDGE},
    {SPI_POLARITY_HIGH, SPI_PHASE_1EDGE},
    {SPI_POLARITY_HIGH, SPI_PHASE_2EDGE}
};

/* 定义spi设备链表头 */
LIST_HEAD(DevSpiRoot);

/* 定义spi通道链表头 */
LIST_HEAD(DevSpiChRoot);

/**
 * @brief 硬件spi初始化
 *
 * @param dev spi的参数指针
 *        @arg 设备名称
 *        @arg 设备类型：模拟/硬件
 *        @arg CLK/MOSI/MISO端口和引脚
 *
 * @attention 根据不同的初始化SPI方式修改此函数
 *            cubemx生成的直接将spi初始化写入
 *            自行实现的根据实际需要实现
 */
static void mcu_hspi_init(const DevSpi *dev)
{
    if(strcmp(dev->name, "SPI1") == 0)
    {
        MX_SPI1_Init();
    }
    // else if(strcmp(dev->name, "SPI2") == 0)
    // {
    //     MX_SPI2_Init();
    // }
    // else if(strcmp(dev->name, "SPI3") == 0)
    // {
    //     MX_SPI3_Init();
    // }
}

/**
 * @brief Enable SPI
 *
 * @param node spi控制器的节点指针
 * @param mode 打开spi的模式
 * @param pre  spi的预分频系数
 * @return bool 成功:true, 失败:false
 */
static bool mcu_hspi_open(DevSpiNode *node, SPI_MODE mode, uint16_t pre)
{
    SPI_HandleTypeDef *hspi;
    if(node->gd != -1)
        return false;

    if(mode >= SPI_MODE_MAX)
        return false;

    if(strcmp(node->dev.name, "SPI1") == 0)
        hspi = &hspi1;
    // else if(strcmp(node->dev.name, "SPI2") == 0)
    //     hspi = &hspi2;
    // else if(strcmp(node->dev.name, "SPI3") == 0)
    //     hspi = &hspi3;

    __HAL_SPI_ENABLE(hspi);

    node->gd = 0;

    return true;
}

/**
 * @brief Close SPI
 *
 * @param node spi控制器的节点指针
 * @return bool 成功:true, 失败:false
 */
static bool mcu_hspi_close(DevSpiNode *node)
{
    SPI_HandleTypeDef *hspi;

    if(node->gd != 0)
        return false;

    if(strcmp(node->dev.name, "SPI1") == 0)
        hspi = &hspi1;
    // else if(strcmp(node->dev.name, "SPI2") == 0)
    //     hspi = &hspi2;
    // else if(strcmp(node->dev.name, "SPI3") == 0)
    //     hspi = &hspi3;

    __HAL_SPI_DISABLE(hspi);

    node->gd = -1;

    return true;
}

static bool mcu_spi_transfer_byte(SPI_HandleTypeDef *spin, uint8_t *send, uint8_t *rsv)
{
    uint32_t time_out = MCU_SPI_WAIT_TIMEOUT;
    uint8_t temp;

    /* 等待发送缓冲区为空，TXE事件 */
    while (__HAL_SPI_GET_FLAG( spin, SPI_FLAG_TXE ) == RESET)
    {
        if((time_out--) == 0) return false;
    }

    /* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
    WRITE_REG(spin->Instance->DR, (*send));

    time_out = MCU_SPI_WAIT_TIMEOUT;

    /* 等待接收缓冲区非空，RXNE事件 */
    while (__HAL_SPI_GET_FLAG( spin, SPI_FLAG_RXNE ) == RESET)
    {
        if((time_out--) == 0) return false;
    }

    /* 读取数据寄存器，获取接收缓冲区数据 */
    temp = READ_REG(spin->Instance->DR);
    if(rsv)
    {
        *rsv = temp;
    }

    return true;
}

/**
 * @brief 硬件spi收发数据
 *
 * @param node spi控制器的节点指针
 * @param send 发送数据的指针
 * @param rsv 接收数据指针
 * @param len 收/发数据长度
 * @return bool 成功:true, 失败:false
 */
static bool mcu_hspi_transfer(DevSpiNode *node, uint8_t *send, uint8_t *rsv, uint32_t len)
{
    SPI_HandleTypeDef *hspi;
    uint32_t time_out = 0;
    uint8_t temp = 0xff;

    if(node == NULL)
		return false;

	if(node->gd != 0)
	{
		// SPI_DEBUG(LOG_DEBUG, "spi dev no open\r\n");
		return false;
	}

    if( ((send == NULL) && (rsv == NULL)) || (len < 0) )
        return false;

    if(strcmp(node->dev.name, "SPI1") == 0)
        hspi = &hspi1;

     /* 忙等待 */
    time_out = 0;
    while(__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_BSY) == SET)
    {
        if(time_out++ > MCU_SPI_WAIT_TIMEOUT)
        {
            return false;
        }
    }

    if(send)
    {
        while(len--)
        {
            if(mcu_spi_transfer_byte(hspi, send, NULL) != true)
                return false;
            send++;
        }
    }
    else if(rsv)
    {
        while(len--)
        {
            if(mcu_spi_transfer_byte(hspi, &temp, rsv) != true)
                return false;
            rsv++;
        }
    }
    else
    {
        return false;
    }

    return true;
}

/**
 * @brief 软件spi初始化，主要初始化对应引脚
 *
 * @param dev spi的参数指针
 */
static void mcu_vspi_init(const DevSpi *dev)
{
    if(strcmp(dev->name, "SPI1") == 0)
    {
        // MX_SPI1_Init();
    }
    // else if(strcmp(dev->name, "SPI2") == 0)
    // {
    //
    // }
    // else if(strcmp(dev->name, "SPI3") == 0)
    // {
    //
    // }
}

/**
 * @brief Enable SPI
 *
 * @param node spi控制器的节点指针
 * @param mode 打开spi的模式
 * @param pre  spi的预分频系数
 * @return bool 成功:true, 失败:false
 */
static bool mcu_vspi_open(DevSpiNode *node, SPI_MODE mode, uint16_t pre)
{
    if(node == NULL)
		return false;

	if(node->gd != -1)
	{
		//SPI_DEBUG(LOG_DEBUG, "vspi dev busy\r\n");
		return false;
	}

	node->clk = pre;
	node->gd = 0;

    return true;
}

/**
 * @brief Close SPI
 *
 * @param node spi控制器的节点指针
 * @return bool 成功:true, 失败:false
 */
static bool mcu_vspi_close(DevSpiNode *node)
{
    if(node->gd != 0)
		return false;
	//SPI_DEBUG(LOG_DEBUG, "vc-");
	node->gd = -1;

    return true;
}

/**
 * @brief 软件spi收发数据
 *
 * @param node spi控制器的节点指针
 * @param send 发送数据的指针
 * @param rsv 接收数据指针
 * @param len 收/发数据长度
 * @return bool 成功:true, 失败:false
 */
static bool mcu_vspi_transfer(DevSpiNode *node, uint8_t *send, uint8_t *rsv, uint32_t len)
{
    uint8_t i;
	uint8_t data;
	int32_t slen;
	uint8_t misosta;

	volatile uint16_t delay;

	DevSpi *dev;

    if(node == NULL)
	{
		// SPI_DEBUG(LOG_DEBUG, "vspi dev err\r\n");
		return false;
	}

	if(node->gd != 0)
	{
		// SPI_DEBUG(LOG_DEBUG, "vspi dev no open\r\n");
		return false;
	}

    if( ((send == NULL) && (rsv == NULL)) || (len < 0) )
    {
        return false;
    }

    dev = &(node->dev);

    slen = 0;

    while(1)
    {
        if(slen >= len)
			break;

        if(send == NULL)
			data = 0xff;
		else
			data = *(send+slen);

        for(i=0; i<8; i++)
		{
            mcu_io_resetbit(dev->clk.gpio_port, dev->clk.gpio_pin);

			delay = node->clk;
			while(delay-- > 0);

			if(data & 0x80)
            {
                mcu_io_setbit(dev->mosi.gpio_port, dev->mosi.gpio_pin);
            }
			else
            {
                mcu_io_resetbit(dev->mosi.gpio_port, dev->mosi.gpio_pin);
            }

			delay = node->clk;
			while(delay-- > 0);

			data <<= 1;
			mcu_io_setbit(dev->clk.gpio_port, dev->clk.gpio_pin);

			delay = node->clk;
			while(delay-- > 0);

			misosta = mcu_io_readbit(dev->miso.gpio_port, dev->miso.gpio_pin);
			if(misosta)
			{
				data |=0x01;
			}
			else
			{
				data &=0xfe;
			}

			delay = node->clk;
			while(delay-- > 0);
		}

        if(rsv != NULL)
			*(rsv+slen) = data;

		slen++;
    }

    return true;
}


/**
 * @brief 注册spi设备
 *
 * @param dev spi设备的参数指针
 * @return bool 成功:true, 失败:false
 */
bool mcu_spi_register(const DevSpi *dev)
{
    struct list_head *listp;
	DevSpiNode *p;

	// wjq_log(LOG_INFO, "[register] spi :%s!\r\n", dev->name);

    /*
		先要查询当前，防止重名
	*/
	listp = DevSpiRoot.next;
	while(1)
	{
		if(listp == &DevSpiRoot)
			break;

		p = list_entry(listp, DevSpiNode, list);

		if(strcmp(dev->name, p->dev.name) == 0)
		{
			// wjq_log(LOG_INFO, "spi dev name err!\r\n");
			return false;
		}

		listp = listp->next;
	}

    /*
        申请一个节点空间
	*/
	p = (DevSpiNode *)MALLOC(sizeof(DevSpiNode));
	list_add(&(p->list), &DevSpiRoot);

	memcpy((uint8_t *)&p->dev, (uint8_t *)dev, sizeof(DevSpi));

	/*初始化*/
	if(dev->type == DEV_SPI_V)
		mcu_vspi_init(dev);
	else if(dev->type == DEV_SPI_H)
		mcu_hspi_init(dev);

	p->gd = -1;

	return true;
}

/**
 * @brief 注册spi通道
 *
 * @param dev spi通道的参数指针
 * @return bool 成功:true, 失败:false
 */
bool mcu_spich_register(const DevSpiCh *dev)
{
    struct list_head *listp;
	DevSpiChNode *p;
	DevSpiNode *p_spi;

	// wjq_log(LOG_INFO, "[register] spi ch :%s!\r\n", dev->name);

	/*
		先要查询当前，防止重名
	*/
	listp = DevSpiChRoot.next;
	while(1)
	{
		if(listp == &DevSpiChRoot)
			break;

		p = list_entry(listp, DevSpiChNode, list);

		if(strcmp(dev->name, p->dev.name) == 0)
		{
			// wjq_log(LOG_INFO, ">--------------------spi ch dev name err!\r\n");
			return false;
		}

		listp = listp->next;
	}

	/* 寻找SPI控制器*/
	listp = DevSpiRoot.next;
	while(1)
	{
		if(listp == &DevSpiRoot)
		{
			// wjq_log(LOG_INFO, ">---------------------spi ch reg err:no spi!\r\n");
			return false;
		}

		p_spi = list_entry(listp, DevSpiNode, list);

		if(strcmp(dev->spi, p_spi->dev.name) == 0)
		{
			//wjq_log(LOG_INFO, "spi ch find spi!\r\n");
			break;
		}

		listp = listp->next;
	}
	/*
		申请一个节点空间
	*/
	p = (DevSpiChNode *)MALLOC(sizeof(DevSpiChNode));
	list_add(&(p->list), &DevSpiChRoot);
	memcpy((uint8_t *)&p->dev, (uint8_t *)dev, sizeof(DevSpiCh));
	p->gd = -1;
	p->spi = p_spi;

	/* 初始化管脚 */
	// mcu_io_config_out(dev->csport,dev->cspin);
    mcu_io_setbit(dev->cs.gpio_port, dev->cs.gpio_pin);

	return true;
}

/**
 * @brief 打开spi
 *
 * @param name 要打开的spi名称
 * @param mode 以什么模式打开
 * @param pre  spi的预分频系数(硬件)/spi的延时次数(软件)
 * @return DevSpiNode* 对应spi通道的地址
 */
DevSpiChNode *mcu_spi_open(char *name, SPI_MODE mode, uint16_t pre)
{
    bool res;
    DevSpiChNode *node;
    struct list_head *listp;

    //SPI_DEBUG(LOG_INFO, "spi ch open:%s!\r\n", name);

	listp = DevSpiChRoot.next;
	node = NULL;

	while(1)
	{
		if(listp == &DevSpiChRoot)
			break;

		node = list_entry(listp, DevSpiChNode, list);
		//SPI_DEBUG(LOG_INFO, "spi ch name%s!\r\n", node->dev.name);

		if(strcmp(name, node->dev.name) == 0)
		{
			//SPI_DEBUG(LOG_INFO, "spi ch dev get ok!\r\n");
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
			//SPI_DEBUG(LOG_INFO, "spi ch open err:using!\r\n");
			node = NULL;
		}
		else
		{
			/*打开SPI控制器*/
			if(node->spi->dev.type == DEV_SPI_H)
			{
				res = mcu_hspi_open(node->spi, mode, pre);
			}
			else if(node->spi->dev.type == DEV_SPI_V)
			{
				res = mcu_vspi_open(node->spi, mode, pre);
			}

			if(res == true)
			{
				node->gd = 0;
				//SPI_DEBUG(LOG_INFO, "spi dev open ok: %s!\r\n", name);
                mcu_io_resetbit(node->dev.cs.gpio_port, node->dev.cs.gpio_pin);
			}
			else
			{
				//SPI_DEBUG(LOG_INFO, "spi dev open err!\r\n");
				node = NULL;
			}
		}
	}
	else
	{
		// SPI_DEBUG(LOG_INFO, ">-------spi ch no exist!\r\n");
	}

	return node;
}

/**
 * @brief 关闭spi
 *
 * @param node spi通道指针
 * @return bool 成功:true, 失败:false
 */
bool mcu_spi_close(DevSpiChNode * node)
{
	if(node->spi->dev.type == DEV_SPI_H)
    {
        if(mcu_hspi_close(node->spi) != true)
            return false;
    }
	else
    {
        if(mcu_vspi_close(node->spi) != true)
            return false;
    }

	/*拉高CS*/
    mcu_io_setbit(node->dev.cs.gpio_port, node->dev.cs.gpio_pin);
	node->gd = -1;

	return true;
}

/**
 * @brief spi收发函数
 *
 * @param node spi通道节点指针
 * @param snd 发送数据的指针
 * @param rsv 接收数据指针
 * @param len 收/发数据长度
 * @return bool 成功:true, 失败:false
 */
bool mcu_spi_transfer(DevSpiChNode * node, uint8_t *snd, uint8_t *rsv, uint32_t len)
{
	if(node == NULL)
		return false;

	if(node->spi->dev.type == DEV_SPI_H)
		return mcu_hspi_transfer(node->spi, snd, rsv, len);
	else if(node->spi->dev.type == DEV_SPI_V)
		return mcu_vspi_transfer(node->spi, snd, rsv, len);
	else
	{
		// SPI_DEBUG(LOG_DEBUG, "spi dev type err\r\n");
	}
}

/**
 * @brief spi通道片选
 *
 * @param node spi通道节点指针
 * @param sta cs状态
 * @return bool 成功:true, 失败:false
 */
bool mcu_spi_cs(DevSpiChNode * node, uint8_t sta)
{
	switch(sta)
	{
		case 1:
            mcu_io_setbit(node->dev.cs.gpio_port, node->dev.cs.gpio_pin);
			break;

		case 0:
            mcu_io_resetbit(node->dev.cs.gpio_port, node->dev.cs.gpio_pin);
			break;

		default:
			return false;
	}

	return true;
}

char spi_name[16];
char spi_name2[16];
char spi_name3[16];
uint32_t JID;
uint32_t MID;
uint8_t read_buffer[256] = {0};
uint8_t write_buffer[256] = {0};
void spi_test(void)
{
    struct list_head *listp;
	DevSpiChNode *p;
	DevSpiNode *p_spi;
    uint8_t command[4];
    uint8_t data[3];

    DevSpi spi1 = {
        "SPI1",
        DEV_SPI_H,
    };

    DevSpiCh wq128 = {
        "WQ128",
        "SPI1",
        {.gpio_port = GPIOB, .gpio_pin = GPIO_PIN_14}
    };

    for(int i = 0; i < 256; i++)
    {
        write_buffer[i] = i;
    }

    mcu_spi_register(&spi1);
    mcu_spich_register(&wq128);

    // listp = DevSpiChRoot.next;
    // p = list_entry(listp, DevSpiChNode, list);
    // memcpy((uint8_t *)spi_name, (uint8_t *)p->dev.name, 6);
    // memcpy((uint8_t *)spi_name2, (uint8_t *)p->dev.spi, 6);

    // listp = DevSpiRoot.next;
    // p_spi = list_entry(listp, DevSpiNode, list);
    // memcpy((uint8_t *)spi_name3, (uint8_t *)p_spi->dev.name, 6);

    // mcu_spi_open("SPI1", 0, 0);

    // p = mcu_spi_open("WQ128", SPI_MODE_0, 0);
    // command[0] = 0x9f;
    // if(p)
    // {
    //     mcu_spi_cs(p, 0);
    //     mcu_spi_transfer(p, command, NULL, 1);
    //     mcu_spi_transfer(p, NULL, data, 3);
    //     mcu_spi_cs(p, 1);
    // }
    // JID = data[0];
    // JID = (JID<<8) + data[1];
    // JID = (JID<<8) + data[2];
    // command[0] = 0x90;
    // command[1] = 0xff;
    // command[2] = 0xff;
    // command[3] = 0xff;
    // if(p)
    // {
    //     mcu_spi_cs(p, 0);
    //     mcu_spi_transfer(p, command, NULL, 4);
    //     mcu_spi_transfer(p, NULL, data, 2);
    //     mcu_spi_cs(p, 1);
    // }
    // MID = data[0];
    // MID = (MID<<8) + data[1];

    // // 写使能
    // command[0] = 0x06;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 1);
    // mcu_spi_cs(p, 1);
    // // 查询状态
    // command[0] = 0x05;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 1);
    // do
    // {
    //     mcu_spi_transfer(p, NULL, &command[1], 1);
    // } while ((command[1] & 0x01) == 1);
    // mcu_spi_cs(p, 1);
    // // 扇区擦除
    // command[0] = 0x20;
    // command[1] = 0;
    // command[2] = 0;
    // command[3] = 0;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 4);
    // mcu_spi_cs(p, 1);
    // // 查询状态
    // command[0] = 0x05;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 1);
    // do
    // {
    //     mcu_spi_transfer(p, NULL, &command[1], 1);
    // } while ((command[1] & 0x01) == 1);
    // mcu_spi_cs(p, 1);

    // // 写使能
    // command[0] = 0x06;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 1);
    // mcu_spi_cs(p, 1);
    // // 查询状态
    // command[0] = 0x05;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 1);
    // do
    // {
    //     mcu_spi_transfer(p, NULL, &command[1], 1);
    // } while ((command[1] & 0x01) == 1);
    // mcu_spi_cs(p, 1);
    // // 写数据
    // command[0] = 0x02;
    // command[1] = 0;
    // command[2] = 0;
    // command[3] = 0;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 4);
    // mcu_spi_transfer(p, write_buffer, NULL, 256);
    // mcu_spi_cs(p, 1);
    // // 查询状态
    // command[0] = 0x05;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 1);
    // do
    // {
    //     mcu_spi_transfer(p, NULL, &command[1], 1);
    // } while ((command[1] & 0x01) == 1);
    // mcu_spi_cs(p, 1);
    // // 读数据
    // command[0] = 0x03;
    // command[1] = 0;
    // command[2] = 0;
    // command[3] = 0;
    // mcu_spi_cs(p, 0);
    // mcu_spi_transfer(p, command, NULL, 4);
    // mcu_spi_transfer(p, NULL, read_buffer, 256);
    // mcu_spi_cs(p, 1);
    // mcu_spi_close(p);
}









