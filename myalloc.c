#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#define ALIGNMENT 16 
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))


struct block {
    struct block *next;
    int size; //tried to get fancy and use size_t and failed
    int in_use;
};

struct block *head = NULL;

void *myalloc(int size) {
    if (head == NULL) {
        head = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        head->next = NULL;
        head->size = 1024 - PADDED_SIZE(sizeof(struct block));
        head->in_use = 0;
    }

    struct block *cur = head;
    
    while (cur != NULL && cur->in_use != 1) {
        if (cur->size >= size) {
            cur->in_use = 1;
            int padded_block_size = PADDED_SIZE(sizeof(struct block));
            return PTR_OFFSET(cur, padded_block_size);
        }
        cur = cur->next;
    }
    return NULL;
}

void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main(void) {
    void *p;
    print_data();
    p = myalloc(16);
    print_data();
    p = myalloc(16);
    printf("%p\n", p);
}