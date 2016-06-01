#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// DEFINITIONS

#define TAB 4

#define INB(ret, port) asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port))
#define OUTB(port, val) asm volatile ("outb %%al,%%dx": :"d" (port), "a" (val))
#define INW(ret, port) asm volatile ("inw %%dx,%%ax":"=a" (ret):"d" (port))
#define OUTW(port, val) asm volatile ("outw %%ax,%%dx": :"d" (port), "a" (val))
#define INL(ret, port) asm volatile ("inl %%dx,%%eax":"=a" (ret):"d" (port))
#define OUTL(port, val) asm volatile ("outl %%eax,%%dx": :"d" (port), "a" (val))
#define CLEAR_INTS() asm volatile ("cli")
#define SET_INTS() asm volatile ("sti")
#define HALT() asm volatile ("hlt")
#define PUSH(x) asm ("pushl %%eax": :"a" (x))
#define POP(x) asm ("popl %%eax" : "=a" (x))

// ENUMERATIONS

enum ERR_TYPE {
    KERNEL_INTERNAL_PARAMS = 0x01,
    UNSUPPORTED = 0x02,
    USB_TD_ERROR = 0x10
};

enum BASE {
    BASE2 = 2,
    BASE10 = 10,
    BASE16 = 16
};

enum COLOR {
    BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GRAY, DARK_GRAY, LIGHT_BLUE,
    LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED, LIGHT_MAGENTA, YELLOW, WHITE
};

// DECLARATIONS

const char *TITLE;

char *working_dir;

// STRUCTURES

typedef struct uhci_dev {
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

// MODULES

#ifndef INTERRUPTS32_H
void init_interrupts(void);
#endif

#ifndef IDT_H
void init_idt(void);
#endif

#ifndef STRING_H
void *memset(void *str, int c, uint32_t n);
char *itoa(int n, char *buff, int base);
char *uitoa(uint32_t n, char *buff, int base);
int strcmp(const char *s1, const char *s2);
int memcmp(const char *s1, const char *s2, uint32_t n);
void *memcpy(void *dest, void *src, uint32_t n);
char *strcpy(char *dest, const char *src);
uint32_t strlen(const char *s);
uint32_t atoui(char *buff);
int startswith(char *s1, char *s2);
#endif

#ifndef CONSOLE_H
void init_console(void);
void set_cursor(int row, int column);
void putc(int);
void puts(const char*);
void putc_at(int c, int row, int column);
void puts_at(const char *s, int row, int column);
void clear_screen(void);
uint32_t get_current(void);
void set_color(uint8_t fore, uint8_t back);
#endif

#ifndef KEYBOARD_H
void init_keyboard(void);
void handle_keystroke(void);
int getc(void);
char *gets(char *buff, int max);
void init_time(void);
void handle_tick(void);
void init_switch(unsigned int count);
void wait_ticks(int n);
#endif

#ifndef CMD_H
void init_command_prompt(void);
#endif

#ifndef MEMORY_H
void init_palloc(void);
void print_bios_mmap(void);
void *palloc(void);
void pfree(void *ptr);
#endif

#ifndef DMEMORY_H
void init_malloc(void);
void *malloc(size_t size);
void free(void *ptr);
#endif

#ifndef PAGING_H
void init_paging(void);
void create_entry(uint32_t dir_addr, uint32_t laddr, uint32_t paddr, int read_write, int user_supervisor, uint8_t flags);
void remove_entry(uint32_t dir_addr, uint32_t laddr);
uint32_t dup_dir(uint32_t new_dir, uint32_t ori_dir);
#endif

#ifndef PROCESSING_H
void init_processing(void);
unsigned int switch_proc(void);
int new_thread(const char *name, uint32_t **status);
void dispose_thread(void);
void wait(uint32_t *status);
void print_proc_data(int with_pid, int with_ppid, int with_status, int with_threads, int with_tid, int with_tstatus);
void set_idle(void);
void set_active(void);
void kill_th(uint32_t pid, uint32_t tid);
#endif

#ifndef PCI_H
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
#endif

#ifndef USB_H
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

void init_uhci(void);
void config_usb(uint8_t bus, uint8_t device, uint8_t function);
void config_uhci(uint8_t bus, uint8_t device, uint8_t function);
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

QH *ctrl_qh, *bulk_in_qh, *bulk_out_qh;
#endif

#ifndef BBB_H
int init_bbb(UHCIDevice *dev);
int bbb_reset(UHCIDevice *dev);
int read_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr);
int write_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr);
#endif

#ifndef SCSI_H
void create_read12_packet(void *ptr, uint32_t block, uint32_t len);
void create_write12_packet(void *ptr, uint32_t block, uint32_t len);
#endif

#ifndef EDENFS_H
typedef struct file {
    uint32_t base_lba;
    uint32_t offset;
    uint32_t r_seek;
    uint32_t w_seek;
} __attribute__((packed)) File;
void init_fs(UHCIDevice *dev);
File *open(char *path, uint32_t len, char mode);
void list(char *path, uint32_t len, int tree, int size);
uint32_t get_size(uint32_t part_lba);
int create(char *path, uint32_t len, int target_type);
void delete(char *path, uint32_t len);
uint32_t read_from_file(File *f, uint32_t count, char *data);
void write_to_file(File *f, char *data, uint32_t count);
int is_path(char *path, uint32_t len, int target_type, int not_empty);
char *join_path(char *first, char *second);
char *get_full_path(char *s);
#endif

#endif /* SYSTEM_H */
