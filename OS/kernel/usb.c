#define USB_H

#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif

#define PAGE_SIZE 4096

#define USBBASE_PCI_REG 	0x20

#define USBCMD 				0x00
#define USBSTS				0x02
#define USBINTR				0x04
#define FRNUM				0x06
#define FRBASEADD			0x08
#define SOFMOD				0x0C
#define PORTSC1				0x10
#define PORTSC2				0x12

#define USBSTS_MASK			0x3F

#define PORTSC_CONNECTED			(1 << 0)
#define PORTSC_CONNECTED_CHANGED	(1 << 1)
#define PORTSC_ENABLED				(1 << 2)
#define PORTSC_ENABLED_CHANGED	(1 << 3)
#define PORTSC_RESET				(1 << 9)

#define USBCMD_RUN			(1 << 0)
#define USBCMD_HCRESET		(1 << 1)
#define USBCMD_GRESET		(1 << 2)
#define USBCMD_EGSM			(1 << 3)
#define USBCMD_FRG			(1 << 4)
#define USBCMD_SWDBG		(1 << 5)
#define USBCMD_CF			(1 << 6)
#define USBCMD_MAXP			(1 << 7)

#define IN					0x69
#define OUT					0xE1
#define SETUP				0x2D

#define SETUP_LEN			8

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
#define SETUP_CLEAR_FEATURE	1
#define SETUP_GET_CONFIG	8
#define SETUP_GET_DESC		6
#define SETUP_GET_INTERFACE	10
#define SETUP_GET_STATUS	0
#define SETUP_SET_ADDRESS	5
#define SETUP_SET_CONFIG	9
#define SETUP_SET_DESC		7
#define SETUP_SET_FEATURE	3
#define SETUP_SET_INTERFACE 11
#define SETUP_SYNCH_FRAME	12

#define LP_TERMINATE		(1 << 0)
#define LP_QH_SELECT		(1 << 1)
#define LP_DEPTH_FIRST		(1 << 2)

#define DATA0				0
#define DATA1				1

#define UHCI_PCI_LEGACY_SUPPORT         0xC0
#define UHCI_PCI_LEGACY_SUPPORT_PIRQ    0x2000
#define UHCI_PCI_LEGACY_SUPPORT_NO_CHG  0x5040
#define UHCI_PCI_LEGACY_SUPPORT_STATUS  0x8F00

#define TD_STATUS_BITSTUFF_ERR			(1 << 1)
#define TD_STATUS_CRC_TO_ERR			(1 << 2)
#define TD_STATUS_NAK					(1 << 3)
#define TD_STATUS_BABBLE				(1 << 4)
#define TD_STATUS_DATA_BUFF_ERR			(1 << 5)
#define TD_STATUS_STALLED				(1 << 6)
#define TD_STATUS_ACTIVE				(1 << 7)

#define CLASS_MASS_STORAGE	0x08
#define SUBCLASS_SCSI		0x06
#define PROTO_BBB			0x50

#define FIRST_TAG			'A'

enum USB_PROG_IF
{
	USB_UHCI	= 0x00,
	USB_OHCI	= 0x10,
	USB2		= 0x20,
	USB3		= 0x30,
	UNSPECIFIED	= 0x80,
	NONE		= 0xFE
};

enum DESC_TYPES
{
	DESC_TYPE_DEVICE = 1,
	DESC_TYPE_CONFIG,
	DESC_TYPE_STRING,
	DESC_TYPE_INTERFACE,
	DESC_TYPE_ENDPOINT,
	DESC_TYPE_DEVICE_QUALIFIER,
	DESC_TYPE_OTHER_SPEED_CONFIG,
	DESC_TYPE_INTERFACE_POWER,
	DESC_TYPE_ON_THE_GO
};

typedef struct uhci_dev
{
	uint8_t bus;
	uint8_t device;
	uint8_t function;
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t base_port;
	uint8_t maxp;
	uint8_t addr;
	uint8_t config;
	uint8_t interface;
	uint8_t out_endp;
	uint8_t out_maxp;
	uint8_t in_endp;
	uint8_t in_maxp;
	char tag;
	uint32_t capacity;
	struct uhci_dev *next;
} __attribute__((packed)) UHCIDevice;

