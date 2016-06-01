#ifndef PAGING_H
#define PAGING_H

#include <system.h>

// DEFINITIONS

#define PAGE_TABLE_LEN 4096
#define PAGE_DIR_LEN 4096
#define PAGE_SIZE 4096

#define FLAG_WRITE_THROUGH 1
#define FLAG_CACHE_DISABLED 0
#define FLAG_PAGE_SIZE 0
#define FLAG_IGNORED 0
#define FLAG_GLOBAL 0

#define KERNEL_REGION_END 0x02000000

#define CREATE_ANYWAY 1 << 0
#define OVERRIDE_USER_SUPERVISOR 1 << 1
#define OVERRIDE_READ_WRITE 1 << 2
#define MIXED_USER_SUPERVISOR 1 << 3
#define MIXED_READ_WRITE 1 << 4

// STRUCTURES

typedef struct page_table_entry {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t base_address : 20;
} __attribute__((packed)) PTE;

typedef struct page_dir_entry {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t reserved_zero : 1;
    uint32_t page_size : 1;
    uint32_t ignored : 1;
    uint32_t available : 3;
    uint32_t base_address : 20;
} __attribute__((packed)) PDE;

// FUNCTION DECLARATION

void init_paging(void);
uint32_t dup_dir(uint32_t new_dir, uint32_t ori_dir);
void create_entry(uint32_t dir_addr, uint32_t laddr, uint32_t paddr,
        int read_write, int user_supervisor, uint8_t flags);
void remove_entry(uint32_t dir_addr, uint32_t laddr);

#endif /* PAGING_H */

