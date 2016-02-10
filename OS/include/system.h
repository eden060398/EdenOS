#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef STDDEF_H
#define STDDEF_H
	#include <stddef.h>
#endif

#define TAB 4

#define INB(ret, port) asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port))
#define OUTB(port, val) asm volatile ("outb %%al,%%dx": :"d" (port), "a" (val))
#define CLEAR_INTS() asm volatile ("cli")
#define SET_INTS() asm volatile ("sti")
#define HALT() asm volatile ("hlt")
#define PUSH(x) asm ("pushl %%eax": :"a" (x))
#define POP(x) asm ("popl %%eax" : "=a" (x))

enum ERR_TYPE {
	KENEL_ERR_INVALID_PARAMS = 0x00
};

enum BASE {
	BASE2 = 2,
	BASE10 = 10,
	BASE16 = 16
};

const char *TITLE;

void init_interrupts(void);

void init_idt(void);

void *memset(void *str, int c, uint32_t n);
char *itoa(int n, char *buff, int base);
char *uitoa(uint32_t n, char *buff, int base);
int strcmp(const char *s1, const char *s2);
void *memcpy(void *dest, void *src, uint32_t n);
char *strcpy(char *dest, const char *src);
size_t strlen(const char *s);
uint32_t atoui(char *buff);
int startswith(char *s1, char *s2);

void init_console(void);
void set_cursor(int row, int column);
void putc(int);
void puts(const char*);
void putc_at(int c, int row, int column);
void puts_at(const char *s, int row, int column);
void clear_screen(void);


void init_keyboard(void);
void handle_keystroke(void);
int getc(void);
char *gets(char *buff, int max);

void init_time(void);
void handle_tick(void);
void init_switch(unsigned int count);

void init_command_prompt(void);

void init_palloc(void);
void print_bios_mmap(void);
void *palloc(void);
void pfree(void *ptr);

void init_malloc(void);
void *malloc(size_t size);
void free(void *ptr);

void init_paging(void);
void create_entry(uint32_t dir_addr, uint32_t laddr, uint32_t paddr, int read_write, int user_supervisor, uint8_t flags);
void remove_entry(uint32_t dir_addr, uint32_t laddr);
uint32_t dup_dir(uint32_t new_dir, uint32_t ori_dir);

void init_processing(void);
unsigned int switch_proc(void);
int new_thread(const char *name, uint32_t **status);
void dispose_thread(void);
void wait(uint32_t *status);
void print_proc_data(int with_pid, int with_ppid, int with_status, int with_threads, int with_tid, int with_tstatus);
void set_idle(void);
void set_active(void);
void kill_th(uint32_t pid, uint32_t tid);