typedef struct frame_list_ptr
{
	uint32_t terminate		: 1;
	uint32_t qh_td_select	: 1;
	uint32_t reserved		: 2;
	uint32_t ptr			:28;
} __attribute__((packed)) FLP;

typedef struct transfer_desc
{
	// TD LINK POINTER
	uint32_t link_pointer;
	
	// TD CONTROL AND STATUS
	uint32_t act_len		: 11;
	uint32_t reserved1		: 5;
	uint32_t status			: 8;
	uint32_t ioc			: 1;
	uint32_t ios			: 1;
	uint32_t low_speed		: 1;
	uint32_t err_limit		: 2;
	uint32_t spd			: 1;
	uint32_t reserved2		: 2;
	
	// TD TOKEN
	uint32_t packet_id		: 8;
	uint32_t device_addr	: 7;
	uint32_t end_point		: 4;
	uint32_t data_toggle	: 1;
	uint32_t reserved3		: 1;
	uint32_t max_len		: 11;
	
	// TD BUFFER POINTER
	uint32_t buff_ptr;
	
	// Reserved for software use
	uint32_t software1;
	uint32_t software2;
	uint32_t software3;
	uint32_t software4;
} __attribute__((packed)) TD;

typedef struct queue_head
{
	uint32_t link_pointer;
	uint32_t element_pointer;
} __attribute__((packed)) QH;

typedef struct setup_req
{
	uint8_t req_type;
	uint8_t req;
	uint16_t value;
	uint16_t index;
	uint16_t length;
} __attribute__((packed)) SETUPReq;

typedef struct device_desc
{
	uint8_t length;
	uint8_t desc_type;
	uint16_t bcd_usb;
	uint8_t class;
	uint8_t subclass;
	uint8_t proto;
	uint8_t maxp;
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t bcd_dev;
	uint8_t manu_index;
	uint8_t product_index;
	uint8_t serial_index;
	uint8_t num_config;
} __attribute__((packed)) DevDesc;

typedef struct configuration_desc
{
	uint8_t length;
	uint8_t desc_type;
	uint16_t total_len;
	uint8_t num_inter;
	uint8_t config_val;
	uint8_t config_index;
	uint8_t attributes;
	uint8_t max_power;
} __attribute__((packed)) ConfigDesc;

typedef struct interface_desc
{
	uint8_t length;
	uint8_t desc_type;
	uint8_t inter_num;
	uint8_t alt_setting;
	uint8_t num_endp;
	uint8_t class;
	uint8_t subclass;
	uint8_t proto;
	uint8_t intr_index;
} __attribute__((packed)) InterDesc;

typedef struct endpoint_desc
{
	uint8_t length;
	uint8_t desc_type;
	uint8_t addr;
	uint8_t attributes;
	uint16_t maxp;
	uint8_t interval;
} __attribute__((packed)) EndpDesc;

#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

void init_uhci(void);
void config_ehci(uint8_t bus, uint8_t device, uint8_t function);
void config_usb(uint8_t bus, uint8_t device, uint8_t function);
void config_uhci(uint8_t bus, uint8_t device, uint8_t function);
void reconfig_uhci(UHCIDevice *dev);
void reset_uhci(UHCIDevice *dev);
void supervise_uchi(void);
void insert_into_queue(UHCIDevice *dev, UHCIDevice **queue);
void remove_from_queue(UHCIDevice *dev, UHCIDevice **queue);
TD *add_td(	UHCIDevice 	*dev, 
			uint32_t 	pid,
			uint32_t 	end_point,
			uint32_t 	max_len,
			uint32_t 	buff_ptr,
			uint32_t	data_toggle,
			uint32_t	short_packet,
			QH *qh);
int run_qh(UHCIDevice *dev);
void reset_qh(void);
TD *setup_req(	UHCIDevice 	*dev,
				uint32_t 	end_point,
				uint32_t 	data_toggle,
				uint8_t 	req_type,
				uint8_t 	req, 
				uint16_t	value,
				uint16_t	index,
				uint16_t	length);
