#ifndef PROCESSING_H
#define PROCESSING_H

#include <system.h>
#include <paging.h>

// DEFINITIONS

// ENUMERATIONS

enum STATUS
{
	DEAD = 0,
	IDLE,
	ACTIVE
};

// STRUCTURES

typedef struct page_node {
    uint32_t page;
    struct page_node *next;
} __attribute__((packed)) Page;

typedef struct process_node ProcessNode;
typedef struct thread_node ThreadNode;

typedef struct process_node {
    const char *name;
    const char *source;
    uint32_t pid;
    uint32_t ppid;
    uint32_t cr3;
    uint32_t status;
    uint32_t next_tid;
    Page *pages;
    ThreadNode *main_thread;
    ProcessNode *next;
} __attribute__((packed)) ProcessNode;

typedef struct thread_node {
    const char *name;
    ProcessNode *proc;
    uint32_t status;
    uint32_t tid;
    uint32_t stack_page;
    uint32_t p_stack_page;
    uint32_t stack_pointer;
    uint32_t init;
    ThreadNode *next;
} __attribute__((packed)) ThreadNode;

#endif /* PROCESSING_H */

