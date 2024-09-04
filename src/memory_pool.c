// memory_pool.c
#include "memory_pool.h"
#include <stdlib.h>

memory_pool_t *create_memory_pool(int size) {
    memory_pool_t *pool = (memory_pool_t *)malloc(sizeof(memory_pool_t));
    if (!pool) return NULL;

    pool->current_block = (pool_block_t *)malloc(size);
    if (!pool->current_block) {
        free(pool);
        return NULL;
    }
    pool->current_block->next = NULL;
    pool->current_pos = pool->current_block->data;
    pool->free_blocks = NULL;
    return pool;

}

void destroy_memory_pool(memory_pool_t *pool) {
    if (!pool){
        return;
    }
    pool_block_t *block = pool->free_blocks;
    while (block) {
        pool_block_t *next = block->next;
        free(block);
        block = next;
    }
    free(pool);
}

void *pool_alloc(memory_pool_t *pool, size_t size) {
    // 对齐请求的大小
    size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

    // 如果当前块为空或者没有足够的空间
    if (!pool->current_block || (pool->current_pos + size > pool->current_block->data + POOL_BLOCK_SIZE)) {
        // 如果当前块满了，分配一个新的块
        pool_block_t *block = (pool_block_t *)malloc(sizeof(pool_block_t));
        if (!block) {
            return NULL;
        }
        block->next = pool->free_blocks;
        pool->free_blocks = block;

        pool->current_block = block;
        pool->current_pos = block->data;
    }

    // 返回内存
    void *ptr = pool->current_pos;
    pool->current_pos += size;
    return ptr;
}

void pool_free(memory_pool_t *pool, void *ptr) {
    if (!ptr) {
        return;
    }

    // 将块添加到空闲块列表中
    pool_block_t *block = (pool_block_t *)((uint8_t *)ptr - sizeof(pool_block_t));
    block->next = pool->free_blocks;
    pool->free_blocks = block;
}