TD *input_req(UHCIDevice *dev, uint32_t end_point, uint32_t data_toggle, uint32_t length, void *data);
void *get_desc(	UHCIDevice	*dev,
				uint8_t		desc_type,
				uint8_t		desc_index,
				uint16_t	desc_length);
int set_addr(UHCIDevice *dev, uint16_t dev_addr);
int set_config(UHCIDevice *dev, uint16_t config);
int set_inter(UHCIDevice *dev, uint16_t inter);
DevDesc *get_dev_desc(UHCIDevice *dev, uint8_t dev_index);
ConfigDesc *get_config_desc(UHCIDevice *dev, uint8_t config_index);

UHCIDevice *main_dev;

UHCIDevice *main_empty;

QH *ctrl_qh, *bulk_in_qh, *bulk_out_qh;
void *frame_list, *data_page;
TD *last_td, *next_td;
int ehci_disabled, setup;
char next_tag;
uint16_t next_addr;

void init_uhci(void)
{	
	frame_list = palloc();
	data_page = palloc();
	
	reset_qh();
	
	main_dev = 0;
	next_addr = 1;
	
	next_tag = FIRST_TAG;
	setup = 1;
	ehci_disabled = 0;
	check_all_buses();
	ehci_disabled = 1;
	check_all_buses();
	setup = 0;
	/*thread = new_thread("uhci_sv", &status);
	if (thread)
		supervise_uchi();*/
}

void config_usb(uint8_t bus, uint8_t device, uint8_t function)
{
	uint8_t prog_if;
	
	if (setup)
	{
		prog_if = get_prog_if(bus, device, function);
		switch (prog_if)
		{
			case USB_UHCI:
				if (ehci_disabled)
					config_uhci(bus, device, function);
				break;
			case USB2:
				if (!ehci_disabled)
					config_ehci(bus, device, function);
				break;
		}
	}
}

void config_ehci(uint8_t bus, uint8_t device, uint8_t function)
{
	uint32_t usbbase, eecp, hcbiossem, opregs, capid;
	uint8_t reg;
	
	pci_cfg_write_w(bus, device, function, 0x04, 0x06);
	
	usbbase = 0;
	for (reg = 0x10; reg <= 0x24 && !usbbase; reg += 4)
	{
		usbbase = pci_cfg_read(bus, device, function, reg);
	}
	
	if (usbbase)
	{
		eecp = (*((uint32_t *) (usbbase + 0x08)) & 0xFF00) >> 8;
		if (eecp >= 0x40)
		{
			hcbiossem = pci_cfg_read(bus, device, function, eecp) & 0x00010000;
			if (hcbiossem)
			{
				pci_cfg_write_b(bus, device, function, eecp + 0x03, 0x01);
				wait_ticks(1024);
				capid = pci_cfg_read(bus, device, function, eecp) & 0xFF;
				if (capid == 0x01)
				{
					pci_cfg_write_w(bus, device, function, eecp + 0x04, 0x0000);
				}
			}
		}
		opregs = usbbase + (*((uint32_t *) usbbase) & 0xFF);
		*((uint32_t *) (opregs + 0x40)) = 0;
	}
}

