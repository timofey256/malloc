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

void split_block(header_t* block, size_t size) {
        header_t* new_block = block + size;
        new_block->size = block->size - size;
        new_block->next = block->next;
        new_block->is_free = 1;
        
        block->size = size;
        block->next = new_block;
}

header_t* find_free_block(header_t* first_block, size_t size) {
    header_t* free_block = NULL; // the smallest block which can contain [size]
    header_t* current = first_block;
    while (current) {
        if (current->is_free) {
            free_block = current;
            if (current->size >= 2*size) {
                split_block(current, size);
            }
            return free_block;
        }
        current = current->next;
    }
    return free_block;
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

void free(void* p) {
    if (p == NULL) {
        return;
    }

    header_t* h = p - HEADER_SIZE;
    h->is_free = 1;
    
    header_t* cur = first_block;
    while(cur) {
        if (cur->next) {
            if (cur->is_free && cur->next->is_free) {
                cur->size += cur->next->size;
                cur->next = cur->next->next;
            }
        }
        cur = cur->next;
    }

}

void test() {
    int* a = malloc_t(sizeof(int));
    int* b = malloc_t(sizeof(int));
    *a = 10;
    *b = 20;
    free(a);
    free(b);
    LOG();
    char* c = malloc_t(sizeof(char));
    *c = 'a';
    LOG();
}

int main(int argc, char** argv) {
    test();
    return 0;
}