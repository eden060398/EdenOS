#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define EDENFS_SIGN_LEN 	9
#define BITMAP_SIZE 		4096

#define BLOCK_SIZE			512

#define PRESENT				(1 << 0)
#define	IS_DIR				(1 << 1)
#define NOT_EMPTY				(1 << 2) 

#define NAME_LEN				11
#define DIR_ENTRIES_PER_PART	31
#define FILE_DATA_PER_PART		506

#define TYPE_UNDEFINED			0
#define TYPE_DIR				1
#define TYPE_FILE				2


typedef struct dir_entry
{
	char name[NAME_LEN + 1];
	uint32_t addr;
} __attribute__((packed)) DirEntry;

typedef struct dir_part
{
	uint16_t part_size;
	DirEntry entry[31];
	uint8_t reserved[10];
	uint32_t next_part;
} __attribute__((packed)) DirPart;

typedef struct file_part
{
	uint16_t part_size;
	uint8_t data[FILE_DATA_PER_PART];
	uint32_t next_part;
} __attribute__((packed)) FilePart;

typedef struct boot_sect
{
	uint8_t boot_code[488];
	uint32_t block_count;
	char sign[EDENFS_SIGN_LEN];
	uint8_t levels;
	uint32_t first_sect;
	uint32_t first_bitmap;
	uint16_t boot_sign;
} __attribute__((packed)) BootSect;

typedef struct level_node
{
	uint32_t level_size;
	struct level_node *next;
} __attribute__((packed)) LevelNode;

typedef struct path_part
{
	char name[NAME_LEN + 1];
	struct path_part *next;
} __attribute__((packed)) PathPart;

void init_fs(UHCIDevice *dev);
File *open(char *path, uint32_t len, char mode);
void list(char *path, uint32_t len, int tree, int size);
void _list(uint32_t lba, int tree, int size, uint32_t level);
uint32_t get_size(uint32_t part_lba, int is_dir);
int create(char *path, uint32_t len, int target_type);
void delete(char *path, uint32_t len);
uint32_t read_from_file(File *f, uint32_t count,
						char *data);
void write_to_file(File *f, char *data, uint32_t count);

int is_path(char *path, uint32_t len, int target_type, int not_empty);
int find_path(	char *path, uint32_t len, int target_type, int not_empty,
				uint32_t *base_lba, uint32_t *offset);
int find_in_dir(uint32_t cur_lba, const char *name, uint32_t len, int target_type, int not_empty,
				uint32_t *base_lba, uint32_t *offset);
PathPart *to_path_parts(char *path, uint32_t len);
void free_path_parts(PathPart *first);

void find_empty_entry(	uint32_t dir_lba,
						uint32_t *base_lba, uint32_t *offset);
void delete_chain_parts(uint32_t lba);
uint32_t balloc();
void bfree(uint32_t lba);
uint32_t find_in_level(uint8_t level, uint32_t base_lba, uint32_t offset, LevelNode *level_node);

UHCIDevice *fs_dev;
uint32_t block_count;
uint8_t levels;
LevelNode *first_level;
uint32_t root_lba;
uint32_t first_bitmap_lba;

uint32_t total_bitmaps;

void init_fs(UHCIDevice *dev)
{
	BootSect *bsect;
	uint32_t temp;
	uint8_t i;
	LevelNode *level_node, *temp_level_node;
	
	bsect = (BootSect *) malloc(BLOCK_SIZE);
	
	read_bbb(dev, 0, 1, bsect);
	if (!memcmp(bsect->sign, "EDENFS100", EDENFS_SIGN_LEN))
		return;
	fs_dev = dev;
	block_count = bsect->block_count;
	levels = bsect->levels;
	first_bitmap_lba = bsect->first_bitmap;
	root_lba = bsect->first_sect;
	
	free((void *) bsect);
	
	fs_dev->capacity = block_count * BLOCK_SIZE;
	
	temp = block_count;
	level_node = 0;
	for (i = 0; i < levels; i++)
	{
		temp_level_node = (LevelNode *) malloc(sizeof(LevelNode));
		temp_level_node->next = level_node;
		level_node = temp_level_node;
		level_node->level_size = temp / BITMAP_SIZE;
		temp = level_node->level_size;
	}
	first_level = level_node;
	
	working_dir = (char *) malloc(2);
	*working_dir = '/';
}

