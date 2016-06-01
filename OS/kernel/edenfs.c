// -----------------------------------------------------------------------------
// EdenFS Module
// -------------
// 
// General      :   The module provides an interface to manage the EdenFS
//                  file-system.
//
// Input        :   None
//
// Process      :   Initializes the interface and provides functions to manage
//                  the file-system.
//
// Output       :   None
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------


#include <edenfs.h>


static UHCIDevice *fs_dev;
static uint32_t block_count;
static uint8_t levels;
static LevelNode *first_level;
static uint32_t root_lba;
static uint32_t first_bitmap_lba;


// -----------------------------------------------------------------------------
// init_fs
// -------
// 
// General      :   The function checks whether a BBB-type USB device contains
//                  the EdenFS file-system and initializes the interface.
//
// Parameters   :
//              dev -   A pointer to the USB device descriptor (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void init_fs(UHCIDevice *dev) {
    BootSect *bsect;
    uint32_t temp;
    uint8_t i;
    LevelNode *level_node, *temp_level_node;

    bsect = (BootSect *) malloc(BLOCK_SIZE);

    // Read the boot-sector.
    read_bbb(dev, 0, 1, bsect);
    // Check whether the boot-sector contain the EdenFS signature.
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
    // Calculate the memory space needed for every level of the bitmaps.
    for (i = 0; i < levels; i++) {
        temp_level_node = (LevelNode *) malloc(sizeof (LevelNode));
        temp_level_node->next = level_node;
        level_node = temp_level_node;
        level_node->level_size = temp / BITMAP_SIZE;
        temp = level_node->level_size;
    }
    first_level = level_node;

    working_dir = (char *) malloc(2);
    // Set working directory to root.
    *working_dir = '/';
}

// -----------------------------------------------------------------------------
// open
// ----
// 
// General      :   The function opens a file for reading/writing.
//
// Parameters   :
//              path    -   The path to the file in the file-system (In)
//              len     -   The length of the path string (In)
//              mode    -   The opening mode ('w' for creating the file and
//                          opening it, 'r' for opening an existing file) (In)
//
// Return Value :   A pointer to an opened file descriptor, or 0 if opening a
//                  file that doesn't exist
//
// -----------------------------------------------------------------------------

File *open(char *path, uint32_t len, char mode) {
    uint32_t base_lba;
    uint32_t offset;
    int status;
    File *f;

    if (mode == 'w')
        // If the mode is 'w', create the file.
        create(path, len, TYPE_FILE);
    // Find the entry of the wanted file.
    status = find_path(path, len, TYPE_FILE, 0, &base_lba, &offset);
    if (status)
        return 0;

    // Create the file descriptor.
    f = (File *) malloc(sizeof (File));
    f->base_lba = base_lba;
    f->offset = offset;
    f->r_seek = 0;
    f->w_seek = 0;

    return f;
}

// -----------------------------------------------------------------------------
// list
// ----
// 
// General      :   The function lists directory contents.
//
// Parameters   :
//              path    -   The path to the directory in the file-system (In)
//              len     -   The length of the path string (In)
//              tree    -   Whether to list directory structure as tree
//                          (boolean) (In)
//              size    -   Whether to print the size of each element (boolean)
//                          (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void list(char *path, uint32_t len, int tree, int size) {
    uint32_t base_lba;
    uint32_t offset;
    uint32_t lba;
    int status;
    DirPart *dir;

    if ((len == 1 && path[0] == '/') || len == 0)
        lba = root_lba;
    else {
        status = find_path(path, len, TYPE_DIR, 0, &base_lba, &offset);
        if (status) {
            puts("Path not found!\n");
            return;
        }
        dir = (DirPart *) malloc(sizeof (DirPart));
        // Read the part of the directory that contains the wanted directory.
        read_bbb(fs_dev, base_lba, 1, dir);
        if (!(dir->entry[offset].addr & NOT_EMPTY))
            // If the directory to be listed is empty, return.
            return;
        lba = dir->entry[offset].addr >> 9;
    }
    // List the directory contents.
    _list(lba, tree, size, 0);
}

