#include <stdio.h>
#include <stdlib.h>
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
int padded_size_of_block = PADDED_SIZE(sizeof(struct block));

void split_space(struct block *current_node, int requested_size) {
    int available_space = PADDED_SIZE(current_node->size);
    int required_space = PADDED_SIZE(requested_size) + PADDED_SIZE(sizeof(struct block)) + ALIGNMENT;

    //comment for me: this checks if theres enough space, if there isnt it dosent need to allocate memory/do anything
    if (available_space < required_space) {
        return;
    }

    struct block *new_node = PTR_OFFSET(current_node, PADDED_SIZE(requested_size) + PADDED_SIZE(sizeof(struct block)));
    new_node->size = available_space - required_space;
    new_node->in_use = 0;
    new_node->next = current_node->next;
    current_node->next = new_node;
    current_node->size = PADDED_SIZE(requested_size);
    current_node->in_use = 1;
}


void *myalloc(int size) {
    int padded_node_size = PADDED_SIZE(size);
    //I feel like having 2 differentish required space is icky but oh well
    int required_space = padded_node_size + padded_size_of_block + 16;

    if (head == NULL) {
        head = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        head->next = NULL;
        head->size = 1024 - PADDED_SIZE(sizeof(struct block));
        head->in_use = 0;
    }

    struct block *cur = head;
    while (cur != NULL) {
        if (cur->size >= padded_node_size && !cur->in_use) {
            if (cur->size >= required_space) {
                split_space(cur, padded_node_size);
            }
            cur->in_use = 1;
            int padded_block_size = padded_size_of_block;
            return PTR_OFFSET(cur, padded_block_size);
        }
        cur = cur->next;
    }
    return NULL;
}

void myfree(void *p) {
    struct block *accessed_node = p - padded_size_of_block;
    accessed_node->in_use = 0;
}

void print_data(void){
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

    p = myalloc(512);
    print_data();

    myfree(p);
    print_data();
}