File *open(char *path, uint32_t len, char mode)
{
	uint32_t base_lba, offset;
	int status;
	File *f;
	
	if (mode == 'w')
	{
		create(path, len, TYPE_FILE);
	}
	status = find_path(	path, len, TYPE_FILE, 0,
						&base_lba, &offset);
	if (status)
		return 0;
	
	f = (File *) malloc(sizeof(File));
	f->base_lba = base_lba;
	f->offset = offset;
	f->r_seek = 0;
	f->w_seek = 0;
	
	return f;
}

void list(char *path, uint32_t len, int tree, int size)
{
	uint32_t base_lba, offset, lba;
	int status;
	DirPart *dir;
	
	if ((len == 1 && path[0] == '/') || len == 0)
		lba = root_lba;
	else
	{
		status = find_path(path, len, TYPE_DIR, 0, &base_lba, &offset);
		if (status)
		{
			puts("Path not found!\n");
			return;
		}
		dir = (DirPart *) malloc(sizeof(DirPart));
		read_bbb(fs_dev, base_lba, 1, dir);
		if (!(dir->entry[offset].addr & NOT_EMPTY))
			return;
		lba = dir->entry[offset].addr >> 9;
	}
	_list(lba, tree, size, 0);
}

void _list(uint32_t lba, int tree, int size, uint32_t level)
{
	uint32_t i, c, j;
	DirPart *dir;
	char *buff;
	
	dir = (DirPart *) malloc(sizeof(DirPart));
	while (1)
	{
		read_bbb(fs_dev, lba, 1, (void *) dir);
		c = 0;
		for (i = 0; i < DIR_ENTRIES_PER_PART; i++)
		{
			if (c >= dir->part_size)
				break;
			if (dir->entry[i].addr & PRESENT)
			{
				c++;
				if (level)
				{
					for (j = 0; j < level - 1; j++)
						puts("    |");
					puts("    +--- ");
				}
				puts(dir->entry[i].name);
				if (dir->entry[i].addr & IS_DIR)
					puts(" (DIR)");
				else
					puts(" (FILE)");
				if (size)
				{
					if (dir->entry[i].addr & NOT_EMPTY)
					{
						buff = (char *) malloc(100);
						puts(" (");
						puts(uitoa(get_size(dir->entry[i].addr >> 9, dir->entry[i].addr & IS_DIR), buff, BASE10));
						puts(" bytes)");
						free((void *) buff);
					}
					else
						puts(" (0 bytes)");
				}
				putc('\n');
				if (tree && (dir->entry[i].addr & NOT_EMPTY) && (dir->entry[i].addr & IS_DIR))
				{
					for (j = 0; j < level + 1; j++)
						puts("    |");
					putc('\n');
					_list(dir->entry[i].addr >> 9, tree, size, level + 1);
				}
			}
		}
		if (!(dir->next_part & PRESENT) || !(dir->next_part & NOT_EMPTY))
		{
			free((void *) dir);
			return;
		}
		lba = dir->next_part >> 9;
	}
}

uint32_t get_size(uint32_t part_lba, int is_dir)
{
	uint32_t size, i, c;
	DirPart *dir;
	FilePart *part;
	
	size = 0;
	
	if (is_dir)
	{
		dir = (DirPart *) malloc(sizeof(DirPart));
		while (1)
		{
			read_bbb(fs_dev, part_lba, 1, (void *) dir);
			c = 0;
			for (i = 0; i < DIR_ENTRIES_PER_PART; i++)
			{
				if (c >= dir->part_size)
					break;
				if (dir->entry[i].addr & PRESENT)
				{
					c++;
					if (dir->entry[i].addr & NOT_EMPTY)
						size += get_size(dir->entry[i].addr >> 9, dir->entry[i].addr & IS_DIR);
				}
			}
			if (!(dir->next_part & PRESENT) || !(dir->next_part & NOT_EMPTY))
			{
				free((void *) dir);
				return size;
			}
			part_lba = dir->next_part >> 9;
		}
	}
	
	part = (FilePart *) malloc(sizeof(FilePart));
	while (1)
	{
		read_bbb(fs_dev, part_lba, 1, part);
		size += part->part_size;
		if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY))
		{
			free((void *) part);
			return size;
		}
		part_lba = part->next_part >> 9;
	}
}