// -----------------------------------------------------------------------------
// _list
// -----
// 
// General      :   The function lists directory contents.
//
// Parameters   :
//              lba     -   The LBA of the first directory-part (In)
//              tree    -   Whether to list directory structure as tree
//                          (boolean) (In)
//              size    -   Whether to print the size of each element (boolean)
//                          (In)
//              level   -   The level of the directory in the directory tree
//                          (used for recursion) (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void _list(uint32_t lba, int tree, int size, uint32_t level) {
    uint32_t i, c, j;
    DirPart *dir;
    char *buff;

    dir = (DirPart *) malloc(sizeof (DirPart));
    while (1) {
        // Read the directory-part.
        read_bbb(fs_dev, lba, 1, (void *) dir);
        // Set the entry-count to 0.
        c = 0;
        // For every entry in the directory-part:
        for (i = 0; i < DIR_ENTRIES_PER_PART; i++) {
            if (c >= dir->part_size)
                // If the entry-count reaches or exceeds the size of the
                // directory, quit searching for files to list.
                break;
            // If the entry contains a present entry:
            if (dir->entry[i].addr & PRESENT) {
                // Increment the entry-count.
                c++;
                /* The following code handles the graphic part of the listing.
                 */
                if (level) {
                    for (j = 0; j < level - 1; j++)
                        puts("    |");
                    puts("    +--- ");
                }
                puts(dir->entry[i].name);
                if (dir->entry[i].addr & IS_DIR)
                    puts(" (DIR)");
                else
                    puts(" (FILE)");
                if (size) {
                    if (dir->entry[i].addr & NOT_EMPTY) {
                        buff = (char *) malloc(100);
                        puts(" (");
                        puts(uitoa(get_size(dir->entry[i].addr >> 9), buff, BASE10));
                        free((void *) buff);
                    } else
                        puts(" (0");
                    if (dir->entry[i].addr & IS_DIR)
                        puts(" entries)");
                    else
                        puts(" bytes)");
                }
                putc('\n');
                if (tree && (dir->entry[i].addr & NOT_EMPTY) && (dir->entry[i].addr & IS_DIR)) {
                    for (j = 0; j < level + 1; j++)
                        puts("    |");
                    putc('\n');
                    // If the entry is of a directory which is not empty, and
                    // the listing is recursive (tree), list the entry's
                    // directory at the next level of the tree.
                    _list(dir->entry[i].addr >> 9, tree, size, level + 1);
                }
            }
        }
        if (!(dir->next_part & PRESENT) || !(dir->next_part & NOT_EMPTY)) {
            free((void *) dir);
            // If there is not next part, stop listing.
            return;
        }
        // Get the LBA of the next part of the directory.
        lba = dir->next_part >> 9;
    }
}

// -----------------------------------------------------------------------------
// get_size
// --------
// 
// General      :   The function returns the total size of linked parts.
//
// Parameters   :
//              part_lba    -   The LBA of the first part (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

uint32_t get_size(uint32_t part_lba) {
    uint32_t size;
    FilePart *part;

    size = 0;
    part = (FilePart *) malloc(sizeof (FilePart));
    while (1) {
        // Read the part.
        read_bbb(fs_dev, part_lba, 1, (void *) part);
        // Add its size to the total.
        size += part->part_size;
        if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY)) {
            free((void *) part);
            // If there is not next part, return the size.
            return size;
        }
        // Get the next part.
        part_lba = part->next_part >> 9;
    }
}

// -----------------------------------------------------------------------------
// create
// ------
// 
// General      :   The function create a file or a directory.
//
// Parameters   :
//              path        -   The path to the target in the file-system (In)
//              len         -   The length of the path string (In)
//              target_type -   The type of the target as int (see the DEFINEs
//                              for values) (In)
//
// Return Value :   0 if successful, otherwise error specifier
//
// -----------------------------------------------------------------------------

