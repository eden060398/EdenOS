#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef STDDEF_H
#define STDDEF_H
	#include <stddef.h>
#endif
#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define BIOS_MMAP 		0x15000
#define BIOS_MMAP_SIZE 	*((uint16_t*) 0x16000)
#define PAGE_SIZE 		4096
#define BITMAP_START 	0X100000
#define BITMAP_END 		0x200000

// -----------------------------------------------------------------------------
// Memory Management Module
// ------------------------
// 
// General	:	The module manages and allocates memory as a collection of pages.
//
// Input	:	None
//
// Process	:	Manages and allocates memory as a collection of pages.
//
// Output	:	None
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

enum MEM_TYPE
{
	USABLE = 1,
	RESERVED,
	ACPI_RECLAIMABLE_MEMORY,
	ACPI_NVS_MEMORY,
	BAD_MEMORY
};

struct bios_mmap_entry
{
	uint16_t entry_size;
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t extended;
} __attribute__((packed));

const char *TYPES[6] = {0, "Usable", "Reserved", "ACPI Reclaimable Memory", "ACPI NVS Memory", "Bad Memory"};

// FUNCTION DECLARATION
void init_palloc(void);
void print_bios_mmap(void);
static void remove_page_block(uint32_t base, uint32_t size);
void *palloc(void);
void pfree(void *ptr);

// -----------------------------------------------------------------------------
// init_palloc
// ---------------
// 
// General		:	The function initializes the bit-map for page allocation.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_palloc(void)
{
	struct bios_mmap_entry *entry_ptr;
	uint32_t *p, i;
	
	p = (uint32_t *) BITMAP_START;	
	while (p < (uint32_t *) BITMAP_END)
		*p++ = 0;	
	entry_ptr = (struct bios_mmap_entry *) BIOS_MMAP;
	for (i = 0; i < BIOS_MMAP_SIZE; i += 26)
	{
		if (entry_ptr->type != 1)
		{
			remove_page_block(entry_ptr->base, entry_ptr->length);
		}
	}
	remove_page_block(0, BITMAP_END);	
}

// -----------------------------------------------------------------------------
// print_bios_mmap
// ---------------
// 
// General		:	The function prints the BOIS Memory Map Entries.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void print_bios_mmap(void)
{
	struct bios_mmap_entry *p;
	uint16_t i;
	char *buff;
	
	buff = malloc(100);	
	puts("Size: ");
	puts(uitoa(BIOS_MMAP_SIZE / 26, buff, 10));
	puts(" Entries\n");
	p = (struct bios_mmap_entry*) BIOS_MMAP;
	for (i = 0; i < BIOS_MMAP_SIZE; i += 26)
	{
		if (p->entry_size == 20 || p->entry_size == 24)
		{
			puts("Base = ");
			puts(uitoa(p->base, buff, 16));
			puts(", Length = ");
			puts(uitoa(p->length, buff, 10));
			puts(" Bytes");
			if (p->length > 1 << 20)
			{
				puts(" (");
				puts(uitoa(p->length >> 20, buff, 10));
				puts("MB)");
			}
			else if (p->length > 1 << 10)
			{
				puts(" (");
				puts(uitoa(p->length >> 10, buff, 10));
				puts("KB)");
			}
			puts(", Type = ");
			puts(TYPES[p->type]);
			if (p->entry_size == 24)
			{
				puts(", Extended = ");
				puts(itoa(p->extended, buff, 16));
			}
			putc('\n');
		}
		p++;
	}
	free(buff);
}

// -----------------------------------------------------------------------------
// remove_page_block
// -----------------
// 
// General		:	The function removes a block of pages from the bit-map.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
static void remove_page_block(uint32_t base, uint32_t size)
{
	uint32_t *p;
	uint32_t s;
	
	p = (uint32_t *) (BITMAP_START + base / PAGE_SIZE);
	if (base % PAGE_SIZE)
	{
		p++;
		s = (size - (PAGE_SIZE - base % PAGE_SIZE) % PAGE_SIZE) / PAGE_SIZE;
	}
	else
	{
		s = size / PAGE_SIZE;
	}
	while (s >= 32)
	{
		*p++ = 0xFFFFFFFF;
		s -= 32;
	}
	while (s)
	{
		*p = (*p << 1) & 1;
		s--;
	}
}

// -----------------------------------------------------------------------------
// palloc
// ------
// 
// General		:	The function allocates a page.
//
// Parameters	:	None
//
// Return Value	:	The pointer to the page allocated
//
// -----------------------------------------------------------------------------
void *palloc(void)
{
	void *ptr;
	uint32_t *p, b;
	int i;
	
	p = (uint32_t *) BITMAP_START;
	while (*p == 0xFFFFFFFF && p < (uint32_t *) BITMAP_END)
		p++;
	ptr = (void *) (((uint32_t) p - BITMAP_START) * PAGE_SIZE * 8);
	b = 1;
	while (*p & b)
	{
		ptr += PAGE_SIZE;
		b <<= 1;
	}
	*p |= b;
	
	p = (uint32_t *) ptr;
	for (i = 0; i < PAGE_SIZE / 32; i++)
		*p++ = 0;
	
	return ptr;
}

// -----------------------------------------------------------------------------
// pfree
// -----
// 
// General		:	The function frees an allocated page.
//
// Parameters	:
//		ptr	-	A pinter to a page
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void pfree(void *ptr)
{
	uint32_t base;
	uint8_t *p, b;
	base = (uint32_t) ptr;
	if (!(base % PAGE_SIZE))
	{
		p = (uint8_t *) (BITMAP_START + base / (PAGE_SIZE * 8));
		base %= PAGE_SIZE * 8;
		b = 1 << (base / PAGE_SIZE);
		*p &= ~b;
	}
}