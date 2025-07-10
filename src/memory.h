#ifndef MEMORY_H
#define MEMORY_H

#define NULL ((void*)0)
#define HEAP_SIZE 0x500000

struct MemBlock {
	unsigned long size;
	unsigned char is_free;
	struct MemBlock *next;
};

void memory_init(void *start, unsigned long size);
void* alloc(unsigned long size);
void release(void *ptr);

#endif