int create(char *path, uint32_t len, int target_type)
{
	char *dir_path, *target_name;
	uint32_t dir_path_len, target_name_len, i, last, base_lba, offset;
	int cut_found, status;
	DirPart *dir;
	
	dir = (DirPart *) malloc(sizeof(DirPart));
		
	status = find_path(path, len, TYPE_UNDEFINED, 0, &base_lba, &offset);
	if (!status)
	{
		read_bbb(fs_dev, base_lba, 1, (void *) dir);
		if (dir->entry[offset].addr & NOT_EMPTY)
			delete_chain_parts(dir->entry[offset].addr >> 9);
		dir->entry[offset].addr = PRESENT;
		if (target_type == TYPE_DIR)
			dir->entry[offset].addr |= IS_DIR;
		write_bbb(fs_dev, base_lba, 1, (void *) dir);
		free((void *) dir);
		return 0;
	}
	
	cut_found = 0;
	for (i = 0; i < len; i++)
	{
		if (*(path + i) == '/')
		{
			last = i;
			cut_found = 1;
		}
	}
	
	if (!cut_found)
	{
		free((void *) dir);
		return 1;
	}
	
	dir_path = path;
	dir_path_len = last;
	target_name = path + last + 1;
	target_name_len = len - last - 1;
	
	if (!target_name_len)
	{
		free((void *) dir);
		return 1;
	}
	
	if (!dir_path_len)
	{
		find_empty_entry(root_lba, &base_lba, &offset);
	}
	else
	{
		status = find_path(dir_path, dir_path_len, TYPE_DIR, 0, &base_lba, &offset);
		if (status)
		{
			free((void *) dir);
			return 1;
		}
		read_bbb(fs_dev, base_lba, 1, (void *) dir);
		if (!(dir->entry[offset].addr | NOT_EMPTY))
		{
			dir->entry[offset].addr = (balloc() << 9) | PRESENT | IS_DIR | NOT_EMPTY;
			write_bbb(fs_dev, base_lba, 1, (void *) dir);
		}
		base_lba = dir->entry[offset].addr >> 9;
		find_empty_entry(base_lba, &base_lba, &offset);
	}
	
	read_bbb(fs_dev, base_lba, 1, (void *) dir);
		
	if (target_name_len > NAME_LEN)
		target_name_len = NAME_LEN;
	memcpy(dir->entry[offset].name, target_name, target_name_len);
	memset(dir->entry[offset].name + target_name_len, 0, NAME_LEN - target_name_len + 1);
	if (target_type == TYPE_DIR)
		dir->entry[offset].addr |= IS_DIR;
	
	write_bbb(fs_dev, base_lba, 1, (void *) dir);
	free((void *) dir);
	return 0;
}

void delete(char *path, uint32_t len)
{
	uint32_t base_lba, offset;
	int status;
	DirPart *dir;
	
	dir = (DirPart *) malloc(sizeof(DirPart));
		
	status = find_path(path, len, TYPE_UNDEFINED, 0, &base_lba, &offset);
	if (!status)
	{
		read_bbb(fs_dev, base_lba, 1, (void *) dir);
		if (dir->entry[offset].addr & NOT_EMPTY)
			delete_chain_parts(dir->entry[offset].addr >> 9);
		memset((void *) &dir->entry[offset], 0, sizeof(DirEntry));
		dir->part_size--;
		write_bbb(fs_dev, base_lba, 1, (void *) dir);
		free((void *) dir);
	}
}