void config_uhci(uint8_t bus, uint8_t device, uint8_t function)
{
	UHCIDevice *dev;
	DevDesc *dev_desc;
	ConfigDesc *config_desc;
	InterDesc *inter_desc;
	EndpDesc *endp_desc;
	void *next_desc;
	uint16_t sctemp;
	int i, j, b_inter, b_endp_in, b_endp_out;
	
	// Get PCI data about device
	dev = (UHCIDevice *) malloc(sizeof(UHCIDevice));
	dev->bus = bus;
	dev->device = device;
	dev->function = function;
	dev->vendor_id = get_vendor_id(bus, device, function);
	dev->device_id = get_device_id(bus, device, function);
	dev->maxp = 8;
	dev->addr = 0;
	dev->base_port = pci_cfg_read(bus, device, function, USBBASE_PCI_REG) & 0xFFE0;
	dev->tag = next_tag++;
	
	// Disable the device's Interrupts
	OUTW(dev->base_port + USBINTR, 0);
	// Get the port status
	INW(sctemp, dev->base_port + PORTSC1);
	if (!(sctemp & PORTSC_CONNECTED))
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	if ((sctemp & PORTSC_CONNECTED) && !(sctemp & PORTSC_ENABLED))
	{
		free((void *) dev);
		return;
	}
	
	// Disable the device's Interrupts
	OUTW(dev->base_port + USBINTR, 0);
	// Disable the device's Command register
	OUTW(dev->base_port + USBCMD, 0);
	// Disable the device's Status register
	OUTW(dev->base_port + USBSTS, USBSTS_MASK);
	// Wait at least 1ms (1 frame)
	wait_ticks(2);
	// Disable the legacy BIOS support
	pci_cfg_write_l(bus, device, function, UHCI_PCI_LEGACY_SUPPORT, UHCI_PCI_LEGACY_SUPPORT_STATUS);
	// reset the device
	reset_uhci(dev);
	
	dev_desc = get_dev_desc(dev, 0);
	if (!dev_desc->num_config)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	dev->config = 1;	
	dev->maxp = dev_desc->maxp;
	
	if (set_addr(dev, next_addr++) == USB_TD_ERROR)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	config_desc = get_config_desc(dev, 0);
	if (!config_desc)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	next_desc = ((void *) config_desc) + sizeof(ConfigDesc);
	b_inter = b_endp_in = b_endp_out = 0;
	for (i = 0; i < config_desc->num_inter; i++)
	{
		if (config_desc->total_len > next_desc - ((void *) config_desc))
		{
			inter_desc = next_desc;
			next_desc += sizeof(InterDesc);
			if (inter_desc->class == CLASS_MASS_STORAGE &&
				inter_desc->subclass == SUBCLASS_SCSI &&
				inter_desc->proto == PROTO_BBB)
			{
				dev->interface = inter_desc->inter_num;
				b_inter = 1;
				for (j = 0; j < inter_desc->num_endp; j++)
				{
					if (config_desc->total_len > next_desc - ((void *) config_desc))
					{
						endp_desc = next_desc;
						next_desc += sizeof(EndpDesc);						
						if (endp_desc->addr & (1 << 7))
						{
							dev->in_endp = endp_desc->addr & 0xF;
							dev->in_maxp = endp_desc->maxp;
							b_endp_in = 1;
						}
						else
						{
							dev->out_endp = endp_desc->addr & 0xF;
							dev->out_maxp = endp_desc->maxp;
							b_endp_out = 1;
						}
					}
				}
			}
			else
			{
				next_desc += inter_desc->num_endp * sizeof(EndpDesc);
			}
		}
	}
	
	if (!b_inter || !b_endp_in || !b_endp_out)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	if (set_config(dev, dev->config) == USB_TD_ERROR)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	if (set_inter(dev, dev->interface) == USB_TD_ERROR)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	if (init_bbb(dev) == USB_TD_ERROR)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	insert_into_queue(dev, &main_dev);
	
	free(dev_desc);
	free(config_desc);
}

