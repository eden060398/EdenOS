#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define READ12_OPCODE 			0xA8
#define WRITE12_OPCODE 			0xAA
#define WRITE10_OPCODE 			0x2A
#define TEST_UNIT_READY_OPCODE	0x00
#define INQUIRY_OPCODE			0x12
#define REQUEST_SENSE_OPCODE	0x03
#define READ_CAPACITY10_OPCODE	0x25

typedef struct read12
{
	uint8_t opcode;
	uint8_t obsolete	: 1;
	uint8_t fua_nv		: 1;
	uint8_t reserved1	: 1;
	uint8_t fua			: 1;
	uint8_t dpo			: 1;
	uint8_t rdprotect	: 3;
	uint8_t lba4;
	uint8_t lba3;
	uint8_t lba2;
	uint8_t lba1;
	uint8_t len4;
	uint8_t len3;
	uint8_t len2;
	uint8_t len1;
	uint8_t group_num	: 5;
	uint8_t reseved2	: 2;
	uint8_t restricted	: 1;
	uint8_t control;
} __attribute__((packed)) Read12;

typedef struct write12
{
	uint8_t opcode;
	uint8_t obsolete	: 1;
	uint8_t fua_nv		: 1;
	uint8_t reserved1	: 1;
	uint8_t fua			: 1;
	uint8_t dpo			: 1;
	uint8_t wdprotect	: 3;
	uint8_t lba4;
	uint8_t lba3;
	uint8_t lba2;
	uint8_t lba1;
	uint8_t len4;
	uint8_t len3;
	uint8_t len2;
	uint8_t len1;
	uint8_t group_num	: 5;
	uint8_t reseved2	: 2;
	uint8_t restricted	: 1;
	uint8_t control;
} __attribute__((packed)) Write12;

typedef struct write10
{
	uint8_t opcode;
	uint8_t obsolete	: 1;
	uint8_t fua_nv		: 1;
	uint8_t reserved1	: 1;
	uint8_t fua			: 1;
	uint8_t dpo			: 1;
	uint8_t wdprotect	: 3;
	uint8_t lba4;
	uint8_t lba3;
	uint8_t lba2;
	uint8_t lba1;
	uint8_t group_num	: 5;
	uint8_t reseved2	: 3;
	uint8_t len2;
	uint8_t len1;
	uint8_t control;
} __attribute__((packed)) Write10;

typedef struct test_unit_ready
{
	uint8_t opcode;
	uint32_t reserved;
	uint8_t control;
} __attribute__((packed)) TestUnitReady;

typedef struct inquiry
{
	uint8_t opcode;
	uint8_t evpd		: 1;
	uint8_t obsolete	: 1;
	uint8_t reserved	: 6;
	uint8_t page_code;
	uint8_t alloc_len2;
	uint8_t alloc_len1;
	uint8_t control;
} __attribute__((packed)) Inquiry;

typedef struct request_sense
{
	uint8_t opcode;
	uint8_t desc;
	uint16_t reserved;
	uint8_t alloc_len;
	uint8_t control;
} __attribute__((packed)) ReqSense;

typedef struct read_capacity10
{
	uint8_t		opcode;
	uint8_t		obsolete	: 1;
	uint8_t		reserved1	: 7;
	uint32_t	lba;
	uint16_t	reserved2;
	uint8_t		pmi			: 1;
	uint8_t		reserved3	: 7;
	uint8_t		control;
} __attribute__((packed)) ReadCap10;

void create_read12_packet(void *ptr, uint32_t block, uint32_t len)
{
	Read12 *read_ptr;
	
	read_ptr = (Read12 *) ptr;
	read_ptr->opcode = READ12_OPCODE;
	read_ptr->obsolete = 0;
	read_ptr->fua_nv = 0;
	read_ptr->reserved1 = 0;
	read_ptr->fua = 0;
	read_ptr->dpo = 0;
	read_ptr->rdprotect = 0;
	
	read_ptr->lba1 = block & 0xFF;
	read_ptr->lba2 = (block >> 8) & 0xFF;
	read_ptr->lba3 = (block >> 16) & 0xFF;
	read_ptr->lba4 = (block >> 24) & 0xFF;
	read_ptr->len1 = len & 0xFF;
	read_ptr->len2 = (len >> 8) & 0xFF;
	read_ptr->len3 = (len >> 16) & 0xFF;
	read_ptr->len4 = (len >> 24) & 0xFF;
	
	read_ptr->group_num = 0;
	read_ptr->reseved2 = 0;
	read_ptr->restricted = 0;
	read_ptr->control = 0;
}

