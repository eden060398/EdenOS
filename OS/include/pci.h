#ifndef PCI_H
#define PCI_H

#include <system.h>

// DEFINITIONS

#define CONFIG_ADDRESS 0x0CF8
#define CONFIG_DATA  0x0CFC

#define BUS_COUNT  256
#define DEVICE_COUNT  32
#define FUNCTION_COUNT  8

#define MULTI_FUNC_BIT (1 << 7)

// ENUMERATIONS

enum CLASS_CODES {
    NONE = 0x00,
    MASS_STORAGE_CONTROLLER = 0x01,
    NETWORK_CONTROLLER = 0x02,
    DISPLAY_CONTROLLER = 0x03,
    MULTIMEDIA_CONTROLLER = 0x04,
    MEMORY_CONTROLLER = 0x05,
    BRIDGE_DEVICE = 0x06,
    SIMPLE_COMM_CONTROLS = 0x07,
    BASE_SYSTEM_PERIPHERALS = 0x08,
    INPUT_DEVICE = 0x09,
    DOCKING_STATIONS = 0x0A,
    PROCESSORS = 0x0B,
    SERIAL_BUS_CONTROLLERS = 0x0C,
    WIRELESS_CONTROLLERS = 0x0D,
    INTELLIGENT_IO_CONTROLLERS = 0x0E,
    SATELLITE_COMM_CONTROLLERS = 0x0F,
    ENCRYPT_DECRYPT_CONTROLLERS = 0x10,
    DATA_ACQ_AND_SIGNAL_PROC_CONTROLLERS = 0x11,
    UNDEFINED = 0xFF
};

enum BRIDGE_DEVICE_SUBCLASS {
    HOST_BRIDGE = 0x00,
    ISA_BRIDGE = 0x01,
    EISA_BRIDGE = 0x02,
    MCA_BRIDGE = 0x03,
    PCI_TO_PCI_BRIDGE = 0x04,
    PCMCIA_BRIDGE = 0x05,
    NUBUS_BRIDGE = 0x06,
    CARDBUS_BRIDGE = 0x07,
    RACEWAY_BRIDGE = 0x08,
    PCI_TO_PCI_BRIDGE_ = 0x09,
    INFINIBRAND_TO_PCI = 0x0A,
    OTHER = 0x80
};

// STRUCTURES

typedef struct cfg_addr {
    uint32_t zero : 2;
    uint32_t reg_num : 6;
    uint32_t func_num : 3;
    uint32_t dev_num : 5;
    uint32_t bus_num : 8;
    uint32_t reserved : 7;
    uint32_t enable : 1;
} __attribute__((packed)) ConfigAddr;

// FUNCTION DECLARATIONS

void pci_cfg_write_b(uint8_t bus_num, uint8_t dev_num, uint8_t func_num, uint8_t reg_num, uint8_t val);
void pci_cfg_write_w(uint8_t bus_num, uint8_t dev_num, uint8_t func_num, uint8_t reg_num, uint16_t val);
void pci_cfg_write_l(uint8_t bus_num, uint8_t dev_num, uint8_t func_num, uint8_t reg_num, uint32_t val);
uint32_t pci_cfg_read(uint8_t bus_num, uint8_t dev_num, uint8_t func_num, uint8_t reg_num);
uint16_t get_vendor_id(uint8_t bus, uint8_t device, uint8_t function);
uint16_t get_device_id(uint8_t bus, uint8_t device, uint8_t function);
uint8_t get_class_code(uint8_t bus, uint8_t device, uint8_t function);
uint8_t get_subclass(uint8_t bus, uint8_t device, uint8_t function);
uint8_t get_prog_if(uint8_t bus, uint8_t device, uint8_t function);
uint8_t get_header_type(uint8_t bus, uint8_t device, uint8_t function);
uint8_t get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function);
void check_bus(uint8_t bus);
void check_device(uint8_t bus, uint8_t device);
void check_function(uint8_t bus, uint8_t device, uint8_t function);
void check_all_buses(void);
void print_enum_dev(void);

#endif /* PCI_H */

