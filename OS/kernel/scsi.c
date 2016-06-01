// -----------------------------------------------------------------------------
// SCSI Module
// -----------
// 
// General      :   The module provides an interface to create SCSI packets.
//
// Input        :   None
//
// Process      :   Create SCSI Command Blocks when requested.
//
// Output       :   None
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------

#include <scsi.h>

void create_read12_packet(void *ptr, uint32_t block, uint32_t len) {
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

void create_write12_packet(void *ptr, uint32_t block, uint32_t len) {
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