uint32_t read_from_file(File *f, uint32_t count,
						char *data)
{
	uint32_t total, lba, seek, max;
	DirPart *dir;
	FilePart *part;
	
	dir = (DirPart *) malloc(sizeof(DirPart));
	read_bbb(fs_dev, f->base_lba, 1, (void *) dir);
	if (!(dir->entry[f->offset].addr & PRESENT) || !(dir->entry[f->offset].addr & NOT_EMPTY))
	{
		free((void *) dir);
		return 0;
	}
	lba = dir->entry[f->offset].addr >> 9;
	free((void *) dir);
	part = (FilePart *) malloc(sizeof(FilePart));
	
	total = 0;
	
	seek = f->r_seek;
	while (seek)
	{
		read_bbb(fs_dev, lba, 1, part);
		
		if (part->part_size < FILE_DATA_PER_PART)
		{
			if (seek >= part->part_size)
			{
				free((void *) part);
				return 0;
			}
			max = part->part_size - seek;
			if (count > max)
			{
				count = max;
			}
			total += count;
			memcpy(data, (char *) part->data + seek, count);
			
			free((void *) part);
			return total;
		}
		if (seek < FILE_DATA_PER_PART)
		{
			max = FILE_DATA_PER_PART - seek;
			if (count <= max)
			{
				total += count;
				memcpy(data, (char *) part->data + seek, count);
				free((void *) part);
				return total;
			}
			total += max;
			memcpy(data, (char *) part->data + seek, max);
			data += max;
			count -= max;
			seek = 0;
		}
		else
			seek -= FILE_DATA_PER_PART;
		if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY))
		{
			free((void *) part);
			return total;
		}
		lba = part->next_part >> 9;
	}
	
	while (count)
	{
		read_bbb(fs_dev, lba, 1, part);
		if (part->part_size < FILE_DATA_PER_PART)
		{
			if (count > part->part_size)
				count = part->part_size;
			memcpy(data, (char *) part->data, count);
			total += count;
			free((void *) part);
			return total;
		}
		if (count <= FILE_DATA_PER_PART)
		{
			memcpy(data, (char *) part->data, count);
			total += count;
			free((void *) part);
			return total;
		}
		memcpy(data, (char *) part->data, FILE_DATA_PER_PART);
		count -= FILE_DATA_PER_PART;
		data += FILE_DATA_PER_PART;
		if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY))
		{
			free((void *) part);
			return total;
		}
		lba = part->next_part >> 9;
	}
	free((void *) part);
		return total;
}

void write_to_file(File *f, char *data, uint32_t count)
{
	uint32_t lba, seek, new_size, max;
	DirPart *dir;
	FilePart *part;
	
	dir = (DirPart *) malloc(sizeof(DirPart));
	read_bbb(fs_dev, f->base_lba, 1, (void *) dir);
	if (!(dir->entry[f->offset].addr & PRESENT) || !(dir->entry[f->offset].addr & NOT_EMPTY))
	{
		dir->entry[f->offset].addr = (balloc() << 9) | PRESENT | NOT_EMPTY;
		write_bbb(fs_dev, f->base_lba, 1, (void *) dir);
	}
	lba = dir->entry[f->offset].addr >> 9;
	free((void *) dir);
	part = (FilePart *) malloc(sizeof(FilePart));
	
	seek = f->w_seek;
	while (seek)
	{
		read_bbb(fs_dev, lba, 1, part);
		
		if (seek < FILE_DATA_PER_PART)
		{
			max = FILE_DATA_PER_PART - seek;
			if (count <= max)
			{
				memcpy((char *) part->data + seek, data, count);
				new_size = seek + count;
				if (new_size > part->part_size)
					part->part_size = new_size;
				write_bbb(fs_dev, lba, 1, part);
				free((void *) part);
				return;
			}
			memcpy((char *) part->data + seek, data, max);
			write_bbb(fs_dev, lba, 1, part);
			data += max;
			count -= max;
			seek = 0;
		}
		else
			seek -= FILE_DATA_PER_PART;
		
		part->part_size = FILE_DATA_PER_PART;
		if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY))
		{
			part->next_part = (balloc() << 9) | PRESENT | NOT_EMPTY;
			write_bbb(fs_dev, lba, 1, part);
		}
		lba = part->next_part >> 9;
	}
	
	while (count)
	{
		read_bbb(fs_dev, lba, 1, part);
		
		if (count <= FILE_DATA_PER_PART)
		{
			memcpy((char *) part->data, data, count);
			if (count > part->part_size)
				part->part_size = count;
			write_bbb(fs_dev, lba, 1, part);
			free((void *) part);
			return;
		}
		memcpy((char *) part->data, data, FILE_DATA_PER_PART);
		data += FILE_DATA_PER_PART;
		count -= FILE_DATA_PER_PART;
		
		part->part_size = FILE_DATA_PER_PART;
		if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY))
		{
			part->next_part = (balloc() << 9) | PRESENT | NOT_EMPTY;
			write_bbb(fs_dev, lba, 1, part);
		}
		lba = part->next_part >> 9;
	}
}

int is_path(char *path, uint32_t len, int target_type, int not_empty)
{
	uint32_t base_lba, offset;
	
	return find_path(path, len, target_type, not_empty, &base_lba, &offset);
}

