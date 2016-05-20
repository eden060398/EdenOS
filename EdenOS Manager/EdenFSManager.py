# region ----------IMPORTS----------
import struct
import UI
import os
# endregion

# region ---------- CONSTANTS ----------
EDENFS_BLOCK_COUNT_OFFSET = 488
EDENFS_BLOCK_COUNT_LEN = 4

EDENFS_SIGN_OFFSET = 492
EDENFS_SIGN_LEN = 9
EDENFS_SIGN = 'EDENFS100'

BLOCK_SIZE_BYTES = 512
BITMAP_SIZE_BITS = BLOCK_SIZE_BYTES * 8
LEVELS_OFFSET = 501
FIRST_BITMAP_LBA_OFFSET = 506
FIRST_SECT_LBA_OFFSET = 502

DIR_ENTRY_SIZE = 16
DIR_ENTRIES_PER_PART = 31
DIR_ENTRIES_OFFSET = 2
NAME_LEN = 12
RESERVED_PART = 12
NEXT_PART_OFFSET = DIR_ENTRIES_PER_PART * DIR_ENTRY_SIZE + RESERVED_PART

FILE_DATA_PER_BLOCK = 506

PRESENT = 1 << 0
IS_DIR = 1 << 1
NOT_EMPTY = 1 << 2

CURRENT_SEEK = 0
START_SEEK = 1
END_SEEK = 2

MAX_READ = 4096
# endregion

#  region ---------- FUNCTIONS ----------


def is_edenfs(dev):
    return dev.read(EDENFS_SIGN_OFFSET, EDENFS_SIGN_LEN) == EDENFS_SIGN
# endregion

# region ---------- CLASSES ----------


