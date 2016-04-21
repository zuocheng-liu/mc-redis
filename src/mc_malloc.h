#ifndef MC_MALLOC_H
#define MC_MALLOC_H
#include "mem_cache.h"
extern mem_cache_ptr redis_mem_cache;

void *mc_malloc(size_t size);
void *mc_realloc(void *ptr, size_t size);
void *mc_alloc(size_t n, size_t size);

void mc_free(void *ptr);

#endif /* End definition of MC_MALLOC_H */