int find_path(	char *path, uint32_t len, int target_type, int not_empty,
				uint32_t *base_lba, uint32_t *offset)
{
	PathPart *part, *first;
	int status;
	uint32_t lba, entry_offset, dir_lba;
	DirPart *dir;
	
	part = first = to_path_parts(path, len);
	dir_lba = root_lba;
	dir = (DirPart *) malloc(sizeof(DirPart));
	
	while (part->next)
	{		
		status = find_in_dir(	dir_lba, part->name, NAME_LEN, TYPE_DIR, 1,
								&lba, &entry_offset);
		
		if (status)
		{
			free((void *) dir);
			return status;
		}
		
		read_bbb(fs_dev, lba, 1, (void *) dir);
		dir_lba = dir->entry[entry_offset].addr >> 9;
		part = part->next;
	}
	
	status = find_in_dir(	dir_lba, part->name, NAME_LEN, target_type, not_empty,
								&lba, &entry_offset);
	
	
	if (status)
	{
		free((void *) dir);
		return status;
	}
	
	*base_lba = lba;
	*offset = entry_offset;
	free((void *) dir);
	free_path_parts(first);
	
	return 0;
}

int find_in_dir(uint32_t cur_lba, const char *name, uint32_t len, int target_type, int not_empty,
				uint32_t *base_lba, uint32_t *offset)
{
	char *f_name;
	uint32_t c, entry_offset, next_lba;
	uint32_t f_len;
	DirPart *dir;
	
	f_name = (char *) malloc(NAME_LEN);
	if (len <= NAME_LEN)
		f_len = len;
	else
		f_len = NAME_LEN;
	memcpy((void *) f_name, (void *) name, f_len);
	dir = (DirPart *) malloc(sizeof(DirPart));
	
	while (1)
	{
		read_bbb(fs_dev, cur_lba, 1, (void *) dir);
		c = 0;
		for (entry_offset = 0; entry_offset < DIR_ENTRIES_PER_PART; entry_offset++)
		{
			if (c >= dir->part_size)
				break;
			if (dir->entry[entry_offset].addr & PRESENT)
			{
				c++;
				
				if (target_type == TYPE_DIR && !(dir->entry[entry_offset].addr & IS_DIR))
					continue;
				else if (target_type == TYPE_FILE && (dir->entry[entry_offset].addr & IS_DIR))
					continue;
				
				if (not_empty && !(dir->entry[entry_offset].addr & NOT_EMPTY))
					continue;
				
				if (!memcmp(f_name, dir->entry[entry_offset].name, f_len))
				{
					*base_lba = cur_lba;
					*offset = entry_offset;
					free(dir);
					free(f_name);
					return 0;
				}
			}
		}
		next_lba = dir->next_part;
		if (!(next_lba & PRESENT) || !(next_lba & NOT_EMPTY))
		{
			free(dir);
			free(f_name);
			return 1;
		}
		cur_lba = next_lba >> 9;
	}
}

PathPart *to_path_parts(char *path, uint32_t len)
{
	PathPart *first, *cur;
	uint32_t i, cur_len, start;
	
	first = (PathPart *) malloc(sizeof(PathPart));
	cur = first;
	cur_len = 0;
	start = 0;
	for (i = 0; i < len; i++)
	{
		if (*(path + i) == '/')
		{
			if (cur_len)
			{
				if (cur_len > NAME_LEN)
					cur_len = NAME_LEN;
				memcpy((void *) cur->name,(void *) (path + start), cur_len);
				memset((void *) (cur->name + cur_len), 0, NAME_LEN - cur_len + 1);
				cur->next = (PathPart *) malloc(sizeof(PathPart));
				cur = cur->next;
			}
			start = i + 1;
			cur_len = 0;
		}
		else
		{
			cur_len++;
		}
	}
	
	if (cur_len > NAME_LEN)
		cur_len = NAME_LEN;
	if (cur_len)
		memcpy((void *) cur->name,(void *) (path + start), cur_len);
	memset((void *) (cur->name + cur_len), 0, NAME_LEN - cur_len + 1);
	cur->next = 0;
	return first;
}

void free_path_parts(PathPart *first)
{
	PathPart *next;
	
	while (first)
	{
		next = first->next;
		free((void *) first);
		first = next;
	}
}