void reconfig_uhci(UHCIDevice *dev)
{
	DevDesc *dev_desc;
	ConfigDesc *config_desc;
	InterDesc *inter_desc;
	EndpDesc *endp_desc;
	void *next_desc;
	uint16_t sctemp;
	int i, j, b_inter, b_endp_in, b_endp_out;
	
	//reset_uhci(dev);
	
	// Disable the device's Interrupts
	OUTW(dev->base_port + USBINTR, 0);
	// Get the port status
	INW(sctemp, dev->base_port + PORTSC1);
	
	if (!(sctemp & PORTSC_CONNECTED))// || !(sctemp & PORTSC_ENABLED))
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	reset_uhci(dev);
	
	dev_desc = get_dev_desc(dev, 0);
	if (!dev_desc->num_config)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	dev->config = 1;	
	dev->maxp = dev_desc->maxp;
	
	if (set_addr(dev, next_addr++) == USB_TD_ERROR)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	
	config_desc = get_config_desc(dev, 0);
	if (!config_desc)
	{
		insert_into_queue(dev, &main_empty);
		return;
	}
	next_desc = ((void *) config_desc) + sizeof(ConfigDesc);
	b_inter = b_endp_in = b_endp_out = 0;
	for (i = 0; i < config_desc->num_inter; i++)
	{
		if (config_desc->total_len > next_desc - ((void *) config_desc))
		{
			inter_desc = next_desc;
			next_desc += sizeof(InterDesc);
			if (inter_desc->class == CLASS_MASS_STORAGE &&
				inter_desc->subclass == SUBCLASS_SCSI &&
				inter_desc->proto == PROTO_BBB)
			{
				dev->interface = inter_desc->inter_num;
				b_inter = 1;
				for (j = 0; j < inter_desc->num_endp; j++)
				{
					if (config_desc->total_len > next_desc - ((void *) config_desc))
					{
						endp_desc = next_desc;
						next_desc += sizeof(EndpDesc);						
						if (endp_desc->addr & (1 << 7))
						{
							dev->in_endp = endp_desc->addr & 0xF;
							dev->in_maxp = endp_desc->maxp;
							b_endp_in = 1;
						}
						else
						{
							dev->out_endp = endp_desc->addr & 0xF;
							dev->out_maxp = endp_desc->maxp;
							b_endp_out = 1;
						}
					}
				}
			}
			else
			{
				next_desc += inter_desc->num_endp * sizeof(EndpDesc);
			}
		}
	}
	
	if (!b_inter || !b_endp_in || !b_endp_out)
	{
		//insert_into_queue(dev, &main_empty);
		return;
	}
	
	if (set_config(dev, dev->config) == USB_TD_ERROR)
	{
		//insert_into_queue(dev, &main_empty);
		return;
	}
	
	if (set_inter(dev, dev->interface) == USB_TD_ERROR)
	{
		//insert_into_queue(dev, &main_empty);
		return;
	}
	
	if (init_bbb(dev) == USB_TD_ERROR)
	{
		//insert_into_queue(dev, &main_empty);
		return;
	}
	
	//insert_into_queue(dev, &main_dev);
	
	free(dev_desc);
	free(config_desc);
}

void supervise_uchi(void)
{
	UHCIDevice *temp;
	uint16_t sctemp;
	char *buff;
	
	buff = malloc(20);
	
	set_idle();
	while (1)
	{
		putc('~');
		temp = main_dev;
		while (temp)
		{
			INW(sctemp, temp->base_port + PORTSC1);
			if (!(sctemp & PORTSC_CONNECTED))
			{
				putc('A');
				puts(uitoa(sctemp, buff, 2));
				putc('\n');
				wait_ticks(1024);/*
				OUTW(temp->base_port + PORTSC1, (1 << 9));
				wait_ticks(2048);
				OUTW(temp->base_port + PORTSC1, (1 << 2));
				INW(sctemp, temp->base_port + PORTSC1);
				puts(uitoa(sctemp, buff, 2));
				putc('\n');*/
				remove_from_queue(temp, &main_dev);
				reconfig_uhci(temp);
			}
			temp = temp->next;
		}
		
		temp = main_empty;
		while (temp)
		{
			INW(sctemp, temp->base_port + PORTSC1);
			if (sctemp & PORTSC_CONNECTED)
			{
				putc('B');
				puts(uitoa(sctemp, buff, 2));
				putc('\n');
				wait_ticks(1024);/*
				OUTW(temp->base_port + PORTSC1, (1 << 9));
				wait_ticks(2048);
				OUTW(temp->base_port + PORTSC1, (1 << 2));
				INW(sctemp, temp->base_port + PORTSC1);
				puts(uitoa(sctemp, buff, 2));
				putc('\n');*/
				remove_from_queue(temp, &main_empty);
				reconfig_uhci(temp);
			}
			temp = temp->next;
		}
		wait_ticks(128);
	}
}

void insert_into_queue(UHCIDevice *dev, UHCIDevice **queue)
{
	UHCIDevice *temp;
	
	if (*queue)
	{
		temp = *queue;
		while (temp->next)
			temp = temp->next;
		temp->next = dev;
		dev->next = 0;
	}
	else
	{
		*queue = dev;
		dev->next = 0;
	}
}

