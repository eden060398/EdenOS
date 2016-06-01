#ifndef EDENFS_H
#define EDENFS_H

#include <system.h>

// DEFINITIONS

#define EDENFS_SIGN_LEN  9
#define BITMAP_SIZE   4096

#define BLOCK_SIZE   512

#define PRESENT    (1 << 0)
#define IS_DIR    (1 << 1)
#define NOT_EMPTY    (1 << 2) 

#define NAME_LEN    11
#define DIR_ENTRIES_PER_PART 31
#define FILE_DATA_PER_PART  506

#define TYPE_UNDEFINED   0
#define TYPE_DIR    1
#define TYPE_FILE    2

// STRUCTURES

typedef struct dir_entry {
    char name[NAME_LEN + 1];
    uint32_t addr;
} __attribute__((packed)) DirEntry;

typedef struct dir_part {
    uint16_t part_size;
    DirEntry entry[31];
    uint8_t reserved[10];
    uint32_t next_part;
} __attribute__((packed)) DirPart;

typedef struct file_part {
    uint16_t part_size;
    uint8_t data[FILE_DATA_PER_PART];
    uint32_t next_part;
} __attribute__((packed)) FilePart;

typedef struct boot_sect {
    uint8_t boot_code[488];
    uint32_t block_count;
    char sign[EDENFS_SIGN_LEN];
    uint8_t levels;
    uint32_t first_sect;
    uint32_t first_bitmap;
    uint16_t boot_sign;
} __attribute__((packed)) BootSect;

typedef struct level_node {
    uint32_t level_size;
    struct level_node *next;
} __attribute__((packed)) LevelNode;

typedef struct path_part {
    char name[NAME_LEN + 1];
    struct path_part *next;
} __attribute__((packed)) PathPart;

typedef struct file {
    uint32_t base_lba;
    uint32_t offset;
    uint32_t r_seek;
    uint32_t w_seek;
} __attribute__((packed)) File;

// FUNCTION DECLARATIONS

void init_fs(UHCIDevice *dev);
File *open(char *path, uint32_t len, char mode);
void list(char *path, uint32_t len, int tree, int size);
void _list(uint32_t lba, int tree, int size, uint32_t level);
uint32_t get_size(uint32_t part_lba);
int create(char *path, uint32_t len, int target_type);
void delete(char *path, uint32_t len);
uint32_t read_from_file(File *f, uint32_t count,
        char *data);
void write_to_file(File *f, char *data, uint32_t count);

int is_path(char *path, uint32_t len, int target_type, int not_empty);
int find_path(char *path, uint32_t len, int target_type, int not_empty,
        uint32_t *base_lba, uint32_t *offset);
int find_in_dir(uint32_t cur_lba, const char *name, uint32_t len, int target_type, int not_empty,
        uint32_t *base_lba, uint32_t *offset);
PathPart *to_path_parts(char *path, uint32_t len);
void free_path_parts(PathPart *first);

void find_empty_entry(uint32_t dir_lba,
        uint32_t *base_lba, uint32_t *offset);
void delete_chain_parts(uint32_t lba);
uint32_t balloc(void);
void bfree(uint32_t lba);
uint32_t find_in_level(uint8_t level, uint32_t base_lba, uint32_t offset, LevelNode *level_node);

char *join_path(char *first, char *second);
char *get_full_path(char *s);

#endif /* EDENFS_H */