int create(char *path, uint32_t len, int target_type) {
    char *dir_path;
    char *target_name;
    uint32_t dir_path_len;
    uint32_t target_name_len;
    uint32_t i;
    uint32_t last;
    uint32_t base_lba;
    uint32_t offset;
    int cut_found;
    int status;
    DirPart *dir;

    dir = (DirPart *) malloc(sizeof (DirPart));

    status = find_path(path, len, TYPE_UNDEFINED, 0, &base_lba, &offset);
    if (!status) {
        /* If the object already exist, overwrite it.
         */
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
    /* Find the last appearance of '/' in the path
     */
    for (i = 0; i < len; i++) {
        if (*(path + i) == '/') {
            last = i;
            cut_found = 1;
        }
    }

    if (!cut_found) {
        free((void *) dir);
        // If there was no '/' in the path, it is invalid.
        return 1;
    }

    /* The path to the directory containing the target is the path until the
     * last appearance of '/', and the name of the target is the string 
     * thereafter.
     */
    dir_path = path;
    dir_path_len = last;
    target_name = path + last + 1;
    target_name_len = len - last - 1;

    if (!target_name_len) {
        free((void *) dir);
        // If the target name is empty, it is invalid.
        return 1;
    }

    if (!dir_path_len) {
        // If the path of the directory that contains the target is empty, it is
        // the root directory. Find an empty entry for the target.
        find_empty_entry(root_lba, &base_lba, &offset);
    } else {
        /* Find an empty entry for the target in the directory specified in the
         * path.
         */
        status = find_path(dir_path, dir_path_len, TYPE_DIR, 0, &base_lba, &offset);
        if (status) {
            free((void *) dir);
            // If the directory that contains the target does not exist, it is
            // invalid.
            return 1;
        }
        read_bbb(fs_dev, base_lba, 1, (void *) dir);
        if (!(dir->entry[offset].addr | NOT_EMPTY)) {
            dir->entry[offset].addr = (balloc() << 9) | PRESENT | IS_DIR | NOT_EMPTY;
            write_bbb(fs_dev, base_lba, 1, (void *) dir);
        }
        base_lba = dir->entry[offset].addr >> 9;
        find_empty_entry(base_lba, &base_lba, &offset);
    }

    // Read the directory-part that will contain the target entry.
    read_bbb(fs_dev, base_lba, 1, (void *) dir);
    
    // The name length must not exceed the limit.
    if (target_name_len > NAME_LEN)
        target_name_len = NAME_LEN;
    // Copy the name of the target to the entry.
    memcpy(dir->entry[offset].name, target_name, target_name_len);
    // Fill the rest of the name-buffer with 0's.
    memset(dir->entry[offset].name + target_name_len, 0, NAME_LEN - target_name_len + 1);
    if (target_type == TYPE_DIR)
        // If the target is a directory, set the IS_DIR flag.
        dir->entry[offset].addr |= IS_DIR;

    // Write the modified directory-part.
    write_bbb(fs_dev, base_lba, 1, (void *) dir);
    free((void *) dir);
    return 0;
}

// -----------------------------------------------------------------------------
// delete
// ------
// 
// General      :   The function deletes a file or a directory.
//
// Parameters   :
//              path        -   The path to the target in the file-system (In)
//              len         -   The length of the path string (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void delete(char *path, uint32_t len) {
    uint32_t base_lba;
    uint32_t offset;
    int status;
    DirPart *dir;

    dir = (DirPart *) malloc(sizeof (DirPart));

    status = find_path(path, len, TYPE_UNDEFINED, 0, &base_lba, &offset);
    if (!status) {
        read_bbb(fs_dev, base_lba, 1, (void *) dir);
        if (dir->entry[offset].addr & NOT_EMPTY)
            // If the target is not empty, erase all of its parts.
            delete_chain_parts(dir->entry[offset].addr >> 9);
        // Zero all of the entry.
        memset((void *) &dir->entry[offset], 0, sizeof (DirEntry));
        // decrement the size of the directory.
        dir->part_size--;
        // Write the changes to the device.
        write_bbb(fs_dev, base_lba, 1, (void *) dir);
        free((void *) dir);
    }
}

// -----------------------------------------------------------------------------
// read_from_file
// --------------
// 
// General      :   The function reads data from an open file.
//
// Parameters   :
//              f       -   A pointer to an open file descriptor (In)
//              count   -   The amount of bytes to read (In)
//              data    -   A pointer of the buffer to write the data to (Out)
//
// Return Value :   The actual amount of bytes read
//
// -----------------------------------------------------------------------------