void find_empty_entry(	uint32_t dir_lba,
						uint32_t *base_lba, uint32_t *offset)
{
	DirPart *dir;
	uint32_t i;
	
	dir = (DirPart *) malloc(sizeof(DirPart));
	while (1)
	{
		read_bbb(fs_dev, dir_lba, 1, (void *) dir);
		for (i = 0; i < DIR_ENTRIES_PER_PART; i++)
		{
			if (!(dir->entry[i].addr & PRESENT))
			{
				memset((void *) &dir->entry[i], 0, sizeof(DirEntry));
				dir->entry[i].addr |= PRESENT;
				dir->part_size++;
				write_bbb(fs_dev, dir_lba, 1, (void *) dir);
				*base_lba = dir_lba;
				*offset = i;
				
				free((void *) dir);
				
				return;
			}
		}
		
		if (!(dir->next_part & PRESENT))
		{
			dir->next_part = (balloc() << 9) | PRESENT | IS_DIR | NOT_EMPTY;
			write_bbb(fs_dev, dir_lba, 1, (void *) dir);
		}
		dir_lba = dir->next_part;
	}
}

void delete_chain_parts(uint32_t lba)
{
	FilePart *part;
	uint32_t next_lba;
	
	part = (FilePart *) malloc(sizeof(FilePart));
	while (1)
	{
		read_bbb(fs_dev, lba, 1, part);
		next_lba = part->next_part;
		bfree(lba);
		if (!(next_lba & PRESENT) || !(next_lba & NOT_EMPTY))
		{
			free((void *) part);
			return;
		}
		lba = next_lba >> 9;
	}
}

uint32_t balloc()
{
	uint32_t lba;
	void *block;
	
	lba = find_in_level(0, first_bitmap_lba, 0, first_level);
	if (lba)
	{
		block = malloc(BLOCK_SIZE);
		memset(block, 0, BLOCK_SIZE);
		write_bbb(fs_dev, lba, 1, block);
		free(block);
	}
	return lba;
}

void bfree(uint32_t lba)
{
	uint32_t base_lba, pos, offset, byte, bit, i;
	uint8_t level, *bitmap;
	LevelNode *level_node;
	
	base_lba = first_bitmap_lba;
	level_node = first_level;
	level = levels;
	bitmap = (uint8_t *) malloc(BLOCK_SIZE);
	while (level)
	{
		pos = lba;
		for (i = 1; i < level; i++)
			pos /= BITMAP_SIZE;
		offset = pos / BITMAP_SIZE;
		byte = (pos % BITMAP_SIZE) / 8;
		bit = (pos % BITMAP_SIZE) % 8;
		read_bbb(fs_dev, base_lba + offset, 1, (void *) bitmap);
		*(bitmap + byte) &= (~(1 << bit)) & 0xFF;
		write_bbb(fs_dev, base_lba + offset, 1, (void *) bitmap);
		
		base_lba += level_node->level_size;
		level_node = level_node->next;
		level--;
	}
	free((void *) bitmap);
}

uint32_t find_in_level(uint8_t level, uint32_t base_lba, uint32_t offset, LevelNode *level_node)
{
	uint32_t lba, i, next_offset, val;
	uint8_t *bitmap, n, b;
	
	lba = base_lba + offset;
	bitmap = (uint8_t *) malloc(BLOCK_SIZE);
	read_bbb(fs_dev, lba, 1, (void *) bitmap);
	for (i = 0; i < BLOCK_SIZE; i++)
	{
		if (*(bitmap + i) != 0xFF)
		{
			n = *(bitmap + i);
			for (b = 0; b < 8; b++)
				if (!(n & (1 << b)))
				{
					next_offset = i * 8 + b;
					if (level == levels - 1)
					{
						n |= 1 << b;
						*(bitmap + i) = n;
						write_bbb(fs_dev, lba, 1, (void *) bitmap);
						free((void *) bitmap);
						return offset * BITMAP_SIZE + next_offset;
					}
					val = find_in_level(level + 1, base_lba + level_node->level_size, next_offset, level_node->next);
					if (val)
					{
						free((void *) bitmap);
						return val;
					}
					n |= 1 << b;
					*(bitmap + i) = n;
					write_bbb(fs_dev, lba, 1, (void *) bitmap);
				}
		}
	}
	free((void *) bitmap);
	return 0;
}