class EdenFS:
    def __init__(self, dev):
        self.dev = dev
        self.first_bitmap_lba = struct.unpack('I', self.dev.read(FIRST_BITMAP_LBA_OFFSET, 4))[0]

        cap = self.dev.size / BLOCK_SIZE_BYTES
        self.levels = 0
        self.levels_count = []
        while cap:
            count = cap / BITMAP_SIZE_BITS
            if count:
                self.levels += 1
                self.levels_count.append(count)
            cap = count
        self.levels_count.reverse()
        self.root_lba = self.first_bitmap_lba + sum(self.levels_count)
        self.open_files = []

    def is_edenfs(self):
        return self.dev.read(EDENFS_SIGN_OFFSET, EDENFS_SIGN_LEN) == EDENFS_SIGN

    def format_edenfs(self):
        print 'Writing filesystem...'
        prog = UI.Progress(0)
        total_blocks = sum(self.levels_count)
        done_blocks = 0

        total_allocated = self.first_bitmap_lba + sum(self.levels_count)
        allocated_block = '\xFF' * BLOCK_SIZE_BYTES
        free_block = '\x00' * BLOCK_SIZE_BYTES
        for level in xrange(self.levels):
            bitmap_lba = self.first_bitmap_lba + sum(self.levels_count[:-(level + 1)])
            t_total = total_allocated
            count = self.levels_count[self.levels - level - 1]
            while t_total >= BITMAP_SIZE_BITS:
                self.dev.write_blocks(lba=bitmap_lba, data=allocated_block, block_size=BLOCK_SIZE_BYTES)
                t_total -= BITMAP_SIZE_BITS
                count -= 1
                bitmap_lba += 1
                done_blocks += 1
                prog.update(done_blocks * 100 / total_blocks)
            byte_n = t_total / 8
            bit_n = t_total % 8
            byte_data = (~ ((~ 0) << bit_n)) & 0xFF
            next_data = '\xFF' * byte_n + struct.pack('B', byte_data) + '\x00' * (BLOCK_SIZE_BYTES - byte_n - 1)
            self.dev.write_blocks(lba=bitmap_lba, data=next_data, block_size=BLOCK_SIZE_BYTES)
            count -= 1
            bitmap_lba += 1
            done_blocks += 1
            prog.update(done_blocks * 100 / total_blocks)
            while count:
                self.dev.write_blocks(lba=bitmap_lba, data=free_block, block_size=BLOCK_SIZE_BYTES)
                count -= 1
                bitmap_lba += 1
                done_blocks += 1
                prog.update(done_blocks * 100 / total_blocks)
            total_allocated /= BITMAP_SIZE_BITS
        prog.done()
        self.root_lba = self.balloc()
        print

        block_count = self.dev.size / BLOCK_SIZE_BYTES
        if self.dev.size % BLOCK_SIZE_BYTES:
            block_count += 1
        self.dev.write(EDENFS_BLOCK_COUNT_OFFSET, struct.pack('I', block_count))
        self.dev.write(LEVELS_OFFSET, struct.pack('B', self.levels))
        self.dev.write(FIRST_SECT_LBA_OFFSET, struct.pack('I', self.root_lba))
        self.dev.write(EDENFS_SIGN_OFFSET, EDENFS_SIGN)

    def list_dir(self, path, tree=False, put_type=False, put_size=False):
        cur_lba = self.root_lba
        if path != '/':
            path_parts = EdenFS.path_parts(path)
            for part in path_parts:
                found = self.__find_in_dir(cur_lba, part, PRESENT, IS_DIR)
                if found is None:
                    raise Exception('FS Error', 'Path does not exist!')
                found_attr = self.__get_entry_attr(found[0], found[1])
                if not EdenFS.__has_attr(found_attr, NOT_EMPTY):
                    if part != path_parts[-1]:
                        raise Exception('FS Error', 'Path does not exist!')
                    else:
                        return
                cur_lba = self.__get_entry_lba(found[0], found[1])
        return self.__list(cur_lba, tree=tree, put_type=put_type, put_size=put_size)

    def open(self, path, fmt='r'):
        f = File(fs=self, path=path, fmt=fmt)
        if f not in self.open_files:
            self.open_files.append(f)
            return f
        raise Exception('FS Error', 'File already open!')

    def make_dir(self, path):
        self.create(path, is_dir=True, overwrite=False)

    def delete(self, path):
        found = self.find_path(path, PRESENT)
        if found is not None:
            first_lba = self.__get_entry_lba(found[0], found[1], PRESENT, NOT_EMPTY)
            if first_lba is not None:
                attr = self.__get_entry_attr(found[0], found[1])
                self.__delete_part_chain(first_lba, attr)
            self.__set_entry_lba(found[0], found[1], 0, 0)
            dir_size = self.__get_part_size(found[0])
            self.__set_part_size(found[0], dir_size - 1)

    def import_file(self, ex_path, in_path):
        if os.path.isfile(ex_path):
            in_f = self.open(in_path, 'w')
            ex_f = open(ex_path)
            data = ex_f.read(MAX_READ)
            while data:
                in_f.write(data)
                data = ex_f.read(MAX_READ)
            in_f.close()
            ex_f.close()

    def import_dir(self, ex_path, in_path):
        if os.path.isdir(ex_path):
            for sub in os.walk(ex_path):
                partial_path = sub[0][len(ex_path):]
                for filename in sub[2]:
                    self.import_file(os.path.join(sub[0], filename), os.path.join(in_path, partial_path, filename))
                for dir in sub[1]:
                    self.make_dir(os.path.join(in_path, partial_path, dir))

    def export_file(self, in_path, ex_path):
        ex_f = open(ex_path, 'w')
        in_f = self.open(in_path)
        data = in_f.read(MAX_READ)
        while data:
            ex_f.write(data)
            data = in_f.read(MAX_READ)
        ex_f.close()
        in_f.close()

    def close(self, f):
        if f in self.open_files:
            self.open_files.remove(f)

    def create(self, path, is_dir=False, overwrite=True):
        path_parts = EdenFS.path_parts(path)
        if len(path_parts) > 1:
            found = self.__find_path(path_parts[:-1], IS_DIR)
            if found is None:
                raise Exception('FS Error', 'Path does not exist!')
            found_attr = self.__get_entry_attr(found[0], found[1])
            if not EdenFS.__has_attr(found_attr, NOT_EMPTY):
                self.__set_entry_lba(found[0], found[1], self.balloc(), PRESENT, IS_DIR, NOT_EMPTY)
            cur_lba = self.__get_entry_lba(found[0], found[1])
        else:
            cur_lba = self.root_lba

        found = self.__find_in_dir(cur_lba, path_parts[-1], PRESENT)

        if found is None:
            found = self.__get_empty_entry(cur_lba)
        elif overwrite:
            pre_lba = self.__get_entry_lba(found[0], found[1])
            pre_attr = self.__get_entry_attr(found[0], found[1])
            if EdenFS.__has_attr(pre_attr, NOT_EMPTY):
                self.__delete_part_chain(pre_lba, pre_attr)
        else:
            raise Exception('FS Error', 'File or directory already exist!')
        base_lba = found[0]
        entry_offset = found[1]
        attr = [PRESENT]
        if is_dir:
            attr.append(IS_DIR)
        self.__set_entry_name(base_lba, entry_offset, path_parts[-1])
        self.__set_entry_attr(base_lba, entry_offset, *attr)
        return base_lba, entry_offset

    def balloc(self, clear=True):
        lba = self.__find_in_level(0, self.first_bitmap_lba, 0)
        if clear:
            self.dev.write_blocks(lba=lba, data='\x00' * BLOCK_SIZE_BYTES, block_size=BLOCK_SIZE_BYTES)
        return lba

    def bfree(self, lba):
        for level in range(self.levels):
            base_lba = self.first_bitmap_lba + sum(self.levels_count[:self.levels - level - 1])
            offset = lba / BITMAP_SIZE_BITS
            i = (lba % BITMAP_SIZE_BITS) / 8
            b = (lba % BITMAP_SIZE_BITS) % 8
            b_lba = base_lba + offset
            bitmap = self.dev.read_blocks(lba=b_lba, count=1, block_size=BLOCK_SIZE_BYTES)
            n = struct.unpack('B', bitmap[i])[0]
            n &= (~ (1 << b)) & 0xFF
            bitmap = bitmap[:i] + struct.pack('B', n) + bitmap[i + 1:]
            self.dev.write_blocks(lba=b_lba, data=bitmap, block_size=BLOCK_SIZE_BYTES)
            lba /= BITMAP_SIZE_BITS

    def write_to_file(self, entry_data, data, seek=0):
        found = entry_data
        if found is None:
            raise Exception('FS Error', 'Path does not exist!')
        found_attr = self.__get_entry_attr(found[0], found[1])
        if EdenFS.__has_attr(found_attr, IS_DIR):
            raise Exception('FS Error', 'The target is a directory!')
        if not EdenFS.__has_attr(found_attr, NOT_EMPTY):
            cur_lba = self.balloc()
            self.__set_entry_lba(found[0], found[1], cur_lba, PRESENT, NOT_EMPTY)
        else:
            cur_lba = self.__get_entry_lba(found[0], found[1])
        self.write_to_parts(cur_lba, data, seek)

    def write_to_parts(self, part_lba, data, seek):
        seek_parts = seek / FILE_DATA_PER_BLOCK
        seek_in_part = seek % FILE_DATA_PER_BLOCK

        while seek_parts:
            self.__set_part_size(part_lba, FILE_DATA_PER_BLOCK)
            next_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
            if next_lba is None:
                next_lba = self.balloc()
                self.__set_next_part_lba(part_lba, next_lba, PRESENT, NOT_EMPTY)
            part_lba = next_lba
            seek_parts -= 1

        while data:
            addr = part_lba * BLOCK_SIZE_BYTES + seek_in_part + 2
            size_in_part = FILE_DATA_PER_BLOCK - seek_in_part
            if size_in_part >= len(data):
                self.dev.write_blocks(lba=addr, data=data, block_size=1)
                data_end = seek_in_part + len(data)
                part_size = self.__get_part_size(part_lba)
                if data_end > part_size:
                    self.__set_part_size(part_lba, data_end)
                return
            else:
                self.dev.write_blocks(lba=addr, data=data[:size_in_part], block_size=1)
                self.__set_part_size(part_lba, FILE_DATA_PER_BLOCK)
                next_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
                if next_lba is None:
                    next_lba = self.balloc()
                    self.__set_next_part_lba(part_lba, next_lba, PRESENT, NOT_EMPTY)
                part_lba = next_lba
                data = data[size_in_part:]
                seek_in_part = 0

    def read_from_file(self, entry_data, count=None, seek=0):
        found = entry_data
        if found is None:
            raise Exception('FS Error', 'Path does not exist!')
        found_attr = self.__get_entry_attr(found[0], found[1])
        if EdenFS.__has_attr(found_attr, IS_DIR):
            raise Exception('FS Error', 'The target is a directory!')
        if not EdenFS.__has_attr(found_attr, NOT_EMPTY):
            return ''
        cur_lba = self.__get_entry_lba(found[0], found[1])
        return self.read_from_parts(cur_lba, count, seek)

    def read_from_parts(self, part_lba, count=None, seek=0):
        seek_parts = seek / FILE_DATA_PER_BLOCK
        seek_in_part = seek % FILE_DATA_PER_BLOCK

        while seek_parts:
            self.__set_part_size(part_lba, FILE_DATA_PER_BLOCK)
            next_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
            if next_lba is None:
                next_lba = self.balloc()
                self.__set_next_part_lba(part_lba, next_lba, PRESENT, NOT_EMPTY)
            part_lba = next_lba
            seek_parts -= 1

        data = ''
        while True:
            part_size = self.__get_part_size(part_lba) - seek_in_part
            addr = part_lba * BLOCK_SIZE_BYTES + seek_in_part + 2
            if count is None:
                data += self.dev.read_blocks(lba=addr, count=part_size, block_size=1)
                next_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
                if next_lba is None:
                    return data
            elif part_size >= count:
                data += self.dev.read_blocks(lba=addr, count=count, block_size=1)
                return data
            else:
                if part_size < FILE_DATA_PER_BLOCK:
                    data += self.dev.read_blocks(lba=addr, count=part_size, block_size=1)
                    return data
                else:
                    data += self.dev.read_blocks(lba=addr, count=FILE_DATA_PER_BLOCK, block_size=1)
                    next_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
                    if next_lba is None:
                        return data
                    count -= FILE_DATA_PER_BLOCK
                    seek_in_part = 0
            part_lba = next_lba

    def get_size(self, path):
        found = self.find_path(path)
        if found is None:
            raise Exception('FS Error', 'Path does not exist!')
        found_attr = self.__get_entry_attr(found[0], found[1])
        if not EdenFS.__has_attr(found_attr, NOT_EMPTY):
            return 0
        lba = self.__get_entry_lba(found[0], found[1])
        return self.__get_size(lba, found_attr)

    def __get_size(self, part_lba, attr):
        size = 0
        if EdenFS.__has_attr(attr, IS_DIR):
            while True:
                dir_size = self.__get_part_size(part_lba)
                c = 0
                for entry in xrange(0, DIR_ENTRIES_PER_PART):
                    if c >= dir_size:
                        break
                    entry_offset = entry * DIR_ENTRY_SIZE + DIR_ENTRIES_OFFSET
                    entry_attr = self.__get_entry_attr(part_lba, entry_offset)
                    if EdenFS.__has_attr(entry_attr, PRESENT):
                        c += 1
                    else:
                        continue
                    lba = self.__get_entry_lba(part_lba, entry_offset, PRESENT, NOT_EMPTY)
                    if lba is not None:
                        size += self.__get_size(lba, entry_attr)
                next_part_lba = self.__get_next_part_lba(part_lba, PRESENT, IS_DIR, NOT_EMPTY)
                if next_part_lba is None:
                    return size
                part_lba = next_part_lba
        size += self.__get_part_size(part_lba)
        next_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
        while next_lba is not None:
            size += self.__get_part_size(next_lba)
            next_lba = self.__get_next_part_lba(next_lba, PRESENT, NOT_EMPTY)
        return size

    def __list(self, part_lba, tree=False, level=0, put_type=False, put_size=False):
        output = ''

        dir_size = self.__get_part_size(part_lba)
        c = 0
        for entry in xrange(0, DIR_ENTRIES_PER_PART):
            if c >= dir_size:
                break
            entry_offset = entry * DIR_ENTRY_SIZE + DIR_ENTRIES_OFFSET
            entry_attr = self.__get_entry_attr(part_lba, entry_offset)

            if EdenFS.__has_attr(entry_attr, PRESENT):
                c += 1
                entry_name = self.__get_entry_name(part_lba, entry_offset)
                if EdenFS.__has_attr(entry_attr, IS_DIR):
                    entry_type = 'DIR'
                else:
                    entry_type = 'FILE'
                size = 0
                if EdenFS.__has_attr(entry_attr, NOT_EMPTY):
                    entry_lba = self.__get_entry_lba(part_lba, entry_offset)
                    size = self.__get_size(entry_lba, entry_attr)

                if level:
                    output += '    |' * (level - 1) + '    +--- '

                output += entry_name.rstrip('\x00')
                if put_type:
                    output += ' ({type})'.format(name=entry_name.rstrip('\x00'), type=entry_type, size=size)
                if put_size:
                    output += ' ({size} bytes)'.format(size=size)
                output += '\n'

                if tree and EdenFS.__has_attr(entry_attr, IS_DIR, NOT_EMPTY):
                    output += '    |' * (level + 1) + '\n'
                    output += self.__list(self.__get_entry_lba(part_lba, entry_offset), tree=True, level=level + 1, put_type=put_type, put_size=put_size)

        next_part_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
        if next_part_lba:
            output += self.__list(next_part_lba, tree=tree, level=level, put_type=put_type, put_size=put_size)

        return output

    def find_path(self, path, *attr):
        path_parts = EdenFS.path_parts(path)
        return self.__find_path(path_parts, *attr)

    @staticmethod
    def path_parts(path):
        path_parts = path.split('/')
        while '' in path_parts:
            path_parts.remove('')
        return path_parts

    def __find_path(self, path_parts, *attr):
        cur_lba = self.root_lba
        for part in path_parts[:-1]:
            found = self.__find_in_dir(cur_lba, part, PRESENT, IS_DIR, NOT_EMPTY)
            if found is None:
                return None
            cur_lba = self.__get_entry_lba(found[0], found[1])
        found = self.__find_in_dir(cur_lba, path_parts[-1], PRESENT, *attr)
        return found

    def is_path(self, path, is_dir=None):
        path_parts = EdenFS.path_parts(path)
        return self.__is_path(path_parts, is_dir=is_dir)

    def __is_path(self, path_parts, is_dir=None):
        if not path_parts:
            return True
        found = self.__find_path(path_parts)
        if found is None:
            return False
        if is_dir is None:
            return True
        attr = self.__get_entry_attr(found[0], found[1])
        if EdenFS.__has_attr(attr, IS_DIR):
            return is_dir
        return not is_dir

    def __find_in_level(self, level, base_lba, offset):
        lba = base_lba + offset
        bitmap = self.dev.read_blocks(lba=lba, count=1, block_size=BLOCK_SIZE_BYTES)
        for i in xrange(len(bitmap)):
            if bitmap[i] != '\xFF':
                n = struct.unpack('B', bitmap[i])[0]
                for b in xrange(8):
                    if not n & (1 << b):
                        next_offset = i * 8 + b
                        if level == self.levels - 1:
                            n |= 1 << b
                            bitmap = bitmap[:i] + struct.pack('B', n) + bitmap[i + 1:]
                            self.dev.write_blocks(lba=lba, data=bitmap, block_size=BLOCK_SIZE_BYTES)
                            return offset * BITMAP_SIZE_BITS + next_offset
                        val = self.__find_in_level(level + 1, self.first_bitmap_lba + sum(self.levels_count[:level + 1]), next_offset)
                        if val is not None:
                            return val
                        n |= 1 << b
                        bitmap = bitmap[:i] + struct.pack('B', n) + bitmap[i + 1:]
                        self.dev.write_blocks(lba=lba, data=bitmap, block_size=BLOCK_SIZE_BYTES)
        return None

    def __find_in_dir(self, cur_lba, name, *attr):
        name += '\x00' * (NAME_LEN - len(name))
        while True:
            dir_size = self.__get_part_size(cur_lba)
            c = 0
            for entry in xrange(0, DIR_ENTRIES_PER_PART):
                if c >= dir_size:
                    break
                entry_offset = entry * DIR_ENTRY_SIZE + DIR_ENTRIES_OFFSET
                entry_attr = self.__get_entry_attr(cur_lba, entry_offset)
                entry_name = self.__get_entry_name(cur_lba, entry_offset)
                if EdenFS.__has_attr(entry_attr, PRESENT):
                    c += 1
                if EdenFS.__has_attr(entry_attr, *attr) and name == entry_name:
                    return cur_lba, entry_offset
            next_part_lba = self.__get_next_part_lba(cur_lba, PRESENT, IS_DIR, NOT_EMPTY)
            if next_part_lba is None:
                return None
            cur_lba = next_part_lba

    @staticmethod
    def __has_attr(entry_attr, *attr):
        return reduce(lambda x, y: x and y, [entry_attr & a for a in attr] + [True])

    def __get_next_part_lba(self, part_lba, *attr):
        offset = part_lba * BLOCK_SIZE_BYTES + NEXT_PART_OFFSET
        next_part = self.dev.read_blocks(lba=offset, count=4, block_size=1)
        next_part_lba = struct.unpack('I', next_part)[0]
        if EdenFS.__has_attr(next_part_lba, *attr):
            return next_part_lba >> 9
        return None

    def __set_next_part_lba(self, part_lba, lba, *attr):
        offset = part_lba * BLOCK_SIZE_BYTES + NEXT_PART_OFFSET
        next_part_lba = (lba << 9) | (reduce(lambda x, y: x | y, attr) if attr else 0)
        next_part = struct.pack('I', next_part_lba)
        self.dev.write_blocks(lba=offset, data=next_part, block_size=1)

    def __get_part_size(self, part_lba):
        offset = part_lba * BLOCK_SIZE_BYTES
        size_data = self.dev.read_blocks(lba=offset, count=2, block_size=1)
        return struct.unpack('H', size_data)[0]

    def __set_part_size(self, part_lba, size):
        size_data = struct.pack('H', size)
        offset = part_lba * BLOCK_SIZE_BYTES
        self.dev.write_blocks(lba=offset, data=size_data, block_size=1)

    def __get_next_part_attr(self, part_lba):
        offset = part_lba * BLOCK_SIZE_BYTES + NEXT_PART_OFFSET
        next_part = self.dev.read_blocks(lba=offset, count=4, block_size=1)
        next_part_lba = struct.unpack('I', next_part)[0]
        return next_part_lba & 0x1FF

    def __get_entry_lba(self, base_lba, entry_offset, *attr):
        offset = base_lba * BLOCK_SIZE_BYTES + entry_offset + NAME_LEN
        addr_data = self.dev.read_blocks(lba=offset, count=4, block_size=1)
        addr = struct.unpack('I', addr_data)[0]
        if EdenFS.__has_attr(addr, *attr):
            return addr >> 9
        return None

    def __set_entry_lba(self, base_lba, entry_offset, lba, *attr):
        addr = (lba << 9) | reduce(lambda x, y: x | y, attr)
        packed = struct.pack('I', addr)
        offset = base_lba * BLOCK_SIZE_BYTES + entry_offset + NAME_LEN
        self.dev.write_blocks(offset, data=packed, block_size=1)

    def __get_entry_attr(self, base_lba, entry_offset):
        offset = base_lba * BLOCK_SIZE_BYTES + entry_offset + NAME_LEN
        addr = self.dev.read_blocks(lba=offset, count=4, block_size=1)
        return struct.unpack('I', addr)[0] & 0x1FF

    def __set_entry_attr(self, base_lba, entry_offset, *attr):
        lba = self.__get_entry_lba(base_lba, entry_offset)
        self.__set_entry_lba(base_lba, entry_offset, lba, *attr)

    def __get_entry_name(self, base_lba, entry_offset):
        offset = base_lba * BLOCK_SIZE_BYTES + entry_offset
        name = self.dev.read_blocks(lba=offset, count=NAME_LEN, block_size=1)
        return name

    def __set_entry_name(self, base_lba, entry_offset, name):
        if len(name) < 12:
            name += '\x00' * (NAME_LEN - len(name))
            offset = base_lba * BLOCK_SIZE_BYTES + entry_offset
            self.dev.write_blocks(lba=offset, data=name, block_size=1)

    def __get_empty_entry(self, cur_lba):
        while True:
            dir_size = self.__get_part_size(cur_lba)
            if dir_size < DIR_ENTRIES_PER_PART:
                for entry in xrange(0, DIR_ENTRIES_PER_PART):
                    entry_offset = entry * DIR_ENTRY_SIZE + DIR_ENTRIES_OFFSET
                    entry_attr = self.__get_entry_attr(cur_lba, entry_offset)
                    if not EdenFS.__has_attr(entry_attr, PRESENT):
                        self.__set_part_size(cur_lba, dir_size + 1)
                        return cur_lba, entry_offset
            else:
                next_part_lba = self.__get_next_part_lba(cur_lba, PRESENT, IS_DIR, NOT_EMPTY)
                if next_part_lba is None:
                    return None
                next_part_lba = self.balloc()
                self.__set_next_part_lba(cur_lba, next_part_lba, PRESENT, IS_DIR, NOT_EMPTY)
                cur_lba = next_part_lba

    def __delete_part_chain(self, part_lba, part_attr):
        next_part_lba = self.__get_next_part_lba(part_lba, PRESENT, NOT_EMPTY)
        next_part_attr = self.__get_next_part_attr(part_lba)
        if next_part_lba is not None:
            self.__delete_part_chain(next_part_lba, next_part_attr)
        if EdenFS.__has_attr(part_attr, IS_DIR):
            dir_size = self.__get_part_size(part_lba)
            c = 0
            for entry in xrange(0, DIR_ENTRIES_PER_PART):
                if c >= dir_size:
                    break
                entry_offset = entry * DIR_ENTRY_SIZE + DIR_ENTRIES_OFFSET
                entry_attr = self.__get_entry_attr(part_lba, entry_offset)
                if EdenFS.__has_attr(entry_attr, PRESENT):
                    c += 1
                else:
                    continue
                if EdenFS.__has_attr(entry_attr, NOT_EMPTY):
                    entry_lba = self.__get_entry_lba(part_lba, entry_offset)
                    self.__delete_part_chain(entry_lba, entry_attr)
        self.bfree(part_lba)


