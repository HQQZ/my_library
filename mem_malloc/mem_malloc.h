/**
 * @file mem_malloc.h
 * @author h
 * @brief 静态内存池方式实现的内存malloc free memory
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __MEM_MALLOC_H__
#define __MEM_MALLOC_H__

#include <stdio.h>
#include <stdlib.h>

void mem_init(void);
void *mem_alloc(size_t size);
void mem_free(void *ptr);
void *mem_realloc(void *ptr, size_t size);

#endif /* __MEM_MALLOC_H__ */