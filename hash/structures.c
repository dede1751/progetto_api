#include "structures.h"

static uint32_t hash(char *, uint32_t);

static void swap(node_ptr *, node_ptr *);
static node_ptr *partition(node_ptr *, node_ptr *);

static void empty_blocks(node_ptr);


dict_ptr generate_dict(){
    dict_ptr dict = (dict_ptr)malloc(sizeof(dict_t));
    node_ptr *t;
    int i;

    dict->size = BASE_SIZE;
    dict->items = 0;
    dict->collisions = 0;
    dict->head = NULL;

    // NULL == (void *) 0 != 0, calloc makes this platform dependent
    t = (node_ptr *)malloc(BASE_SIZE * sizeof(node_ptr));
    for (i = 0; i < BASE_SIZE; ++i) t[i] = NULL;
    dict->table = t;

    return dict;
}


static uint32_t hash(char * key, uint32_t size){
    uint32_t h = 3323198485ul;
    for (; *key; ++key) {
        h ^= *key;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h % size;
}


node_ptr insert(dict_ptr D, char *s){
    node_ptr new = (node_ptr)malloc(sizeof(node_t));
    uint32_t h = hash(s, D->size);

    new->word = s;
    new->next = NULL;
    if (D->table[h] == NULL){
        D->table[h] = new;
        new->chain = NULL;
    } else { // head insert
        new->chain = D->table[h];
        D->table[h] = new;
        ++(D->collisions);
    }
    ++(D->items);

    return new;
}


uint8_t search(dict_ptr D, char *s){
    uint32_t h = hash(s, D->size);
    node_ptr curr = D->table[h];

    while (curr != NULL){
        if (strcmp(s, curr->word) == 0) return 1;
        curr = curr->chain;
    };
    return 0;
}


static void swap(node_ptr *src, node_ptr *tgt){
    node_ptr temp = *src;

    *src = *tgt;
    *tgt = temp;
}

static node_ptr *partition(node_ptr *start, node_ptr *end){
    char *pivot;

    // Hoare partitioning, middle element as pivot
    // (start + end) / 2 returns error for obscure reason
    pivot = (*(start + (end - start) / 2))->word;
    --start;
    ++end;

    while (1){
        while (strcmp((*(++start))->word, pivot) < 0);
        while (strcmp((*(--end))->word, pivot) > 0);

        if (start >= end) return end;
        swap(start, end);
    }
}

void quicksort(node_ptr *start, node_ptr *end){
    node_ptr *pivot;

    if (start < end){
        pivot = partition(start, end);
        quicksort(start, pivot);
        quicksort(pivot + 1, end);
    }
}


// this can never be performed on an empty list
void sequential_insert(dict_ptr D, node_ptr *buffer, int len){
    node_ptr prev, curr;
    int i = 0;

    curr = D->head;
    prev = NULL;
    while (curr != NULL && i < len){
        // buffer[i] goes before curr, insert and stay put
        if (strcmp(curr->word, buffer[i]->word) > 0){
            if (prev == NULL) D->head = buffer[i];
            else prev->next = buffer[i];

            buffer[i]->next = curr;
            prev = buffer[i];
            ++i;
            ++(D->len);
        } else { // buffer[i] goes after curr, move down list
            prev = curr;
            curr = curr->next;
        }
    }
    while (i < len){
        // since list has at least one element, prev is never NULL in loop
        prev->next = buffer[i];
        prev = buffer[i];
        ++i;
        ++(D->len);
    }
}


void print_list(dict_ptr D, FILE *output){
    node_ptr curr;

    for (curr = D->head; curr != NULL; curr = curr->next){
        fputs(curr->word, output);
        fputs("\n", output);
    }
}


// free pointers
static void empty_blocks(node_ptr curr){ //linear traversal, resets .next fields
    if (curr->next != NULL) {
        empty_blocks(curr->next);
        curr->next = NULL;
    }
}
void reset_list(dict_ptr D){
    if (D->head != NULL){
        empty_blocks(D->head);
        D->head = NULL;
        D->len = 0;
    }
}

void free_dict(dict_ptr D){
    node_ptr curr, temp;
    int i;

    for (i = 0; i < D->size; ++i){
        curr = D->table[i];
        while(curr != NULL){
            temp = curr;
            curr = curr->chain;
            free(temp->word);
            free(temp);
        }
    }
    free(D->table);
    free(D);
}