class File:
    def __init__(self, fs, path, fmt='r'):
        self.fs = fs
        self.path = path

        if fmt == 'w':
            self.entry_data = fs.create(path)
        else:
            found = self.fs.find_path(path, PRESENT)
            if found is None:
                raise Exception('FS Error', 'File does not exist!')
            self.entry_data = found

        self.r_seek = 0
        self.w_seek = 0

    def __eq__(self, other):
        return self.path == other.path

    def read(self, count=None, seek=0, seek_from=CURRENT_SEEK):
        if seek < 0:
            raise Exception('File Error', 'Illegal seek value')
        if seek_from == CURRENT_SEEK:
            self.r_seek += seek
        elif seek_from == START_SEEK:
            self.r_seek = seek
        else:
            raise Exception('File Error', 'Illegal seek mode!')

        data = self.fs.read_from_file(self.entry_data, count=count, seek=self.r_seek)
        self.r_seek += len(data)

        return data

    def write(self, data, seek=0, seek_from=0):
        if seek < 0:
            raise Exception('File Error', 'Illegal seek value')
        if seek_from == CURRENT_SEEK:
            self.r_seek += seek
        elif seek_from == START_SEEK:
            self.r_seek = seek
        else:
            raise Exception('File Error', 'Illegal seek mode!')

        self.fs.write_to_file(self.entry_data, data=data, seek=self.w_seek)
        self.w_seek += len(data)

    def close(self):
        self.fs.close(self)

# endregion
