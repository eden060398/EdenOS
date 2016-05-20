#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define CBW_SIGNATURE 0x43425355
#define CSW_SIGNATURE 0x53425355

#define TO_DEVICE 	0x00
#define TO_HOST 	0x80

#define DATA0		0
#define DATA1		1
#define DATA_IN		(data_in ^= 1)
#define DATA_OUT	(data_out ^= 1)

#define HOST_TO_DEVICE		(0 << 7)
#define DEVICE_TO_HOST		(1 << 7)
#define TYPE_STANDARD		(0 << 5)
#define TYPE_CLASS			(1 << 5)
#define TYPE_VENDOR			(2 << 5)
#define TYPE_RESERVED		(3 << 5)
#define RECP_DEVICE			0
#define RECP_INTERFACE		1
#define RECP_ENDPOINT		2
#define RECP_OTHER			3

#define REQ_MASS_RESET		0xFF
#define REQ_GET_MAX_LUN		0xFE

#define IN					0x69
#define OUT					0xE1
#define SETUP				0x2D

#define CBW_LEN				0x1F
#define CSW_LEN				0xD

#define INQUIRY_LEN			36
#define REQUEST_SENSE_LEN	18

#define BLOCK_LEN			512

typedef struct command_block_wrapper
{
	uint32_t 	signature;
	uint32_t 	tag;
	uint32_t 	trans_length;
	uint8_t 	flags;
	uint8_t 	lun;
	uint8_t		cmd_length;
} __attribute__((packed)) CBW;

typedef struct command_status_wrapper
{
	uint32_t 	signature;
	uint32_t 	tag;
	uint32_t	data_res;
	uint8_t		status;
} __attribute__((packed)) CSW;

int init_bbb(UHCIDevice *dev);
int bbb_reset(UHCIDevice *dev);
uint8_t get_max_lun(UHCIDevice *dev);
int test_unit(UHCIDevice *dev);
int inquiry(UHCIDevice *dev, void *ptr);
int request_sense(UHCIDevice *dev, void *ptr);
int read_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr);
int write_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr);
//int write_bbb(UHCIDevice *dev, uint32_t block, uint16_t count, void *ptr);
int read_capacity(UHCIDevice *dev, uint32_t *cap);
uint32_t tag;

uint32_t data_in, data_out;

int init_bbb(UHCIDevice *dev)
{
	data_in = 1;
	data_out = 1;
	tag = 0;
	
	if (bbb_reset(dev) == USB_TD_ERROR)
		return USB_TD_ERROR;	
	
	/*int temp = test_unit(dev);
	char *buff = malloc(10);
	puts(itoa(temp, buff, 10));
	putc('\n');
	if (temp == USB_TD_ERROR)
		return USB_TD_ERROR;
	
	void *inq = malloc(INQUIRY_LEN);
	temp = inquiry(dev, inq);
	puts(itoa(temp, buff, 10));
	putc('\n');
	if (temp == USB_TD_ERROR)
		return USB_TD_ERROR;
	free(inq);*/
	
	//if (read_capacity(dev, (void *) &dev->capacity) == USB_TD_ERROR)
	//	return USB_TD_ERROR;
	
	init_fs(dev);
	
	return 0;
}

int bbb_reset(UHCIDevice *dev)
{
	setup_req(	dev,
				0,
				DATA0,
				HOST_TO_DEVICE | TYPE_CLASS | RECP_INTERFACE,
				REQ_MASS_RESET,
				0,
				dev->interface,
				0);
	add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	return 0;
}

uint8_t get_max_lun(UHCIDevice *dev)
{
	uint8_t lun;
	
	setup_req(	dev,
				0,
				DATA0,
				DEVICE_TO_HOST | TYPE_CLASS | RECP_INTERFACE,
				REQ_GET_MAX_LUN,
				0,
				dev->interface,
				1
				);
	input_req(dev, 0, DATA1, 1, (void *) &lun);
	add_td(dev, OUT, 0, 0, 0, DATA1, 0, ctrl_qh);
	run_qh(dev);
	reset_qh();
	
	return lun;
}

int test_unit(UHCIDevice *dev)
{
	void *cbw_ptr, *csw_ptr;
	CBW *cbw;
	CSW *csw;
	
	cbw_ptr = malloc(CBW_LEN);
	csw_ptr = malloc(CSW_LEN);
	
	cbw = cbw_ptr;
	cbw->signature = CBW_SIGNATURE;
	cbw->tag = 0xAABBCCDD;
	cbw->trans_length = 0;
	cbw->flags = 0;
	cbw->lun = 0;
	cbw->cmd_length = 6;
	create_test_unit_packet(cbw_ptr + sizeof(CBW));
	
	add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);
	add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	csw = csw_ptr;
	return csw->status;
}

