// -----------------------------------------------------------------------------
// Paging Module
// -------------
// 
// General  :   The module manages Paging Directories and Paging Tables.
//
// Input    :	None
//
// Process  :   Manages Paging Directories and Paging Tables.
//
// Output   :   None
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------


#include <paging.h>


// -----------------------------------------------------------------------------
// init_paging
// -----------
// 
// General      :   The function initializes the Paging Directory of the kernel.
//
// Parameters   :   None
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void init_paging(void) {
    void *page_dir;
    uint32_t addr;

    page_dir = palloc();
    memset(page_dir, 0, PAGE_TABLE_LEN);

    for (addr = 0x00000000;; addr += PAGE_SIZE) {
        create_entry((uint32_t) page_dir, addr, addr, 0, 0, CREATE_ANYWAY | OVERRIDE_USER_SUPERVISOR | OVERRIDE_READ_WRITE);
        if (addr == 0xFFFFF000)
            break;
    }
    asm volatile ("movl %%eax, %%cr3" : : "a" ((uint32_t) page_dir));
    asm volatile ("movl %cr0, %eax");
    asm volatile ("orl $0x80000000, %eax");
    asm volatile ("movl %eax, %cr0");
}

// -----------------------------------------------------------------------------
// dup_dir
// -------
// 
// General      :   The function duplicates a Page Directory.
//
// Parameters   :
//              new_dir -   The address of the new directory
//              ori_dir -   The address of the original directory
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

uint32_t dup_dir(uint32_t new_dir, uint32_t ori_dir) {
    PDE *ori_pde;
    PDE *new_pde;
    void *pt;

    for (ori_pde = (PDE *) ori_dir, new_pde = (PDE *) new_dir; (uint32_t) ori_pde < ori_dir + PAGE_DIR_LEN; ori_pde++, new_pde++) {
        if (ori_pde->present) {
            new_pde->present = 1;
            new_pde->read_write = ori_pde->read_write;
            new_pde->user_supervisor = ori_pde->user_supervisor;
            new_pde->write_through = FLAG_WRITE_THROUGH;
            new_pde->cache_disabled = FLAG_CACHE_DISABLED;
            new_pde->accessed = 0;
            new_pde->reserved_zero = 0;
            new_pde->page_size = FLAG_PAGE_SIZE;
            new_pde->ignored = FLAG_IGNORED;
            new_pde->available = 0;
            pt = palloc();
            new_pde->base_address = ((uint32_t) pt) >> 12;
            memcpy(pt, (void *) ((ori_pde->base_address << 12) & 0xFFFFF000), PAGE_TABLE_LEN);
        } else
            memset((void *) new_pde, 0, sizeof (PDE));
    }

    return new_dir;
}

// -----------------------------------------------------------------------------
// create_entry
// ------------
// 
// General      :   The function creates or modifies an entry in a Page Directory
//                  (in a Page Table of the Page Directory - not necesarily in 
//                  the Page Directory).
//
// Parameters   :
//              dir_addr        -   The address of the Page Directory
//              laddr           -   The linear address of the page
//		paddr           -   The physical address of the page
//		read_write      -   A flag, that if set, allows writing and reading from
//                                  the page. Otherwise - reading only
//		user_supervisor	-   A flag, that if set, allows user and supervisor;
//                                  Otherwise - supervisor only
//              flags           -   The flags of the entry
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void create_entry(uint32_t dir_addr, uint32_t laddr, uint32_t paddr,
        int read_write, int user_supervisor, uint8_t flags) {
    PDE *pde;
    PTE *pte;
    void *pt;

    if (!(read_write == 0 || read_write == 1) || !(user_supervisor == 0 || user_supervisor == 1))
        return;

    pde = (PDE *) ((dir_addr & 0xFFFFF000) | ((laddr >> 20) & 0x00000FFC));

    if (!pde->present) {
        if (flags & MIXED_READ_WRITE)
            pde->read_write = 1;
        else
            pde->read_write = read_write;
        if (flags & MIXED_USER_SUPERVISOR)
            pde->user_supervisor = 1;
        else
            pde->user_supervisor = user_supervisor;
        pde->write_through = FLAG_WRITE_THROUGH;
        pde->cache_disabled = FLAG_CACHE_DISABLED;
        pde->accessed = 0;
        pde->reserved_zero = 0;
        pde->page_size = FLAG_PAGE_SIZE;
        pde->ignored = 0;
        pde->available = 0;
        pt = palloc();
        pde->base_address = ((uint32_t) pt) >> 12;
        memset(pt, 0, PAGE_SIZE);
        pde->present = 1;
    } else {
        if (!pde->user_supervisor && user_supervisor) {
            if (flags & OVERRIDE_USER_SUPERVISOR)
                pde->user_supervisor = user_supervisor;
            else
                return;
        }
        if (!pde->read_write && read_write) {
            if (flags & OVERRIDE_READ_WRITE)
                pde->read_write = read_write;
            else
                return;
        }
    }
    pte = (PTE *) (((pde->base_address << 12) & 0xFFFFF000) | ((laddr >> 10) & 0x00000FFC));

    if (pte->present && !(flags & CREATE_ANYWAY))
        return;
    pte->read_write = read_write;
    pte->user_supervisor = user_supervisor;
    pte->write_through = FLAG_WRITE_THROUGH;
    pte->cache_disabled = FLAG_CACHE_DISABLED;
    pte->accessed = 0;
    pte->dirty = 0;
    pte->page_size = FLAG_PAGE_SIZE;
    pte->global = 0;
    pte->available = 0;
    pte->base_address = paddr >> 12;
    pte->present = 1;
}

// -----------------------------------------------------------------------------
// remove_entry
// ------------
// 
// General      :   The function creates or modifies an entry in a Page Directory
//                  (in a Page Table of the Page Directory - not necesarily in 
//                  the Page Directory).
//
// Parameters   :
//              dir_addr        -   The address of the Page Directory
//              laddr           -   The linear address of the page
//              paddr           -   The physical address of the page
//              read_write      -   A flag, that if set, allows writing and reading from
//                                  the page. Otherwise - reading only
//              user_supervisor -   A flag, that if set, allows user and supervisor;
//                                  Otherwise - supervisor only
//              flags           -   The flags of the entry
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void remove_entry(uint32_t dir_addr, uint32_t laddr) {
    PDE *pde;
    PTE *pte;
    int i;

    pde = (PDE *) ((dir_addr & 0xFFFFF000) | ((laddr >> 20) & 0x00000FFC));
    if (pde->present) {
        pte = (PTE *) (((pde->base_address << 12) & 0xFFFFF000) | ((laddr >> 10) & 0x00000FFC));
        memset((void *) pte, 0, sizeof (PTE));
    }
    for (i = 0, pte = (PTE *) ((pde->base_address << 12) & 0xFFFFF000); i < 1024; i++, pte++)
        if (pte->present)
            return;
    pfree((void *) ((pde->base_address << 12) & 0xFFFFF000));
    memset((void *) pde, 0, sizeof (PDE));
}