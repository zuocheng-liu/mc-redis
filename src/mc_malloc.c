#include "mc_malloc.h"
#include "string.h"


void *mc_malloc(size_t size) {
    return mem_cache_alloc(redis_mem_cache, size);
}

void *mc_realloc(void *ptr, size_t size) {
    void *new_ptr;
    mem_cache_ptr mem_cache;
    mem_cache_header_ptr mem_cache_header;

    if (NULL == ptr || size <= 0) {
        return NULL;
    }
    mem_cache_header = (mem_cache_header_ptr)(ptr - sizeof(mem_cache_header_t));
    mem_cache = mem_cache_header->mem_cache;
    if (mem_cache_header->size <= size) {
        return ptr;
    }
    if (mem_cache != redis_mem_cache) {
        return NULL;
    }
    if (MEMCACHE_MAGIC_NUMBER != mem_cache->magic_number) {
        return NULL;        
    }
    if (mem_cache_header->size <= 0) {
        return NULL;
    }

    new_ptr = mem_cache_alloc(redis_mem_cache, size);
    if (NULL == new_ptr) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, mem_cache_header->size);
    mem_cache_free(ptr);
    return new_ptr;
}
void *mc_alloc(size_t n, size_t size) {
    void *ptr = mem_cache_alloc(redis_mem_cache, size * n);
    bzero(ptr, size * n);
    return ptr;
}

void mc_free(void *ptr) {
    mem_cache_free(ptr);
}
