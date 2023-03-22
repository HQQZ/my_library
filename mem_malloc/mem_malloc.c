/**
 * @file mem_malloc.c
 * @author h
 * @brief 静态内存池方式实现的内存malloc free memory
 * @version 0.1
 * @date 2023-03-22
 * @attention 申请到的内存不一定为0，使用前需要清零
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <string.h>
#include "mem_malloc.h"

//------------------------------ private ---------------------------------//
#define MEM_SIZE 1024 // 内存池大小
static char memory_pool[MEM_SIZE] = {0}; // 静态内存池

//------------------------------ typedef --------------------------------//
// 内存块结构体
typedef struct mem_block {
    size_t size; // 内存块大小
    int used; // 是否被使用
    struct mem_block *next; // 下一个内存块
    struct mem_block *prev; // 前一个内存块
} MemBlock;


/**
 * @brief 初始化内存申请内存池
 *
 */
void mem_init(void)
{
    MemBlock *block = (MemBlock*)memory_pool;
    block->size = MEM_SIZE - sizeof(MemBlock);
    block->next = block->prev = NULL;
    block->used = 0;
}

/**
 * @brief 内存申请
 *
 * @param size 要申请的内存大小，以Byte为单位
 * @return void* 申请到的内存的地址，NULL表示申请失败
 */
void *mem_alloc(size_t size)
{
    MemBlock *curr_block = (MemBlock*)memory_pool;

    // 遍历内存池查找合适的内存块
    while (curr_block)
    {
        if ((!curr_block->used) && (curr_block->size >= size))
        {
            // 找到合适的空闲内存块
            if (curr_block->size - size >= sizeof(MemBlock))
            {
                // 内存块大小大于size，需要拆分出一块空闲内存
                MemBlock *new_block = (MemBlock*)((char*)curr_block + sizeof(MemBlock) + size);
                new_block->size = curr_block->size - sizeof(MemBlock) - size;
                new_block->next = curr_block->next;
                new_block->prev = curr_block;
                new_block->used = 0;
                curr_block->size = size;
                curr_block->next = new_block;
            }
            // 标记内存块为已使用
            curr_block->used = 1;
            // 返回内存块指针
            return (void*)((char*)curr_block + sizeof(MemBlock));
        }
        curr_block = curr_block->next;
    }
    // 没有找到合适的内存块，返回NULL
    return NULL;
}

/**
 * @brief 内存释放
 *
 * @param ptr 要释放内存的指针
 */
void mem_free(void *ptr) {
    if (!ptr) {
        return;
    }
    MemBlock *curr_block = (MemBlock*)((char*)ptr - sizeof(MemBlock));
    // 标记内存块为未使用
    curr_block->used = 0;
    MemBlock *prev_block = curr_block->prev;
    MemBlock *next_block = curr_block->next;
    // 合并相邻的空闲内存块
    while (next_block) {
        if (!next_block->used && (char*)curr_block + sizeof(MemBlock) + curr_block->size == (char*)next_block) {
            curr_block->size += sizeof(MemBlock) + next_block->size;
            curr_block->next = next_block->next;
            // memset(next_block, 0, sizeof(MemBlock));
            next_block = curr_block->next;
        } else {
            break;
        }
    }
    while (prev_block) {
        if (!prev_block->used && (char*)prev_block + sizeof(MemBlock) + prev_block->size == (char*)curr_block) {
            prev_block->size += sizeof(MemBlock) + curr_block->size;
            prev_block->next = curr_block->next;
            // memset(curr_block, 0, sizeof(MemBlock));
            prev_block = prev_block->prev;
        } else {
            break;
        }
    }
}
// 内存重新分配函数
/**
 * @brief 重新分配内存，保留原数据
 *
 * @param ptr 需要重新分配的内存的指针
 * @param size 重新分配的大小
 * @return void* 重新分配后的地址
 */
void *mem_realloc(void *ptr, size_t size) {
    if (!ptr) {
        // 如果ptr为NULL，直接分配新的内存块
        return mem_alloc(size);
    }
    MemBlock *curr_block = (MemBlock*)((char*)ptr - sizeof(MemBlock));
    MemBlock *next_block = curr_block->next;
    if (size <= curr_block->size) {
        // 新内存大小小于等于原内存大小，直接返回原内存块指针
        return ptr;
    } else if (next_block && !next_block->used && curr_block->size + sizeof(MemBlock) + next_block->size >= size) {
        // 下一个内存块存在且合并后大小大于等于size，合并内存块并返回原内存块指针
        mem_free(ptr);
        mem_alloc(size);
        return ptr;
    } else {
        // 分配新内存并拷贝原内存数据
        void *new_ptr = mem_alloc(size);
        if (new_ptr) {
            memcpy(new_ptr, ptr, curr_block->size);
            mem_free(ptr);
        }
        return new_ptr;
    }
}