int inquiry(UHCIDevice *dev, void *ptr)
{
	void *cbw_ptr, *csw_ptr;
	CBW *cbw;
	CSW *csw;
	
	cbw_ptr = (CBW *) malloc(CBW_LEN);
	csw_ptr = (CSW *) malloc(CSW_LEN);
	
	cbw = cbw_ptr;
	cbw->signature = CBW_SIGNATURE;
	cbw->tag = 0xAABBCCDD;
	cbw->trans_length = INQUIRY_LEN;
	cbw->flags = TO_HOST;
	cbw->lun = 0;
	cbw->cmd_length = 6;
	create_inquiry_packet(cbw_ptr + sizeof(CBW), INQUIRY_LEN);
	
	add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);
	add_td(dev, IN, dev->in_endp, INQUIRY_LEN, (uint32_t) ptr, DATA_IN, 0, bulk_in_qh);
	add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	csw = csw_ptr;
	return csw->status;
}

int request_sense(UHCIDevice *dev, void *ptr)
{
	void *cbw_ptr, *csw_ptr;
	CBW *cbw;
	
	cbw_ptr = (CBW *) malloc(CBW_LEN);
	csw_ptr = (CSW *) malloc(CSW_LEN);
	
	cbw = cbw_ptr;
	cbw->signature = CBW_SIGNATURE;
	cbw->tag = 0xAABBCCDD;
	cbw->trans_length = REQUEST_SENSE_LEN;
	cbw->flags = TO_HOST;
	cbw->lun = 0;
	cbw->cmd_length = 6;
	create_req_sense_packet(cbw_ptr + sizeof(CBW), REQUEST_SENSE_LEN);
	
	add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);
	add_td(dev, IN, dev->in_endp, REQUEST_SENSE_LEN, (uint32_t) ptr, DATA_IN, 0, bulk_in_qh);
	add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	return 0;
}

int read_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr)
{
	void		*cbw_ptr;
	void		*csw_ptr;
	uint32_t	data_addr;
	uint32_t	i;
	CBW 		*cbw;
	CSW 		*csw;
	int 		result;
	
	cbw_ptr = malloc(CBW_LEN);
	csw_ptr = malloc(CSW_LEN);
	
	cbw = cbw_ptr;
	cbw->signature = CBW_SIGNATURE;
	cbw->tag = tag++;
	cbw->trans_length = BLOCK_LEN * count;
	cbw->flags = TO_HOST;
	cbw->lun = 0;
	cbw->cmd_length = 12;
	create_read12_packet(cbw_ptr + sizeof(CBW), block, count);
	
	data_addr = (uint32_t) ptr;
	
	add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);
	
	for (i = 0; i < count * (BLOCK_LEN / dev->in_maxp); i++)
	{
		add_td(dev, IN, dev->in_endp, dev->in_maxp, data_addr, DATA_IN, 0, bulk_in_qh);
		data_addr += dev->in_maxp;
	}
	
	add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);
	
	if (run_qh(dev))
	{
		return USB_TD_ERROR;
	}
	reset_qh();
	
	csw = csw_ptr;
	result = csw->status;

	free(cbw_ptr);
	free(csw_ptr);
	
	return result;
}

int write_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr)
{
	void		*cbw_ptr;
	void		*csw_ptr;
	uint32_t	data_addr;
	uint32_t	i;
	CBW 		*cbw;
	CSW 		*csw;
	int 		result;
	
	cbw_ptr = malloc(CBW_LEN);
	csw_ptr = malloc(CSW_LEN);
	
	cbw = cbw_ptr;
	cbw->signature = CBW_SIGNATURE;
	cbw->tag = tag++;
	cbw->trans_length = BLOCK_LEN * count;
	cbw->flags = TO_DEVICE;
	cbw->lun = 0;
	cbw->cmd_length = 12;
	create_write12_packet(cbw_ptr + sizeof(CBW), block, count);
	
	data_addr = (uint32_t) ptr;
	
	add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);
	
	for (i = 0; i < count * (BLOCK_LEN / dev->out_maxp); i++)
	{
		add_td(dev, OUT, dev->out_endp,  dev->out_maxp, data_addr, DATA_OUT, 0, bulk_out_qh);
		data_addr += dev->out_maxp;
	}
	
	add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);	
	
	if (run_qh(dev))
	{
		return USB_TD_ERROR;
	}
	reset_qh();
	
	csw = csw_ptr;
	result = csw->status;

	free(cbw_ptr);
	free(csw_ptr);
	
	return result;
}