uint32_t read_from_file(File *f, uint32_t count, char *data) {
    uint32_t total;
    uint32_t lba;
    uint32_t seek;
    uint32_t max;
    DirPart *dir;
    FilePart *part;

    dir = (DirPart *) malloc(sizeof (DirPart));
    read_bbb(fs_dev, f->base_lba, 1, (void *) dir);
    if (!(dir->entry[f->offset].addr & PRESENT) || !(dir->entry[f->offset].addr & NOT_EMPTY)) {
        free((void *) dir);
        // If the file is empty or does not exist, return 0 (no bytes read).
        return 0;
    }
    // Get the LBA of the first part.
    lba = dir->entry[f->offset].addr >> 9;
    free((void *) dir);
    part = (FilePart *) malloc(sizeof (FilePart));

    // Set the total amount of byte read to 0.
    total = 0;
    
    // Get the read seek.
    seek = f->r_seek;
    while (seek) {
        read_bbb(fs_dev, lba, 1, part);

        if (part->part_size < FILE_DATA_PER_PART) {
            if (seek >= part->part_size) {
                free((void *) part);
                // If this is the last part of the file and the seek is greater
                // than the remaining size, return 0 (no bytes read)
                return 0;
            }
            max = part->part_size - seek;
            if (count > max) {
                count = max;
            }
            total += count;
            memcpy(data, (char *) part->data + seek, count);

            free((void *) part);
            return total;
        }
        if (seek < FILE_DATA_PER_PART) {
            max = FILE_DATA_PER_PART - seek;
            if (count <= max) {
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
        } else
            seek -= FILE_DATA_PER_PART;
        if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY)) {
            free((void *) part);
            return total;
        }
        lba = part->next_part >> 9;
    }

    while (count) {
        read_bbb(fs_dev, lba, 1, part);
        if (part->part_size < FILE_DATA_PER_PART) {
            if (count > part->part_size)
                count = part->part_size;
            memcpy(data, (char *) part->data, count);
            total += count;
            free((void *) part);
            return total;
        }
        if (count <= FILE_DATA_PER_PART) {
            memcpy(data, (char *) part->data, count);
            total += count;
            free((void *) part);
            return total;
        }
        memcpy(data, (char *) part->data, FILE_DATA_PER_PART);
        count -= FILE_DATA_PER_PART;
        data += FILE_DATA_PER_PART;
        if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY)) {
            free((void *) part);
            return total;
        }
        lba = part->next_part >> 9;
    }
    free((void *) part);
    return total;
}

// -----------------------------------------------------------------------------
// write_to_file
// -------------
// 
// General      :   The function writes data to an open file.
//
// Parameters   :
//              f       -   A pointer to an open file descriptor (In)
//              count   -   The amount of bytes to write (In)
//              data    -   A pointer of the buffer to read the data from (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void write_to_file(File *f, char *data, uint32_t count) {
    uint32_t lba;
    uint32_t seek;
    uint32_t new_size;
    uint32_t max;
    DirPart *dir;
    FilePart *part;

    dir = (DirPart *) malloc(sizeof (DirPart));
    read_bbb(fs_dev, f->base_lba, 1, (void *) dir);
    if (!(dir->entry[f->offset].addr & PRESENT) || !(dir->entry[f->offset].addr & NOT_EMPTY)) {
        dir->entry[f->offset].addr = (balloc() << 9) | PRESENT | NOT_EMPTY;
        write_bbb(fs_dev, f->base_lba, 1, (void *) dir);
    }
    lba = dir->entry[f->offset].addr >> 9;
    free((void *) dir);
    part = (FilePart *) malloc(sizeof (FilePart));

    seek = f->w_seek;
    while (seek) {
        read_bbb(fs_dev, lba, 1, part);

        if (seek < FILE_DATA_PER_PART) {
            max = FILE_DATA_PER_PART - seek;
            if (count <= max) {
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
        } else
            seek -= FILE_DATA_PER_PART;

        part->part_size = FILE_DATA_PER_PART;
        if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY)) {
            part->next_part = (balloc() << 9) | PRESENT | NOT_EMPTY;
            write_bbb(fs_dev, lba, 1, part);
        }
        lba = part->next_part >> 9;
    }

    while (count) {
        read_bbb(fs_dev, lba, 1, part);

        if (count <= FILE_DATA_PER_PART) {
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
        if (!(part->next_part & PRESENT) || !(part->next_part & NOT_EMPTY)) {
            part->next_part = (balloc() << 9) | PRESENT | NOT_EMPTY;
            write_bbb(fs_dev, lba, 1, part);
        }
        lba = part->next_part >> 9;
    }
}

// -----------------------------------------------------------------------------
// is_path
// -------
// 
// General      :   The function deletes a file or a directory.
//
// Parameters   :
//              path        -   The path to the target in the file-system (In)
//              len         -   The length of the path string (In)
//              target_type -   The type of the target as int (see the DEFINEs
//                              for values) (In)
//              not_empty   -   Whether an empty entry is invalid (boolean) (In)
//
// Return Value :   True (non-zero) if found, otherwise False (zero)
//
// -----------------------------------------------------------------------------