void create_write12_packet(void *ptr, uint32_t block, uint32_t len)
{
	Write12 *write_ptr;
	
	write_ptr = (Write12 *) ptr;
	write_ptr->opcode = WRITE12_OPCODE;
	write_ptr->obsolete = 0;
	write_ptr->fua_nv = 0;
	write_ptr->reserved1 = 0;
	write_ptr->fua = 0;
	write_ptr->dpo = 0;
	write_ptr->wdprotect = 0;
	
	write_ptr->lba1 = block & 0xFF;
	write_ptr->lba2 = (block >> 8) & 0xFF;
	write_ptr->lba3 = (block >> 16) & 0xFF;
	write_ptr->lba4 = (block >> 24) & 0xFF;
	write_ptr->len1 = len & 0xFF;
	write_ptr->len2 = (len >> 8) & 0xFF;
	write_ptr->len3 = (len >> 16) & 0xFF;
	write_ptr->len4 = (len >> 24) & 0xFF;
	
	write_ptr->group_num = 0;
	write_ptr->reseved2 = 0;
	write_ptr->restricted = 0;
	write_ptr->control = 0;
}

void create_write10_packet(void *ptr, uint32_t block, uint16_t len)
{
	Write10 *write_ptr;
	
	write_ptr = (Write10 *) ptr;
	write_ptr->opcode = WRITE10_OPCODE;
	write_ptr->obsolete = 0;
	write_ptr->fua_nv = 0;
	write_ptr->reserved1 = 0;
	write_ptr->fua = 0;
	write_ptr->dpo = 0;
	write_ptr->wdprotect = 0;
	
	write_ptr->lba1 = block & 0xFF;
	write_ptr->lba2 = (block >> 8) & 0xFF;
	write_ptr->lba3 = (block >> 16) & 0xFF;
	write_ptr->lba4 = (block >> 24) & 0xFF;
	write_ptr->len1 = len & 0xFF;
	write_ptr->len2 = (len >> 8) & 0xFF;
	
	write_ptr->group_num = 0;
	write_ptr->reseved2 = 0;
	write_ptr->control = 0;
}

void create_test_unit_packet(void *ptr)
{
	TestUnitReady *test_ptr;
	
	test_ptr = (TestUnitReady *) ptr;
	test_ptr->opcode = TEST_UNIT_READY_OPCODE;
	test_ptr->reserved = 0;
	test_ptr->control = 0;
}

void create_inquiry_packet(void *ptr, uint16_t alloc_len)
{
	Inquiry *inq_ptr;
	
	inq_ptr = ptr;
	inq_ptr->opcode = INQUIRY_OPCODE;
	inq_ptr->evpd = 0;
	inq_ptr->obsolete = 0;
	inq_ptr->reserved = 0;
	inq_ptr->page_code = 0;
	inq_ptr->alloc_len1 = alloc_len & 0xFF;
	inq_ptr->alloc_len2 = (alloc_len >> 8) & 0xFF;
}

void create_req_sense_packet(void *ptr, uint8_t alloc_len)
{
	ReqSense *req_ptr;
	
	req_ptr = ptr;
	req_ptr->opcode = REQUEST_SENSE_OPCODE;
	req_ptr->desc = 0;
	req_ptr->reserved = 0;
	req_ptr->alloc_len = alloc_len;
	req_ptr->control = 0;
}

void create_read_cap10_packet(void *ptr)
{
	ReadCap10 *read_ptr;
	
	read_ptr = ptr;
	read_ptr->opcode = READ_CAPACITY10_OPCODE;
	read_ptr->obsolete = 0;
	read_ptr->reserved1 = 0;
	read_ptr->lba = 0;
	read_ptr->reserved2 = 0;
	read_ptr->pmi = 0;
	read_ptr->reserved3 = 0;
	read_ptr->control = 0;
}