void remove_from_queue(UHCIDevice *dev, UHCIDevice **queue)
{
	UHCIDevice *temp;
	
	if (*queue)
	{
		if (*queue == dev)
		{
			*queue = dev->next;
		}
		else
		{
			temp = *queue;
			while (temp->next)
			{
				if (temp->next == dev)
				{
					temp->next = dev->next;
					break;
				}
				temp = temp->next;
			}
		}
	}
		
}

TD *add_td(	UHCIDevice 	*dev, 
			uint32_t 	pid,
			uint32_t 	end_point,
			uint32_t 	max_len,
			uint32_t 	buff_ptr,
			uint32_t	data_toggle,
			uint32_t	short_packet,
			QH *qh)
{
	TD *td;
	
	td = next_td;
	next_td++;
	if (last_td)
		last_td->link_pointer = ((uint32_t) td);
	else
		qh->element_pointer = ((uint32_t) td);
	last_td = td;
	
	td->link_pointer = LP_TERMINATE;
	
	td->act_len = 0x7FF;
	td->reserved1 = 0;
	td->status = TD_STATUS_ACTIVE;
	td->ioc = 0;
	td->ios = 0;
	td->low_speed = 0;
	td->err_limit = 0;
	td->spd = short_packet;
	td->reserved2 = 0;
	
	td->packet_id = pid;
	td->device_addr = dev->addr;
	td->end_point = end_point;
	td->data_toggle = data_toggle;
	td->reserved3 = 0;
	td->max_len = (max_len - 1) & 0x7FF;
	
	td->buff_ptr = buff_ptr;
	
	return td;
}

int run_qh(UHCIDevice *dev)
{
	uint16_t temp;
	TD *td;
	
	OUTL(dev->base_port + FRBASEADD, (uint32_t) frame_list);
	INW(temp, dev->base_port + FRNUM);
	// Zero FRNUM
	temp &= 0xF800;
	OUTW(dev->base_port + FRNUM, temp);
	INW(temp, dev->base_port + USBCMD);
	temp |= USBCMD_RUN;
	
	wait_ticks(2);
	
	OUTW(dev->base_port + USBCMD, temp);
	
	char *buff = malloc(20);
	
	uint32_t c;
	for (td = (TD *) (data_page + 3 * sizeof(TD)); td <= last_td && last_td; td++)
	{
		c = 256;
		while (td->status & TD_STATUS_ACTIVE)
		{
			wait_ticks(8);
			c--;
			if (!c)
				break;
		}
		//puts(uitoa(td->status, buff, BASE2));
		//putc(' ');
		if (td->status)
		{
			CLEAR_INTS();
			puts("\nUSB ERROR: STATUS = ");
			puts(uitoa(td->status, buff, BASE2));
			HALT();
		}
	}
	
	free(buff);
	//putc('\n');
	
	INW(temp, dev->base_port + USBCMD);
	temp &= ~USBCMD_RUN;
	
	wait_ticks(2);
	return 0;
}

void reset_qh(void)
{
	uint32_t *temp_ptr;
	int i;
	
	ctrl_qh = (QH *) (data_page);
	bulk_out_qh = (QH *) (data_page + 1 * sizeof(TD));
	bulk_in_qh = bulk_out_qh;
	last_td = 0;
	next_td = (TD *) (data_page + 2 * sizeof(TD));
	
	ctrl_qh->link_pointer = ((uint32_t) bulk_out_qh) | LP_QH_SELECT;
	ctrl_qh->element_pointer = LP_TERMINATE;
	bulk_out_qh->link_pointer = LP_TERMINATE;
	bulk_out_qh->element_pointer = LP_TERMINATE;
	
	temp_ptr = (uint32_t *) frame_list;
	for (i = 0; i < 1024; i++)
		*temp_ptr++ = ((uint32_t) ctrl_qh) | LP_QH_SELECT;
}