int is_path(char *path, uint32_t len, int target_type, int not_empty) {
    uint32_t base_lba;
    uint32_t offset;

    return !find_path(path, len, target_type, not_empty, &base_lba, &offset);
}

// -----------------------------------------------------------------------------
// find_path
// ---------
// 
// General      :   The function finds the entry of a file or a directory.
//
// Parameters   :
//              path        -   The path to the target in the file-system (In)
//              len         -   The length of the path string (In)
//              target_type -   The type of the target as int (see the DEFINEs
//                              for values) (In)
//              not_empty   -   Whether an empty entry is invalid (boolean) (In)
//              base_lba    -   A pointer to an unsigned int, which will contain
//                              the base LBA of the entry of the target (Out)
//              offset      -   A pointer to an unsigned int, which will contain
//                              the offset of the entry of the target (Out)
//
// Return Value :   0 if successful, otherwise error specifier
//
// -----------------------------------------------------------------------------

int find_path(char *path, uint32_t len, int target_type, int not_empty,
        uint32_t *base_lba, uint32_t *offset) {
    PathPart *part;
    PathPart *first;
    int status;
    uint32_t lba;
    uint32_t entry_offset;
    uint32_t dir_lba;
    DirPart *dir;

    part = first = to_path_parts(path, len);
    dir_lba = root_lba;
    dir = (DirPart *) malloc(sizeof (DirPart));

    while (part->next) {
        status = find_in_dir(dir_lba, part->name, NAME_LEN, TYPE_DIR, 1,
                &lba, &entry_offset);

        if (status) {
            free((void *) dir);
            return status;
        }

        read_bbb(fs_dev, lba, 1, (void *) dir);
        dir_lba = dir->entry[entry_offset].addr >> 9;
        part = part->next;
    }

    status = find_in_dir(dir_lba, part->name, NAME_LEN, target_type, not_empty,
            &lba, &entry_offset);


    if (status) {
        free((void *) dir);
        return status;
    }

    *base_lba = lba;
    *offset = entry_offset;
    free((void *) dir);
    free_path_parts(first);

    return 0;
}

// -----------------------------------------------------------------------------
// find_in_dir
// -----------
// 
// General      :   The function finds the entry of a file or a directory inside
//                  a directory-parts chain. 
//
// Parameters   :
//              cur_lba     -   The LBA of the first part of the directory
//                              chain (In)
//              name        -   The name of the target (In)
//              len         -   The length of the name string (In)
//              target_type -   The type of the target as int (see the DEFINEs
//                              for values) (In)
//              not_empty   -   Whether an empty entry is invalid (boolean) (In)
//              base_lba    -   A pointer to an unsigned int, which will contain
//                              the base LBA of the entry of the target (Out)
//              offset      -   A pointer to an unsigned int, which will contain
//                              the offset of the entry of the target (Out)
//
// Return Value :   0 if successful, otherwise error specifier
//
// -----------------------------------------------------------------------------

int find_in_dir(uint32_t cur_lba, const char *name, uint32_t len,
        int target_type, int not_empty, uint32_t *base_lba, uint32_t *offset) {
    char *f_name;
    uint32_t c;
    uint32_t entry_offset;
    uint32_t next_lba;
    uint32_t f_len;
    DirPart *dir;

    f_name = (char *) malloc(NAME_LEN);
    if (len <= NAME_LEN)
        f_len = len;
    else
        f_len = NAME_LEN;
    memcpy((void *) f_name, (void *) name, f_len);
    dir = (DirPart *) malloc(sizeof (DirPart));

    while (1) {
        read_bbb(fs_dev, cur_lba, 1, (void *) dir);
        c = 0;
        for (entry_offset = 0; entry_offset < DIR_ENTRIES_PER_PART; entry_offset++) {
            if (c >= dir->part_size)
                break;
            if (dir->entry[entry_offset].addr & PRESENT) {
                c++;

                if (target_type == TYPE_DIR && !(dir->entry[entry_offset].addr & IS_DIR))
                    continue;
                else if (target_type == TYPE_FILE && (dir->entry[entry_offset].addr & IS_DIR))
                    continue;

                if (not_empty && !(dir->entry[entry_offset].addr & NOT_EMPTY))
                    continue;

                if (!memcmp(f_name, dir->entry[entry_offset].name, f_len)) {
                    *base_lba = cur_lba;
                    *offset = entry_offset;
                    free(dir);
                    free(f_name);
                    return 0;
                }
            }
        }
        
        next_lba = dir->next_part;
        if (!(next_lba & PRESENT) || !(next_lba & NOT_EMPTY)) {
            free(dir);
            free(f_name);
            return 1;
        }
        cur_lba = next_lba >> 9;
    }
}

