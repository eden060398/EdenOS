#ifndef BBB_H
#define BBB_H

#include <system.h>

// DEFINITIONS

#define CBW_SIGNATURE 0x43425355
#define CSW_SIGNATURE 0x53425355

#define TO_DEVICE  0x00
#define TO_HOST  0x80

#define DATA0  0
#define DATA1  1
#define DATA_IN  (data_in ^= 1)
#define DATA_OUT (data_out ^= 1)

#define HOST_TO_DEVICE  (0 << 7)
#define DEVICE_TO_HOST  (1 << 7)
#define TYPE_STANDARD  (0 << 5)
#define TYPE_CLASS  (1 << 5)
#define TYPE_VENDOR  (2 << 5)
#define TYPE_RESERVED  (3 << 5)
#define RECP_DEVICE  0
#define RECP_INTERFACE  1
#define RECP_ENDPOINT  2
#define RECP_OTHER  3

#define REQ_MASS_RESET  0xFF
#define REQ_GET_MAX_LUN  0xFE

#define IN   0x69
#define OUT   0xE1
#define SETUP   0x2D

#define CBW_LEN    0x1F
#define CSW_LEN    0xD

#define INQUIRY_LEN   36
#define REQUEST_SENSE_LEN 18

#define BLOCK_LEN   512

// STRUCTURES

typedef struct command_block_wrapper {
    uint32_t signature;
    uint32_t tag;
    uint32_t trans_length;
    uint8_t flags;
    uint8_t lun;
    uint8_t cmd_length;
} __attribute__((packed)) CBW;

typedef struct command_status_wrapper {
    uint32_t signature;
    uint32_t tag;
    uint32_t data_res;
    uint8_t status;
} __attribute__((packed)) CSW;

// FUNCTION DECLARATIONS

int init_bbb(UHCIDevice *dev);
int bbb_reset(UHCIDevice *dev);
uint8_t get_max_lun(UHCIDevice *dev);
int test_unit(UHCIDevice *dev);
int inquiry(UHCIDevice *dev, void *ptr);
int request_sense(UHCIDevice *dev, void *ptr);
int read_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr);
int write_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr);
int read_capacity(UHCIDevice *dev, uint32_t *cap);

#endif /* BBB_H */
