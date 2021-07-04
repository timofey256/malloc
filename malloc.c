#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

typedef struct header {
  size_t size;
  struct header* next;
  int is_free;
} header_t;

#define HEADER_SIZE sizeof(header_t);

header_t* first_block = NULL;

void LOG() {
    header_t* current = first_block; 
    printf("\n---------LOG----------\n");
    while (current) {
        printf("Address: [%p]. Data: [%ld]\n", current, current->size);
        current = current->next;
    }
    printf("----------------------\n\n");
}

header_t* find_free_block(header_t* first_block, size_t size) {
    header_t* best = NULL; // the smallest block which can contain [size]
    header_t* current = first_block;
    while (current) {
        if (current->is_free && (best==NULL || (current->size < best->size && current->size >= size))) {
            best = current;
        }
        current = current->next;
    }
    return best;
}

header_t* request_memory(header_t* last, size_t size) {
    header_t* header;
    header = sbrk(0);
    void *request = sbrk(size);
    assert((void*)header == request);
    if (header == (void*)-1) {
        return NULL;
    }
    
    if (last) {
        last->next = header;
    }
    else {  
        first_block = header;
    }
    
    header->size = size;
    header->next = NULL;
    header->is_free = 0;
    
    return header;
}  

void* malloc_t(size_t size) {    
    header_t* block;
    static header_t* last;
    
    if (size <= 0) {
        return NULL;
    }

    size += HEADER_SIZE; 
    
    if (!first_block) {
        block = request_memory(NULL, size);
        if (!block) {
            return NULL;
        }
    }
    else {
        block = find_free_block(first_block, size);
        if (!block) {
            block = request_memory(last, size);
            if (!block) {
                return NULL;
            }
        }
        else {
            block->is_free = 0;
        }
    }

    last = block;
    return (block+1);
}

void test() {
    int* a = malloc_t(sizeof(int));
    *a = 10;
    int* b = malloc_t(sizeof(int));
    *b = 15;
    printf("Address: [%p]. Data: [%d]\n", a, *a);
    printf("Address: [%p]. Data: [%d]\n", b, *b);
    printf("[%ld]\n", b-a);
    printf("header size: [%d]", sizeof(header_t));
}

int main(int argc, char** argv) {
    test();
    return 0;
}