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

// -----------------------------------------------------------------------------
// Dynamic Memory Module
// ---------------------
// 
// General	:	The module initializes the dynamic memory and lets the kernel
//				allocate memory dynamically.
//
// Input	:	None
//
// Process	:	Allocates pages for dynamic usage and divides them as requested.
//
// Output	:	None
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

static void *base_page;

// FUNCTION DECLARATIONS
void init_malloc(void);
void *malloc(size_t size);
void free(void *ptr);

// -----------------------------------------------------------------------------
// init_malloc
// ------------
// 
// General		:	The function initializes the dymanic memory.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_malloc(void)
{	
	base_page = palloc();
}

// -----------------------------------------------------------------------------
// malloc
// ------
// 
// General		:	The function allocates memory for dynamic usage.
//
// Parameters	:
//		size	-	The size of space to be allocated (In)
//
// Return Value	:	A pointer to the allocated space
//
// -----------------------------------------------------------------------------
void *malloc(size_t size)
{
	uint32_t *p, b, page_addr;
	uint8_t *temp;
	size_t n;
	int i;
	void *page;
	
	if (size > PAGE_SIZE)
		return NULL;
	
	// Start with the base page.
	page = base_page;
	while (1)
	{
		// Point 'p' to the begining of the page - where there is a bit-map of the free bytes.
		p = (uint32_t *) page;
		// The amount of free bytes found is set to zero.
		n = 0;
		// '1' means a allocated byte in the bit-map. 'b' is set to the first bit of the first
		// byte of the bit-map.
		b = 1;
		// Scan the page for free memory of size 'n'.
		for (i = 0; i < ALLOC_AREA; i++)
		{
			if (*p & b)
				// If the byte is already allocated, set the number of free bytes found to zero.
				n = 0;
			else
				// Otherwise, increase that number.
				n += 1;
			if (n == size + sizeof(size_t))
				// If the amout of bytes found is enough for the wanted space + the size of the
				// space, exit the loop.
				break;
			if (b == 1 << 31) {
				// If the bit is the highest one, proceed to the first bit of the next byte.
				b = 1;
				p++;
			}
			else
				// Otherwise, proceed to the next bit.
				b <<= 1;
		}
		// If n is enough for the allocation, set the bits of that space in the bit-map,
		// put the size on the bytes before the space and return a pointer to the space
		// requested.
		if (n == size + sizeof(size_t))
		{
			temp = (uint8_t *) page + (BMAP_AREA + i);
			while (n) {
				*p |= b;
				if (b == 1)
				{
					p--;
					b = 1 << 31;
				} 
				else
					b >>= 1;
				*temp-- = 0;
				n--;
			}
			*((size_t *) ++temp) = size;
			temp += sizeof(size_t);
			return (void *) temp;
		}
		// Otherwise, go to the next page.
		page_addr = *((uint32_t *) (page + PAGE_SIZE - NEXT_PTR_SIZE));
		if (page_addr)
			// If a next page exist, check it.
			page = (void *) page_addr;
		else
		{
			// Otherwise, allocate a new one.
			page_addr = (uint32_t) palloc();
			if (!page_addr)
				return NULL;
			*((uint32_t *) (page + PAGE_SIZE - NEXT_PTR_SIZE)) = page_addr;
			page = (void *) page_addr;
		}
	}
}

// -----------------------------------------------------------------------------
// free
// ----
// 
// General		:	The fuctions deallocates allocated memory.
//
// Parameters	:
//		ptr	-	A pointer to the allocated space (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void free(void *ptr)
{
	size_t size;
	void *page;
	uint32_t *p, b;
	uint8_t *temp;
	
	page = base_page;
	size = *((size_t *) (ptr -= sizeof(size_t))) + sizeof(size_t);
	// While 'ptr' doesn't point to space on that page.
	while (ptr < page || ptr - page >= PAGE_SIZE)
	{
		if (*((uint32_t *) (page + PAGE_SIZE - NEXT_PTR_SIZE)))
			// If a next page exist, check it.
			page = (void *) *((uint32_t *) (page + PAGE_SIZE - NEXT_PTR_SIZE));
		else
			// Otherwise, the pointer is illogical. return.
			return;
	}
	// Calculate the place in the bit-map of the space 'ptr' points to.
	p = (uint32_t *) (page + (ptr - page - BMAP_AREA) / BITS_PER_BYTE);
	b = 1 << ((ptr - page - BMAP_AREA) % BITS_PER_BYTE);
	temp = (uint8_t *) ptr;
	// Revert the bits in the bit-map to zero for that space (deallocate).
	while (size--)
	{
		*p &= ~b;
		if (b == 1 << 31)
		{
			b = 1;
			p++;
		}
		else
			b <<= 1;
		*temp++ = 0;
	}
}