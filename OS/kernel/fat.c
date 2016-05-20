#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif
#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif

#define FAT32_BS_SIZE 		36
#define FAT32_BS_EXT32_SIZE	90

#define FAT12_TOTAL_CLUSTERS_LIMIT	0x00000FF5
#define FAT16_TOTAL_CLUSTERS_LIMIT	0x0000FFF5
#define FAT32_TOTAL_CLUSTERS_LIMIT	0x0FFFFFF5

typedef struct fat_ext_bs_32
{
	uint32_t	table_size_32;
	uint16_t	extended_flags;
	uint16_t	fat_version;
	uint32_t	root_cluster;
	uint16_t	fat_info;
	uint16_t	backup_BS_sector;
	uint8_t 	reserved_0[12];
	uint8_t		drive_number;
	uint8_t 	reserved_1;
	uint8_t		boot_signature;
	uint32_t 	volume_id;
	uint8_t		volume_label[11];
	uint8_t		fat_type_label[8];
}__attribute__((packed)) FATBootSectExt32;
 
typedef struct fat_bs_ext_16
{
	uint8_t		bios_drive_num;
	uint8_t		reserved1;
	uint8_t		boot_signature;
	uint32_t	volume_id;
	uint8_t		volume_label[11];
	uint8_t		fat_type_label[8];
}__attribute__((packed)) FATBootSectExt16;
 
typedef struct fat_bs
{
	uint8_t 	bootjmp[3];
	uint8_t 	oem_name[8];
	uint16_t 	bytes_per_sector;
	uint8_t		sectors_per_cluster;
	uint16_t	reserved_sector_count;
	uint8_t		table_count;
	uint16_t	root_entry_count;
	uint16_t	total_sectors_16;
	uint8_t		media_type;
	uint16_t	table_size_16;
	uint16_t	sectors_per_track;
	uint16_t	head_side_count;
	uint32_t 	hidden_sector_count;
	uint32_t 	total_sectors_32;
 
	uint8_t		extended_section[54];
}__attribute__((packed)) FATBootSect;


void 		*bs;
void 		**fats;
uint32_t	total_sectors;
uint32_t	fat_size;
uint32_t	root_dir_sectors;
uint32_t	first_data_sector;
uint32_t	first_fat_sector;
uint32_t	data_sectors;
uint32_t	total_clusters;


int init_fat(UHCIDevice *dev)
{
	int 				read;
	FATBootSect 		*fat_boot;
	FATBootSectExt16 	*fat_boot_ext16;
	FATBootSectExt32 	*fat_boot_ext32;
	
	bs = malloc(FAT32_BS_EXT32_SIZE);
	read = read_bbb(dev, 0, FAT32_BS_EXT32_SIZE, bs);
	if (read)
		return read;
	fat_boot = bs;
	fat_boot_ext16 = bs + FAT32_BS_SIZE;
	fat_boot_ext32 = bs + FAT32_BS_SIZE;
	
	total_sectors = (fat_boot->total_sectors_16) ? fat_boot->total_sectors_16 : fat_boot->total_sectors_32;
	fat_size = (fat_boot->table_size_16) ? fat_boot->table_size_16 : fat_boot_ext32->table_size_32;
	root_dir_sectors = ((fat_boot->root_entry_count * 32) + (fat_boot->bytes_per_sector - 1)) / fat_boot->bytes_per_sector;
	first_data_sector = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors;
	first_fat_sector = fat_boot->reserved_sector_count;
	data_sectors = total_sectors - (fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors);
	total_clusters = data_sectors / fat_boot->sectors_per_cluster;
	
	if (total_clusters >= FAT12_TOTAL_CLUSTERS_LIMIT ||
		(fat_boot_ext16->boot_signature != 0x28 && fat_boot_ext16->boot_signature != 0x29))
		return UNSUPPORTED;
	
	return 0;
}