// -----------------------------------------------------------------------------
// to_path_parts
// -------------
// 
// General      :   The function creates a linked list of path-parts.
//
// Parameters   :
//              path        -   The path to the target in the file-system (In)
//              len         -   The length of the path string (In)
//
// Return Value :   A pointer to the fisrt path-part
//
// -----------------------------------------------------------------------------

PathPart *to_path_parts(char *path, uint32_t len) {
    PathPart *first;
    PathPart *cur;
    uint32_t i;
    uint32_t cur_len;
    uint32_t start;

    first = (PathPart *) malloc(sizeof (PathPart));
    cur = first;
    cur_len = 0;
    start = 0;
    for (i = 0; i < len; i++) {
        if (*(path + i) == '/') {
            if (cur_len) {
                if (cur_len > NAME_LEN)
                    cur_len = NAME_LEN;
                memcpy((void *) cur->name, (void *) (path + start), cur_len);
                memset((void *) (cur->name + cur_len), 0, NAME_LEN - cur_len + 1);
                cur->next = (PathPart *) malloc(sizeof (PathPart));
                cur = cur->next;
            }
            start = i + 1;
            cur_len = 0;
        } else {
            cur_len++;
        }
    }

    if (cur_len > NAME_LEN)
        cur_len = NAME_LEN;
    if (cur_len)
        memcpy((void *) cur->name, (void *) (path + start), cur_len);
    memset((void *) (cur->name + cur_len), 0, NAME_LEN - cur_len + 1);
    cur->next = 0;
    return first;
}

// -----------------------------------------------------------------------------
// free_path_parts
// ---------------
// 
// General      :   The function frees the memory that is occupied by a
//                  path-part linked list.
//
// Parameters   :
//              first   -   A pointer to the first path-part (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void free_path_parts(PathPart *first) {
    PathPart *next;

    while (first) {
        next = first->next;
        free((void *) first);
        first = next;
    }
}

// -----------------------------------------------------------------------------
// find_empty_entry
// ----------------
// 
// General      :   The function finds an empty entry in a directory, and sets
//                  it to be present.
//
// Parameters   :
//              dir_lba     -   The LBA of the first directory part
//              base_lba    -   A pointer to an unsigned int, which will contain
//                              the base LBA of the entry (Out)
//              offset      -   A pointer to an unsigned int, which will contain
//                              the offset of the entry (Out)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void find_empty_entry(uint32_t dir_lba, uint32_t *base_lba, uint32_t *offset) {
    DirPart *dir;
    uint32_t i;

    dir = (DirPart *) malloc(sizeof (DirPart));
    while (1) {
        read_bbb(fs_dev, dir_lba, 1, (void *) dir);
        for (i = 0; i < DIR_ENTRIES_PER_PART; i++) {
            if (!(dir->entry[i].addr & PRESENT)) {
                memset((void *) &dir->entry[i], 0, sizeof (DirEntry));
                dir->entry[i].addr |= PRESENT;
                dir->part_size++;
                write_bbb(fs_dev, dir_lba, 1, (void *) dir);
                *base_lba = dir_lba;
                *offset = i;

                free((void *) dir);

                return;
            }
        }

        if (!(dir->next_part & PRESENT)) {
            dir->next_part = (balloc() << 9) | PRESENT | IS_DIR | NOT_EMPTY;
            write_bbb(fs_dev, dir_lba, 1, (void *) dir);
        }
        dir_lba = dir->next_part;
    }
}

// -----------------------------------------------------------------------------
// delete_chain_parts
// ------------------
// 
// General      :   The function deletes a chain of linked parts.
//
// Parameters   :
//              lba -   The LBA of the first part
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void delete_chain_parts(uint32_t lba) {
    FilePart *part;
    uint32_t next_lba;

    part = (FilePart *) malloc(sizeof (FilePart));
    while (1) {
        read_bbb(fs_dev, lba, 1, part);
        next_lba = part->next_part;
        bfree(lba);
        if (!(next_lba & PRESENT) || !(next_lba & NOT_EMPTY)) {
            free((void *) part);
            return;
        }
        lba = next_lba >> 9;
    }
}

