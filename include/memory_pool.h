// memory_pool.h
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define POOL_BLOCK_SIZE 4096

typedef struct pool_block {
    struct pool_block *next;
    uint8_t data[POOL_BLOCK_SIZE];
} pool_block_t;

typedef struct memory_pool {
    pool_block_t *free_blocks;
    uint8_t *current_pos;
    pool_block_t *current_block;
} memory_pool_t;

memory_pool_t *create_memory_pool(int size);
void destroy_memory_pool(memory_pool_t *pool);
void *pool_alloc(memory_pool_t *pool, size_t size);
void pool_free(memory_pool_t *pool, void *ptr);

#endif // MEMORY_POOL_H
