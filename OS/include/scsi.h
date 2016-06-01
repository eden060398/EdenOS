#ifndef SCSI_H
#define SCSI_H

#include <system.h>

// DEFINITIONS

#define READ12_OPCODE    0xA8
#define WRITE12_OPCODE    0xAA

// STRUCTURES

typedef struct read12 {
    uint8_t opcode;
    uint8_t obsolete : 1;
    uint8_t fua_nv : 1;
    uint8_t reserved1 : 1;
    uint8_t fua : 1;
    uint8_t dpo : 1;
    uint8_t rdprotect : 3;
    uint8_t lba4;
    uint8_t lba3;
    uint8_t lba2;
    uint8_t lba1;
    uint8_t len4;
    uint8_t len3;
    uint8_t len2;
    uint8_t len1;
    uint8_t group_num : 5;
    uint8_t reseved2 : 2;
    uint8_t restricted : 1;
    uint8_t control;
} __attribute__((packed)) Read12;

typedef struct write12 {
    uint8_t opcode;
    uint8_t obsolete : 1;
    uint8_t fua_nv : 1;
    uint8_t reserved1 : 1;
    uint8_t fua : 1;
    uint8_t dpo : 1;
    uint8_t wdprotect : 3;
    uint8_t lba4;
    uint8_t lba3;
    uint8_t lba2;
    uint8_t lba1;
    uint8_t len4;
    uint8_t len3;
    uint8_t len2;
    uint8_t len1;
    uint8_t group_num : 5;
    uint8_t reseved2 : 2;
    uint8_t restricted : 1;
    uint8_t control;
} __attribute__((packed)) Write12;

#endif /* SCSI_H */

