#ifndef MEMORY_H
#define MEMORY_H

#include <system.h>

// DEFINITIONS

#define BIOS_MMAP   0x15000
#define BIOS_MMAP_SIZE  *((uint16_t*) 0x16000)
#define PAGE_SIZE   4096
#define BITMAP_START  0X100000
#define BITMAP_END   0x200000

// ENUMERATIONS

enum MEM_TYPE {
    USABLE = 1,
    RESERVED,
    ACPI_RECLAIMABLE_MEMORY,
    ACPI_NVS_MEMORY,
    BAD_MEMORY
};

// STRUCTURES

struct bios_mmap_entry {
    uint16_t entry_size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t extended;
} __attribute__((packed));

// FUNCTION DECLARATION

void init_palloc(void);
void print_bios_mmap(void);
static void remove_page_block(uint32_t base, uint32_t size);
void *palloc(void);
void pfree(void *ptr);

#endif /* MEMORY_H */

