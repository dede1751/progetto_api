#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define BASE_SIZE 2097152
#define LIMIT_RATIO 0.5867f
#define CHARSET 64
#define BUFSIZE 256


typedef struct n {
    char *word;
    struct n *chain;
    struct n *next;
} node_t;
typedef node_t *node_ptr;

typedef struct d {
    node_ptr *table;
    int size;
    int items;
    int collisions;

    node_ptr head;
    int len;
} dict_t;
typedef dict_t *dict_ptr;

typedef struct reqs {
    char *match;
    int8_t occ[CHARSET];
    uint8_t *pos[CHARSET];
} req_t;

typedef req_t *req_ptr;


dict_ptr generate_dict();

uint32_t hash(char *, uint32_t);
node_ptr insert(dict_ptr, char*);
uint8_t search(dict_ptr, char*);

void swap(node_ptr *, node_ptr *);
node_ptr *partition(node_ptr *, node_ptr *);
void quicksort(node_ptr *, node_ptr *);
void sequential_insert(dict_ptr, node_ptr *, int);
void print_list(dict_ptr);

void empty_blocks(node_ptr);
void reset_list(dict_ptr);
void free_dict(dict_ptr);


void safe_fgets(char *, int);
void safe_scanf(int *);
uint8_t map_charset(char);
char *calculate_eval(char *, char *, int);

void generate_req(req_ptr, int);
void calculate_req(char *, char *, req_ptr);
uint8_t check_req(char *, req_ptr);
void filter_req(dict_ptr, req_ptr);
void free_req(req_ptr);

void create_list(dict_ptr, req_ptr);
void check_guess(dict_ptr, char *, char *, int, req_ptr);
void handle_insert(dict_ptr, int, req_ptr);
uint8_t new_game(dict_ptr, int);


double insert_time = 0.0;
double insert_ord_time = 0.0;
double search_time = 0.0;
double fill_time = 0.0;
double filter_time = 0.0;
double total_time = 0.0;
double print_time = 0.0;

int main(){
    dict_ptr word_dict = generate_dict();
    char *buff;
    int wordsize, restart;
    int i;
    clock_t start = clock();

    safe_scanf(&wordsize);
    buff = (char *) malloc((wordsize + 1) * sizeof(char));
    safe_fgets(buff, wordsize);
    while (buff[0] != '+'){
        if (insert(word_dict, buff) == NULL) exit(EXIT_FAILURE);

        buff = (char *) malloc((wordsize + 1) * sizeof(char));
        getchar();
        safe_fgets(buff, wordsize);
    }
    if (wordsize <= 14) while (getchar() != '\n');
    free(buff);

    i = 1;
    do {
        restart = new_game(word_dict, wordsize);
        fprintf(stderr, "\nGAME %d FINISHED", i);
        ++i;
    } while(restart == 1);

    free_dict(word_dict);

    clock_t stop = clock();
    total_time = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    fprintf(stderr, "\n\nTOTAL TIME:%lf\nINSERT: %lf\nORDERED INS: %lf\nSEARCH: %lf\nFILL: %lf\nFILTER: %lf\nPRINT: %lf\n",
    total_time, insert_time, insert_ord_time, search_time, fill_time, filter_time, print_time);

    exit(EXIT_SUCCESS);
}


dict_ptr generate_dict(){
    dict_ptr dict = (dict_ptr)malloc(sizeof(dict_t));
    node_ptr *t;
    int i;

    dict->size = BASE_SIZE;
    dict->items = 0;
    dict->collisions = 0;
    dict->head = NULL;

    t = (node_ptr *)malloc(BASE_SIZE * sizeof(node_ptr));
    for (i = 0; i < BASE_SIZE; ++i) t[i] = NULL;
    dict->table = t;

    return dict;
}


