#include "memory.h"

static unsigned char static_heap[HEAP_SIZE];
static struct MemBlock* heap_head = NULL;

void memory_init(void *start, unsigned long size) {
	heap_head = (struct MemBlock*)start;
	heap_head->size = size - sizeof(struct MemBlock);
	heap_head->is_free = 1;
	heap_head->next = NULL;
}

static void lazy_init() {
	if (!heap_head) {
		memory_init((void*)static_heap, HEAP_SIZE);
	}
}

static struct MemBlock* locate_fit(unsigned long req_size) {
	struct MemBlock *scan = heap_head;

search_again:
	if (!scan) return NULL;

	if (!(scan->is_free && scan->size >= req_size)) {
		scan = scan->next;
		goto search_again;
	}

	if (scan->size > req_size + sizeof(struct MemBlock) + 8) {
		struct MemBlock *split = (struct MemBlock*)((char*)scan +
			sizeof(struct MemBlock) + req_size);
		split->size = scan->size - req_size - sizeof(struct MemBlock);
		split->is_free = 1;
		split->next = scan->next;

		scan->size = req_size;
		scan->next = split;
	}

	scan->is_free = 0;
	return scan;
}

void* alloc(unsigned long size) {
	if (!size) return NULL;

	lazy_init();
	size = (size + 7) & ~7;

	struct MemBlock *block = locate_fit(size);
	if (block) {
		return (void*)((char*)block + sizeof(struct MemBlock));
	}
	return NULL;
}

void release(void *ptr) {
	if (!ptr) return;

	struct MemBlock *block = (struct MemBlock*)((char*)ptr - sizeof(struct MemBlock));
	block->is_free = 1;

	if (block->next && block->next->is_free) {
		block->size += block->next->size + sizeof(struct MemBlock);
		block->next = block->next->next;
	}

	struct MemBlock *walker = heap_head;

backtrack:
	if (!walker || walker->next == block) goto merge_check;

	walker = walker->next;
	goto backtrack;

merge_check:
	if (walker && walker->is_free) {
		walker->size += block->size + sizeof(struct MemBlock);
		walker->next = block->next;
	}
}