/*int write_bbb(UHCIDevice *dev, uint32_t block, uint16_t count, void *ptr)
{
	void		*cbw_ptr;
	void		*csw_ptr;
	uint32_t	data_addr;
	uint32_t	written_bytes;
	uint32_t	i;
	CBW 		*cbw;
	CSW 		*csw;
	TD 			*td;
	int 		result;
	
	cbw_ptr = malloc(CBW_LEN);
	csw_ptr = malloc(CSW_LEN);
	
	cbw = cbw_ptr;
	cbw->signature = CBW_SIGNATURE;
	cbw->tag = 0xAABBCCDD;
	cbw->trans_length = BLOCK_LEN * count;
	cbw->flags = TO_DEVICE;
	cbw->lun = 0;
	cbw->cmd_length = 10;
	create_write10_packet(cbw_ptr + sizeof(CBW), block, count);
	
	add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0);
	if (run_qh(dev))
		return USB_TD_ERROR;
	
	data_addr = (uint32_t) ptr;
	for (i = 0; i < count; i++)
	{
		written_bytes = 0;
		do
		{
			reset_qh();
			td = add_td(dev, OUT, dev->out_endp, BLOCK_LEN - written_bytes, data_addr + written_bytes, DATA_OUT, 0);
			if (run_qh(dev))
				return USB_TD_ERROR;
			written_bytes += (td->act_len + 1) & 0x3FF;
		} while (written_bytes < BLOCK_LEN);
		reset_qh();
		data_addr += BLOCK_LEN;
	}
	putc('A');
	add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0);
	
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	csw = csw_ptr;
	result = csw->status;

	free(cbw_ptr);
	free(csw_ptr);
	
	return result;
}*/

int read_capacity(UHCIDevice *dev, uint32_t *cap)
{
	void		*cbw_ptr;
	void		*csw_ptr;
	CBW 		*cbw;
	CSW 		*csw;
	TD 			*td;
	int 		result;
	void		*data;
	uint32_t	data_addr;
	uint32_t	written_bytes;
	
	cbw_ptr = malloc(CBW_LEN);
	csw_ptr = malloc(CSW_LEN);
	data = malloc(8);
	*((uint32_t *) data) = 0;
	*((uint32_t *) (data + 4)) = 0;
	
	cbw = cbw_ptr;
	cbw->signature = CBW_SIGNATURE;
	cbw->tag = 0xAABBCCDD;
	cbw->trans_length = 8;
	cbw->flags = TO_HOST;
	cbw->lun = 0;
	cbw->cmd_length = 10;
	create_read_cap10_packet(cbw_ptr + sizeof(CBW));
	
	add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	
	data_addr = (uint32_t) &data;
	written_bytes = 0;
	do
	{
		reset_qh();
		td = add_td(dev, IN, dev->in_endp, 8 - written_bytes, data_addr + written_bytes, DATA_IN, 0, bulk_in_qh);
		if (run_qh(dev))
				return USB_TD_ERROR;
		written_bytes += (td->act_len + 1) & 0x3FF;
	} while (written_bytes < 8);
	reset_qh();
	
	add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	csw = csw_ptr;
	result = csw->status;
	
	if (!result)
		*cap = *((uint32_t *) data);
	
	char *buff = malloc(50);
	for (int i = 0; i < 8; i++)
	{
		puts(uitoa(*((uint8_t *) (data + i)), buff, 16));
		putc(' ');
	}
	putc('\n');
	for (int i = 0; i < CSW_LEN; i++)
	{
		puts(uitoa(*((uint8_t *) (csw_ptr + i)), buff, 16));
		putc(' ');
	}
	putc('\n');
	puts(uitoa(*((uint32_t *) data), buff, 10));
	putc('\n');
	puts(uitoa(*((uint32_t *) (data + 4)), buff, 10));
	putc('\n');
	puts(uitoa(*((uint32_t *) data), buff, 16));
	putc('\n');
	puts(uitoa(*((uint32_t *) (data + 4)), buff, 16));
	putc('\n');
	free(buff);

	free(cbw_ptr);
	free(csw_ptr);
	free(data);
	
	return result;
}