void reset_uhci(UHCIDevice *dev)
{
	uint16_t temp;
	
	INW(temp, dev->base_port + USBCMD);
	OUTW(dev->base_port + USBCMD, temp | 1);
	wait_ticks(256);
	OUTW(dev->base_port + PORTSC1, (1 << 9));
	wait_ticks(256);
	OUTW(dev->base_port + PORTSC1, 0);
	wait_ticks(128);
	OUTW(dev->base_port + PORTSC1, PORTSC_ENABLED_CHANGED | PORTSC_ENABLED | PORTSC_CONNECTED_CHANGED);
	wait_ticks(256);
	OUTW(dev->base_port + USBCMD, USBCMD_GRESET);
	wait_ticks(256);
	OUTW(dev->base_port + USBCMD, USBCMD_HCRESET);
	wait_ticks(256);
	OUTW(dev->base_port + USBINTR, 0);	
	OUTW(dev->base_port + PORTSC1, PORTSC_ENABLED);
	wait_ticks(256);
}

TD *setup_req(	UHCIDevice 	*dev,
				uint32_t 	end_point,
				uint32_t	data_toggle,
				uint8_t 	req_type,
				uint8_t 	req, 
				uint16_t	value,
				uint16_t	index,
				uint16_t	length)
{
	SETUPReq *setreq;
	
	setreq = (SETUPReq *) malloc(sizeof(SETUPReq));
	setreq->req_type = req_type;
	setreq->req = req;
	setreq->value = value;
	setreq->index = index;
	setreq->length = length;
	return add_td(dev, SETUP, end_point, sizeof(SETUPReq), (uint32_t) setreq, data_toggle, 0, ctrl_qh);
}

TD *input_req(UHCIDevice *dev, uint32_t end_point, uint32_t data_toggle, uint32_t length, void *data)
{
	return add_td(dev, IN, end_point, length, (uint32_t) data, data_toggle, 0, ctrl_qh);
}

void *get_desc(UHCIDevice *dev, uint8_t desc_type, uint8_t desc_index, uint16_t desc_length)
{
	void *data;
	
	setup_req(
				dev,
				0,
				DATA0,
				DEVICE_TO_HOST | TYPE_STANDARD | RECP_DEVICE,
				SETUP_GET_DESC,
				(((uint16_t) desc_type) << 8) | ((uint16_t) desc_index),
				0,
				desc_length
			);
	data = malloc(desc_length);
	input_req(dev, 0, DATA1, desc_length, data);
	add_td(dev, OUT, 0, 0, 0, DATA1, 0, ctrl_qh);
	if (run_qh(dev))
		return 0;
	reset_qh();
	return data;
}

int set_addr(UHCIDevice *dev, uint16_t dev_addr)
{
	setup_req(	dev,
				0,
				DATA0,
				HOST_TO_DEVICE | TYPE_STANDARD | RECP_DEVICE,
				SETUP_SET_ADDRESS,
				dev_addr,
				0,
				0);
	add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);
	
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	dev->addr = dev_addr;
	
	return 0;
}

int set_config(UHCIDevice *dev, uint16_t config)
{
	setup_req(	dev,
				0,
				DATA0,
				HOST_TO_DEVICE | TYPE_STANDARD | RECP_DEVICE,
				SETUP_SET_CONFIG,
				config,
				0,
				0);
	add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	reset_qh();
	
	return 0;
}

int set_inter(UHCIDevice *dev, uint16_t inter)
{
	setup_req(	dev,
				0,
				DATA0,
				HOST_TO_DEVICE | TYPE_STANDARD | RECP_INTERFACE,
				SETUP_SET_INTERFACE,
				inter,
				0,
				0);
	add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);
	if (run_qh(dev))
		return USB_TD_ERROR;
	
	reset_qh();
	return 0;
}

DevDesc *get_dev_desc(UHCIDevice *dev, uint8_t dev_index)
{
	return (DevDesc *) get_desc(dev, DESC_TYPE_DEVICE, dev_index, sizeof(DevDesc));
}

ConfigDesc *get_config_desc(UHCIDevice *dev, uint8_t config_index)
{
	return (ConfigDesc *) get_desc(dev, DESC_TYPE_CONFIG, config_index, 64);
}