// -----------------------------------------------------------------------------
// balloc
// ------
// 
// General      :   The function allocates a block.
//
// Parameters   :   None
//
// Return Value :   The LBA of the block
//
// -----------------------------------------------------------------------------

uint32_t balloc(void) {
    uint32_t lba;
    void *block;

    lba = find_in_level(0, first_bitmap_lba, 0, first_level);
    if (lba) {
        block = malloc(BLOCK_SIZE);
        memset(block, 0, BLOCK_SIZE);
        write_bbb(fs_dev, lba, 1, block);
        free(block);
    }
    return lba;
}

// -----------------------------------------------------------------------------
// bfree
// -----
// 
// General      :   The function deallocates a block.
//
// Parameters   :
//              lba -  The LBA of the block (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void bfree(uint32_t lba) {
    uint32_t base_lba;
    uint32_t pos;
    uint32_t offset;
    uint32_t byte;
    uint32_t bit;
    uint32_t i;
    uint8_t level;
    uint8_t *bitmap;
    LevelNode *level_node;

    base_lba = first_bitmap_lba;
    level_node = first_level;
    level = levels;
    bitmap = (uint8_t *) malloc(BLOCK_SIZE);
    while (level) {
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

// -----------------------------------------------------------------------------
// find_in_level
// -------------
// 
// General      :   The function finds a free block in the bitmap hierarchy.
//
// Parameters   :
//              level       -   The level of the bitmap in the hierarchy (for
//                              recursion) (In)
//              base_lba    -   The base LBA of the bitmap level (In)
//              offset      -   The offset of the bitmap (In)
//              level_node  -   A pointer to a level-node which contains the
//                              amount of bitmap in the current level (In)
//
// Return Value :   The LBA of the block
//
// -----------------------------------------------------------------------------

uint32_t find_in_level(uint8_t level, uint32_t base_lba, uint32_t offset,
        LevelNode *level_node) {
    uint32_t lba;
    uint32_t i;
    uint32_t next_offset;
    uint32_t val;
    uint8_t *bitmap;
    uint8_t n;
    uint8_t b;

    lba = base_lba + offset;
    bitmap = (uint8_t *) malloc(BLOCK_SIZE);
    read_bbb(fs_dev, lba, 1, (void *) bitmap);
    for (i = 0; i < BLOCK_SIZE; i++) {
        if (*(bitmap + i) != 0xFF) {
            n = *(bitmap + i);
            for (b = 0; b < 8; b++)
                if (!(n & (1 << b))) {
                    next_offset = i * 8 + b;
                    if (level == levels - 1) {
                        n |= 1 << b;
                        *(bitmap + i) = n;
                        write_bbb(fs_dev, lba, 1, (void *) bitmap);
                        free((void *) bitmap);
                        return offset * BITMAP_SIZE + next_offset;
                    }
                    val = find_in_level(level + 1, base_lba + level_node->level_size, next_offset, level_node->next);
                    if (val) {
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

// -----------------------------------------------------------------------------
// join_path
// ---------
// 
// General      :   The function joins two parts of a path.
//
// Parameters   :
//              first   -   A string representing the first part of the path
//                          (In)
//              second  -   A string representing the second part of the path
//                          (In)
//
// Return Value :   A string representing the whole path
//
// -----------------------------------------------------------------------------

char *join_path(char *first, char *second) {
    uint32_t len;
    uint32_t first_len;
    uint32_t second_len;
    char *final;

    first_len = strlen(first);
    second_len = strlen(second);

    while (*(first + (first_len - 1)) == '/')
        first_len--;
    while (*second == '/') {
        second++;
        second_len--;
    }

    len = first_len + second_len + 2;
    final = malloc(len);
    memcpy(final, first, first_len);
    *(final + first_len) = '/';
    memcpy(final + (first_len + 1), second, second_len);

    return final;
}

// -----------------------------------------------------------------------------
// get_full_path
// -------------
// 
// General      :   The function creates the full path (if the path is relative
//                  to the working directory).
//
// Parameters   :
//              s   -   A string representing the path (In)
//
// Return Value :   A string representing the full path
//
// -----------------------------------------------------------------------------

char *get_full_path(char *s) {
    char *path;


    if (*s == '/') {
        path = malloc(strlen(s) + 1);
        strcpy(path, s);
    } else
        path = join_path(working_dir, s);

    return path;
}
