#ifndef USB_H
#define USB_H

#include <system.h>

// DEFINITIONS

#define PAGE_SIZE 4096

#define USBBASE_PCI_REG  0x20

#define USBCMD     0x00
#define USBSTS    0x02
#define USBINTR    0x04
#define FRNUM    0x06
#define FRBASEADD   0x08
#define SOFMOD    0x0C
#define PORTSC1    0x10
#define PORTSC2    0x12

#define USBSTS_MASK   0x3F

#define PORTSC_CONNECTED   (1 << 0)
#define PORTSC_CONNECTED_CHANGED (1 << 1)
#define PORTSC_ENABLED    (1 << 2)
#define PORTSC_ENABLED_CHANGED (1 << 3)
#define PORTSC_RESET    (1 << 9)

#define USBCMD_RUN   (1 << 0)
#define USBCMD_HCRESET  (1 << 1)
#define USBCMD_GRESET  (1 << 2)
#define USBCMD_EGSM   (1 << 3)
#define USBCMD_FRG   (1 << 4)
#define USBCMD_SWDBG  (1 << 5)
#define USBCMD_CF   (1 << 6)
#define USBCMD_MAXP   (1 << 7)

#define IN     0x69
#define OUT     0xE1
#define SETUP    0x2D

#define SETUP_LEN   8

#define HOST_TO_DEVICE  (0 << 7)
#define DEVICE_TO_HOST  (1 << 7)
#define TYPE_STANDARD  (0 << 5)
#define TYPE_CLASS   (1 << 5)
#define TYPE_VENDOR   (2 << 5)
#define TYPE_RESERVED  (3 << 5)
#define RECP_DEVICE   0
#define RECP_INTERFACE  1
#define RECP_ENDPOINT  2
#define RECP_OTHER   3
#define SETUP_CLEAR_FEATURE 1
#define SETUP_GET_CONFIG 8
#define SETUP_GET_DESC  6
#define SETUP_GET_INTERFACE 10
#define SETUP_GET_STATUS 0
#define SETUP_SET_ADDRESS 5
#define SETUP_SET_CONFIG 9
#define SETUP_SET_DESC  7
#define SETUP_SET_FEATURE 3
#define SETUP_SET_INTERFACE 11
#define SETUP_SYNCH_FRAME 12

#define LP_TERMINATE  (1 << 0)
#define LP_QH_SELECT  (1 << 1)
#define LP_DEPTH_FIRST  (1 << 2)

#define DATA0    0
#define DATA1    1

#define UHCI_PCI_LEGACY_SUPPORT         0xC0
#define UHCI_PCI_LEGACY_SUPPORT_PIRQ    0x2000
#define UHCI_PCI_LEGACY_SUPPORT_NO_CHG  0x5040
#define UHCI_PCI_LEGACY_SUPPORT_STATUS  0x8F00

#define TD_STATUS_BITSTUFF_ERR   (1 << 1)
#define TD_STATUS_CRC_TO_ERR   (1 << 2)
#define TD_STATUS_NAK     (1 << 3)
#define TD_STATUS_BABBLE    (1 << 4)
#define TD_STATUS_DATA_BUFF_ERR   (1 << 5)
#define TD_STATUS_STALLED    (1 << 6)
#define TD_STATUS_ACTIVE    (1 << 7)

#define CLASS_MASS_STORAGE 0x08
#define SUBCLASS_SCSI  0x06
#define PROTO_BBB   0x50

#define FIRST_TAG   'A'

// ENUMERATIONS

enum USB_PROG_IF {
    USB_UHCI = 0x00,
    USB_OHCI = 0x10,
    USB2 = 0x20,
    USB3 = 0x30,
    UNSPECIFIED = 0x80,
    NONE = 0xFE
};

enum DESC_TYPES {
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

// STRUCTURES

typedef struct frame_list_ptr {
    uint32_t terminate : 1;
    uint32_t qh_td_select : 1;
    uint32_t reserved : 2;
    uint32_t ptr : 28;
} __attribute__((packed)) FLP;

typedef struct transfer_desc {
    // TD LINK POINTER
    uint32_t link_pointer;

    // TD CONTROL AND STATUS
    uint32_t act_len : 11;
    uint32_t reserved1 : 5;
    uint32_t status : 8;
    uint32_t ioc : 1;
    uint32_t ios : 1;
    uint32_t low_speed : 1;
    uint32_t err_limit : 2;
    uint32_t spd : 1;
    uint32_t reserved2 : 2;

    // TD TOKEN
    uint32_t packet_id : 8;
    uint32_t device_addr : 7;
    uint32_t end_point : 4;
    uint32_t data_toggle : 1;
    uint32_t reserved3 : 1;
    uint32_t max_len : 11;

    // TD BUFFER POINTER
    uint32_t buff_ptr;

    // Reserved for software use
    uint32_t software1;
    uint32_t software2;
    uint32_t software3;
    uint32_t software4;
} __attribute__((packed)) TD;

typedef struct queue_head {
    uint32_t link_pointer;
    uint32_t element_pointer;
} __attribute__((packed)) QH;

typedef struct setup_req {
    uint8_t req_type;
    uint8_t req;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) SETUPReq;

typedef struct device_desc {
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

typedef struct configuration_desc {
    uint8_t length;
    uint8_t desc_type;
    uint16_t total_len;
    uint8_t num_inter;
    uint8_t config_val;
    uint8_t config_index;
    uint8_t attributes;
    uint8_t max_power;
} __attribute__((packed)) ConfigDesc;

typedef struct interface_desc {
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

typedef struct endpoint_desc {
    uint8_t length;
    uint8_t desc_type;
    uint8_t addr;
    uint8_t attributes;
    uint16_t maxp;
    uint8_t interval;
} __attribute__((packed)) EndpDesc;

// FUNCTION DECLARATIONS

void init_uhci(void);
void config_ehci(uint8_t bus, uint8_t device, uint8_t function);
void config_usb(uint8_t bus, uint8_t device, uint8_t function);
void config_uhci(uint8_t bus, uint8_t device, uint8_t function);
void reset_uhci(UHCIDevice *dev);
TD *add_td(UHCIDevice *dev,
        uint32_t pid,
        uint32_t end_point,
        uint32_t max_len,
        uint32_t buff_ptr,
        uint32_t data_toggle,
        uint32_t short_packet,
        QH *qh);
int run_qh(UHCIDevice *dev);
void reset_qh(void);
TD *setup_req(UHCIDevice *dev,
        uint32_t end_point,
        uint32_t data_toggle,
        uint8_t req_type,
        uint8_t req,
        uint16_t value,
        uint16_t index,
        uint16_t length);
TD *input_req(UHCIDevice *dev, uint32_t end_point, uint32_t data_toggle, uint32_t length, void *data);
void *get_desc(UHCIDevice *dev,
        uint8_t desc_type,
        uint8_t desc_index,
        uint16_t desc_length);
int set_addr(UHCIDevice *dev, uint16_t dev_addr);
int set_config(UHCIDevice *dev, uint16_t config);
int set_inter(UHCIDevice *dev, uint16_t inter);
DevDesc *get_dev_desc(UHCIDevice *dev, uint8_t dev_index);
ConfigDesc *get_config_desc(UHCIDevice *dev, uint8_t config_index);

#endif /* USB_H */

