#include "mem_cache.h"

/*
 * Determines the chunk sizes and initializes the slab class descriptors
 * accordingly.
 */
mem_cache_ptr mem_cache_create(size_t base_chunk_size, 
        double factor, 
        size_t power_largest, 
        size_t power_block, 
        size_t mem_limit, 
        int pre_alloc) {

    mem_cache_ptr mem_cache;
    slab_ptr *slabclass;
    size_t i;
    size_t chunk_size;
    if (base_chunk_size <= CHUNK_ALIGN_BYTES || 
            power_largest <= 0 || 
            power_block <= CHUNK_ALIGN_BYTES ||
            power_block < base_chunk_size ||
            mem_limit <= CHUNK_ALIGN_BYTES) {
        return NULL;
    }

    mem_cache = (mem_cache_ptr)calloc(1, sizeof(*mem_cache));
    if (NULL == mem_cache) {
        return NULL;
    }
    
    slabclass = (slab_ptr *)calloc(power_largest, sizeof(slab_ptr));
    if (NULL == slabclass) {
        free (mem_cache);
        return NULL;
    }
    mem_cache->mem_limit = mem_limit;
    mem_cache->mem_malloced = 0;
    mem_cache->factor = factor;
    mem_cache->power_block = power_block;
    mem_cache->base_chunk_size = base_chunk_size;
    mem_cache->power_smallest = 1;
    mem_cache->slabclass = slabclass;
    mem_cache->magic_number = MEMCACHE_MAGIC_NUMBER;
    for (i = 1, chunk_size = base_chunk_size; i < power_largest; ++i) {
        if (chunk_size > power_block) {
            break;
        }
        slabclass[i] = slab_create(chunk_size, power_block, pre_alloc);
        chunk_size *= factor;
    }
    
    mem_cache->power_largest = i - 1;
    mem_cache->mem_malloced = mem_cache->power_largest;
    
    return mem_cache;
}

void *mem_cache_alloc(mem_cache_ptr mem_cache, size_t size) {
    void *ptr;
    slab_ptr slab;
    size_t org_page_total;
      
    if (NULL == mem_cache || size <= 0) {
        return 0;
    }

    if (mem_cache->mem_malloced >= mem_cache->mem_limit) {
        return NULL;
    }

    size += sizeof(mem_cache_header_t);
    size_t slab_id = mem_cache_clsid(mem_cache, size);
    if (slab_id <= 0) {
        return NULL;
    }

    slab = mem_cache->slabclass[slab_id];
    org_page_total = slab->page_total;
    ptr = slab_alloc_chunk(slab);

    if (NULL == ptr) {
        return NULL;
    }

    if (org_page_total < slab->page_total) {
        mem_cache->mem_malloced += slab->page_size;
    }
    ((mem_cache_header_ptr)ptr)->mem_cache = mem_cache;
    ((mem_cache_header_ptr)ptr)->size = size;
    return ptr + sizeof(mem_cache_header_t);
}

void mem_cache_free(void *ptr) {
    size_t size;
    size_t slab_id;
    mem_cache_ptr mem_cache;
    mem_cache_header_ptr mem_cache_header;
    if (NULL == ptr) {
        return;
    }
    mem_cache_header = (mem_cache_header_ptr)(ptr - sizeof(mem_cache_header_t));
    mem_cache = mem_cache_header->mem_cache;
    if (MEMCACHE_MAGIC_NUMBER != mem_cache->magic_number) {
        return;        
    }
    size = mem_cache_header->size;
    if (size <= 0) {
        return;
    }
    slab_id = mem_cache_clsid(mem_cache, size);
    if (slab_id <= 0) {
        return;
    }
    slab_free_chunk(mem_cache->slabclass[slab_id], mem_cache_header);
}

/*
 * Figures out which slab class (chunk size) is required to store an item of
 * a given size.
 * Binary search argrithm
 */
size_t mem_cache_clsid(mem_cache_ptr mem_cache, size_t size) {
    size_t res;
    if(0 == size) {
        return 0;
    }

    res = mem_cache->power_smallest;
    while (size > mem_cache->slabclass[res]->chunk_size) {
        /* won't fit in the biggest slab */
        if (res >= mem_cache->power_largest) {
            return 0;
        }
        ++ res;
    }
    return res;
}
