#ifndef DMEMORY_H
#define DMEMORY_H

#include <system.h>

// DEFINITIONS

#define CREATE_ANYWAY 1 << 0
#define OVERRIDE_USER_SUPERVISOR 1 << 1
#define OVERRIDE_READ_WRITE 1 << 2
#define MIXED_USER_SUPERVISOR 1 << 3
#define MIXED_READ_WRITE 1 << 4

#define PAGE_SIZE 4096
#define ALLOC_AREA 3632
#define BMAP_AREA 484
#define NEXT_PTR_SIZE 4
#define BITS_PER_BYTE 8

// FUNCTION DECLARATIONS

void init_malloc(void);
void *malloc(size_t size);
void free(void *ptr);

#endif /* DMEMORY_H */

