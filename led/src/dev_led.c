/**
 * @file dev_led.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-10-31
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dev_led.h"

LIST_HEAD(dev_led_head);

/**
 * @brief Get the led head object
 *
 * @return struct list_head*
 */
struct list_head *get_led_head(void)
{
    return &dev_led_head;
}


/**
 * @brief open the led
 *
 * @param name led name
 * @return dev_led_node*
 */
dev_led_node *dev_led_open(char *name)
{
    dev_led_node *node, *pos;

    list_for_each_entry(pos, &dev_led_head, list)
    {
        if(strcmp(name, pos->dev.name) == 0)
        {
            node = pos;
            break;
        }
        else
        {
            node = NULL;
        }
    }

    if(node != NULL)
    {
        if(node->gd == 0)
        {
            node = NULL;
        }
    }

    return node;
}

/**
 * @brief close led
 *
 * @param name led name
 * @return int 0:ok   -1:faild
 */
int dev_led_close(char *name)
{
    dev_led_node *tmp;
    struct list_head *pos, *n;

    list_for_each_safe(pos, n, &dev_led_head)
    {
        tmp = list_entry(pos, dev_led_node, list);
        if(strcmp(name, tmp->dev.name) == 0)
        {
            list_del(pos);
            free(tmp);
            return 0;
        }
    }

    return -1;
}

/**
 * @brief add led to list
 *
 * @param dev led device parameter
 * @return int 0:ok   -1:faild
 */
int dev_led_add(dev_led *dev)
{
    dev_led_node *node, *pos;

    list_for_each_entry(pos, &dev_led_head, list)
    {
        if(strcmp(dev->name, pos->dev.name) == 0)
        {
            return -1;
        }
    }

    node = (dev_led_node *)malloc(sizeof(dev_led_node));
    if(node == NULL)
        return -1;
    list_add(&(node->list), &dev_led_head);
    memcpy((char *)&node->dev, (char *)dev, sizeof(dev_led));
    node->gd = -1;

    return 0;
}


