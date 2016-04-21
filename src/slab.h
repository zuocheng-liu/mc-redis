#ifndef SLAB_H
#define SLAB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define CHUNK_ALIGN_BYTES (sizeof(void *))
#define PAGE_ALIGN_BYTES 1048576 /* 1M */
#define SLAB_MAGIC_NUMBER 0xABCDEFAA

#define GET_SLAB_FROM_CHUNK(ptr) (((slab_ptr)ptr) - 1)
#define CHECK_SLAB_MAGIC_NUMBER(ptr) (SLAB_MAGIC_NUMBER == ptr->magic_number)
typedef struct {
    size_t chunk_size;      /* sizes of chunk */
    size_t page_size;       /* sizes of page */
    unsigned int chunk_number_per_page;   /* how many chunks per page */

    void *end_page_ptr; /* pointer to next free item at end of page, or 0 */
    size_t end_page_free; /* number of chunks remaining at end of last alloced page */
    size_t page_total;  /* how many page were allocated for this class */
    
    void **free_chunk_list;     /* list of chunk ptrs */
    size_t free_chunk_list_length;  /* size of previous array */
    size_t free_chunk_end;   /* first free chunk */

} slab_t, *slab_ptr;


/* create a slab structure and initialize it */
slab_ptr slab_create(size_t chunk_size, size_t page_size, int pre_alloc);
/* new page */
int slab_new_page(slab_ptr slab);
/* Allocate object of given length. 0 on error */
void *slab_alloc_chunk(slab_ptr slab);

/* Free previously allocated object */
void slab_free_chunk(slab_ptr slab, void *ptr);

#endif /* End definition SLAB_H */