uint32_t hash(char * key, uint32_t size){
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
    clock_t start = clock();

    new->word = s;
    new->next = NULL;
    if (D->table[h] == NULL){
        D->table[h] = new;
        new->chain = NULL;
    } else {
        new->chain = D->table[h];
        D->table[h] = new;
        ++(D->collisions);
    }
    ++(D->items);

    clock_t stop = clock();
    insert_time = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
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


void swap(node_ptr *src, node_ptr *tgt){
    node_ptr temp = *src;

    *src = *tgt;
    *tgt = temp;
}

node_ptr *partition(node_ptr *start, node_ptr *end){
    char *pivot;

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


void sequential_insert(dict_ptr D, node_ptr *buffer, int len){
    node_ptr prev, curr;
    int i = 0;

    curr = D->head;
    prev = NULL;
    while (curr != NULL && i < len){
        if (strcmp(curr->word, buffer[i]->word) > 0){
            if (prev == NULL) {
                D->head = buffer[i];
                prev = buffer[i];
            } else prev->next = buffer[i];

            buffer[i]->next = curr;
            ++i;
            ++(D->len);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    while (i < len){
        prev->next = buffer[i];
        prev = buffer[i];
        ++i;
        ++(D->len);
    }
}


void print_list(dict_ptr D){
    node_ptr curr;

    for (curr = D->head; curr != NULL; curr = curr->next) puts(curr->word);
}


void empty_blocks(node_ptr curr){
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

void safe_fgets(char *s, int size){
    if (fgets(s, size + 1, stdin) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(int *x){
    if (scanf("%d\n", x) == 0) exit(EXIT_FAILURE);
}

uint8_t map_charset(char c){
    if (c == '-') return 0;
    else if (c >= '0' && c <= '9') return (c - 47);
    else if (c >= 'A' && c <= 'Z') return (c - 53);
    else if (c >= 'a' && c <= 'z') return (c - 59);
    else return 37;
}


char *calculate_eval(char *ref, char *s, int wordsize){
    char *eval = (char *)calloc(wordsize + 1, sizeof(char));
    uint8_t counts[CHARSET] = {0};
    int i;

    for(i = 0; i < wordsize; ++i) ++(counts[map_charset(ref[i])]);

    for (i = 0; i < wordsize; ++i){
        if (ref[i] == s[i]) {
            --(counts[map_charset(ref[i])]);
            eval[i] = '+';
        } else if (counts[map_charset(s[i])] == 0) eval[i] = '/';    
    }

    for (i = 0; i < wordsize; ++i){
        if (eval[i] == 0){
            if (counts[map_charset(s[i])] > 0){
                eval[i] = '|';
                --(counts[map_charset(s[i])]);
            } else eval[i] = '/';
        }
    }
    return eval;
}


void generate_req(req_ptr reqs, int wordsize){
    char *matches;
    uint8_t *p;
    int i, j;

    matches = (char *)malloc((wordsize + 1) * sizeof(char));
    for (i = 0; i < wordsize; ++i) matches[i] = '*';
    matches[wordsize] = '\0';
    reqs->match = matches;

    for (i = 0; i < CHARSET; ++i) {
        p = (uint8_t *)malloc((wordsize) * sizeof(uint8_t));
        for (j = 0; j < wordsize; ++j) p[j] = 1;

        reqs->pos[i] = p;        
        reqs->occ[i] = -1;
    }
}


void calculate_req(char *s, char *eval, req_ptr reqs){
    int8_t counts[CHARSET];
    char c;
    int i, index;

    for (i = 0; i < CHARSET; ++i) counts[i] = -1;

    for (i = 0; s[i] != '\0'; ++i){
        c = eval[i];
        index = map_charset(s[i]);

        if (c == '+'){
            reqs->match[i] = s[i];
            if (counts[index] < 0) --(counts[index]);
            else ++(counts[index]);
        } else if (c == '|'){
            if (counts[index] < 0) --(counts[index]);
            reqs->pos[index][i] = 0;
        } else if (c == '/'){
            if (counts[index] < 0) {
                counts[index] = -(counts[index]) - 1;
                reqs->pos[index][i] = 0;
            }
        }
    }

    for (i = 0; i < CHARSET; ++i){
        if (reqs->occ[i] < 0 &&
            (counts[i] >= 0 || counts[i] < reqs->occ[i])
        ) reqs->occ[i] = counts[i];
    }
}


uint8_t check_req(char *s, req_ptr reqs){
    int16_t counts[CHARSET] = {0};
    int i, index, occs;

    for (i = 0; s[i] != '\0'; ++i){
        index = map_charset(s[i]);
        if ((reqs->match[i] != '*' && s[i] != reqs->match[i]) ||
            reqs->pos[index][i] == 0 ||
            reqs->occ[index] == 0
        ) return 0;
        ++(counts[index]);
    }

    for (i = 0; i < CHARSET; ++i){
        occs = reqs->occ[i];
        if ((occs >  0  && counts[i] !=  occs) ||
            (occs < -1  && counts[i]  < (-occs - 1))
        ) return 0;
    }

    return 1;
}


void filter_req(dict_ptr D, req_ptr reqs){
    node_ptr curr, prev, temp;

    curr = D->head;
    prev = NULL;
    while (curr != NULL){
        if (check_req(curr->word, reqs) == 0){
            if (prev != NULL) prev->next = curr->next;
            else D->head = curr->next;
            temp = curr;
            curr = curr->next;

            temp->next = NULL;
            --(D->len);
        } else{
            prev = curr;
            curr = curr->next;
        }
    }
}


void free_req(req_ptr reqs){
    int i;
    
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free(reqs->pos[i]);
    free(reqs);
}


void create_list(dict_ptr D, req_ptr reqs){
    node_ptr *list_buffer = (node_ptr *)malloc(D->items * sizeof(node_ptr));
    node_ptr curr;
    int i, j, count = 0;

    list_buffer[0] = NULL;
    for (i = 0, j = 0; j < D->items; ++i){
        curr = D->table[i];
        while (curr != NULL){
            if (check_req(curr->word, reqs) == 1){
                list_buffer[count] = curr;
                ++count;
            }
            curr = curr->chain;
            ++j;
        }
    }

    quicksort(list_buffer, list_buffer + count - 1);
    
    curr = list_buffer[0];
    for (i = 1; i < count; ++i){
        curr->next = list_buffer[i];
        curr = curr->next;
    }

    D->head = list_buffer[0];
    D->len = count;
    free(list_buffer);
}

void check_guess(dict_ptr D, char *ref, char *s, int wordsize, req_ptr reqs){
    char *eval;

    eval = calculate_eval(ref, s, wordsize);
    calculate_req(s, eval, reqs);
    if (D->head == NULL){
        clock_t start = clock();
        create_list(D, reqs);
        clock_t stop = clock();
        fill_time += (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    } else {
        clock_t start = clock();
        filter_req(D, reqs);
        clock_t stop = clock();
        filter_time += (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    }
    puts(eval);
    printf("%d\n", D->len);
    free(eval); 
}


void handle_insert(dict_ptr D, int wordsize, req_ptr reqs){
    node_ptr *list_buffer = (node_ptr *)malloc(BUFSIZE * sizeof(node_ptr));
    node_ptr new;
    char *buff = (char *)malloc((wordsize + 1) * sizeof(char));
    int i = 0, j = 1;

    if (D->head == NULL) reqs = NULL;
    
    safe_fgets(buff, wordsize);
    while(buff[0] != '+'){
        new = insert(D, buff);
        if (reqs != NULL && check_req(buff, reqs) == 1) {
            list_buffer[i] = new;
            ++i;
            if (i / BUFSIZE == j){
                list_buffer = (node_ptr *)realloc(list_buffer, j * BUFSIZE * sizeof(node_ptr));
                ++j;
            }
        }

        getchar();
        buff = (char *)malloc((wordsize + 1) * sizeof(char));
        safe_fgets(buff, wordsize);
    }
    if (wordsize <= 15) while (getchar() != '\n');

    clock_t start = clock();
    quicksort(list_buffer, list_buffer + i - 1);
    sequential_insert(D, list_buffer, i);
    clock_t stop = clock();
    insert_ord_time += (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    free(list_buffer);
    free(buff);
}


uint8_t new_game(dict_ptr D, int wordsize){
    req_ptr requirements = (req_ptr)malloc(sizeof(req_t));
    char *buff = (char *)malloc((wordsize + 1) * sizeof(char));
    char *ref = (char *)malloc((wordsize + 1) * sizeof(char));
    int guesses;

    generate_req(requirements, wordsize);  // instantiate requirements struct

    safe_fgets(ref, wordsize);
    getchar();
    safe_scanf(&guesses); // grab reference word and number of guesses

    while(guesses > 0){
        safe_fgets(buff, wordsize);
    
        if(buff[0] == '+'){
            if (buff[1] == 's'){ // +stampa_filtrate
                if (wordsize <= 16) while (getchar() != '\n');
                if (D->head == NULL) create_list(D, requirements);
                print_list(D);
            } else if (buff[1] == 'i'){ // +inserisci_inizio
                if (wordsize <= 17) while (getchar() != '\n');
                handle_insert(D, wordsize, requirements);
            }

        } else{
            getchar(); // trailing newline for words
            if (search(D, buff) == 0) puts("not_exists"); // string not in tree
            else if (strcmp(ref, buff) == 0) { // guessed correctly
                puts("ok");
                break;
            } else {
                check_guess(D, ref, buff, wordsize, requirements);
                --guesses;
            }
        }
    }
    if (guesses == 0) puts("ko");

    // garbage collection
    reset_list(D);
    free_req(requirements);
    free(buff);
    free(ref);

    if (getchar() == EOF) return 0;
    else {
        buff = (char *)malloc(18*sizeof(char));
        safe_fgets(buff, 17);
        if (buff[0] == 'i'){
            handle_insert(D, wordsize, NULL);
            safe_fgets(buff, 18);
        }
        free(buff);
    } return 1;
}
