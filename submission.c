#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define CHARSET 64

typedef struct node{
    char *word;
    uint8_t color;
    struct node *left;
    struct node *right;
    struct node *parent;
    struct node *next;
} node_t;
typedef node_t *node_ptr;

typedef struct dict {
    node_ptr root;
    node_ptr NIL;
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

void left_rotate(dict_ptr, node_ptr);
void right_rotate(dict_ptr, node_ptr);
void insert_fixup(dict_ptr, node_ptr);
node_ptr insert(dict_ptr, char*);
uint8_t search(dict_ptr, char*);

void ordered_insert(dict_ptr, node_ptr);
void print_list(dict_ptr);

void free_tree(node_ptr, node_ptr);
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

void fill_list(dict_ptr, node_ptr, req_ptr);
void check_guess(dict_ptr, char *, char *, int, req_ptr);
void handle_insert(dict_ptr, int, req_ptr);

uint8_t new_game(dict_ptr, int);


int main(){
    dict_ptr word_dict = generate_dict();
    char *buff;
    int wordsize, restart;

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

    do {
        restart = new_game(word_dict, wordsize);
    } while(restart == 1);

    free_dict(word_dict);
    exit(EXIT_SUCCESS);
}


dict_ptr generate_dict(){
    dict_ptr dict = (dict_ptr)malloc(sizeof(dict_t));
    node_ptr leaf = (node_ptr)malloc(sizeof(node_t));

    leaf->color = 0;
    leaf->word = NULL;
    leaf->left = NULL;
    leaf->right = NULL;
    leaf->parent = NULL;
    leaf->next = NULL;

    dict->NIL = leaf;
    dict->root = leaf;
    dict->head = NULL;
    dict->len = 0;

    return dict;
}

void left_rotate(dict_ptr D, node_ptr x){
    node_ptr y = x->right;

    x->right = y->left;
    if (y->left != D->NIL) y->left->parent = x;

    y->parent = x->parent;
    if (x->parent == D->NIL) D->root = y;
    else if (x->parent->left == x) x->parent->left = y;
    else x->parent->right = y;

    y->left = x;
    x->parent = y;
}
void right_rotate(dict_ptr D, node_ptr x){
    node_ptr y = x->left;

    x->left = y->right;
    if (y->right != D->NIL) y->right->parent = x;

    y->parent = x->parent;
    if (x->parent == D->NIL) D->root = y;
    else if (x->parent->left == x) x->parent->left = y;
    else x->parent->right = y;

    y->right = x;
    x->parent = y;
}


void insert_fixup(dict_ptr D, node_ptr curr){
    node_ptr prev, side;

    if (curr == D->root) curr->color = 0;
    else {
        prev = curr->parent;
        if (prev->color == 1){
            if (prev == prev->parent->left){
                side = prev->parent->right;

                if (side->color == 1){
                    prev->color = 0;
                    side->color = 0;
                    prev->parent->color = 1;
                    insert_fixup(D, prev->parent);

                } else {
                    if (curr == prev->right){
                        curr = prev;
                        left_rotate(D, curr);
                        prev = prev->parent;
                    }
                    prev->color = 0;
                    prev->parent->color = 1;
                    right_rotate(D, prev->parent);
                }

            } else if (prev == prev->parent->right){
                side = prev->parent->left;

                if (side->color == 1){
                    prev->color = 0;
                    side->color = 0;
                    prev->parent->color = 1;
                    insert_fixup(D, prev->parent);

                } else {
                    if (curr == prev->left){
                        curr = prev;
                        right_rotate(D, curr);
                        prev = curr->parent;
                    }
                    prev->color = 0;
                    prev->parent->color = 1;
                    left_rotate(D, prev->parent);
                }
            }
        }
    }
};

node_ptr insert(dict_ptr T, char *s){
    node_ptr curr, prev, new;
    int i;

    curr = T->root;
    prev = T->NIL;
    while(curr != T->NIL){
        prev = curr;
        i = strcmp(s, curr->word);
        if (i > 0) curr = curr->right;
        else curr = curr->left;
    }

    new = (node_ptr)malloc(sizeof(node_t));
    new->word = s;
    new->parent = prev;
    new->left = T->NIL;
    new->right = T->NIL;
    new->next = NULL;
    new->color = 1;

    if (prev == T->NIL) T->root = new;
    else if (i > 0) prev->right = new;
    else prev->left = new;

    insert_fixup(T, new);
    return new;
}


uint8_t search(dict_ptr D, char* s){
    node_ptr curr;
    int i;

    curr = D->root;
    while(curr != D->NIL){
        i = strcmp(s, curr->word);
        if (i == 0) return 1;
        else if (i > 0) curr = curr->right;
        else curr = curr->left;
    }
    return 0;
}


void ordered_insert(dict_ptr D, node_ptr x){
    node_ptr curr, prev;

    curr = D->head;
    prev = NULL;
    while(curr != NULL && strcmp(x->word, curr->word) > 0){
        prev = curr;
        curr = curr->next;
    }

    x->next = curr;
    if (D->head == NULL) D->head = x;
    else if (prev != NULL) prev->next = x;
    
    ++(D->len);
}

void print_list(dict_ptr T){
    node_ptr curr;
    for (curr = T->head; curr != NULL; curr = curr->next) puts(curr->word);
}


void free_tree(node_ptr curr, node_ptr NIL){
    if (curr->left != NIL) free_tree(curr->left, NIL);
    if (curr->right != NIL) free_tree(curr->right, NIL);
    free(curr);
}
void empty_blocks(node_ptr curr){
    if (curr->next != NULL) {
        empty_blocks(curr->next);
        curr->next = NULL;
    }
}

void reset_list(dict_ptr T){
    if (T->head != NULL){
        empty_blocks(T->head);
        T->head = NULL;
        T->len = 0;
    }
}

void free_dict(dict_ptr D){
    if (D->root != NULL) free_tree(D->root, D->NIL);
    free(D->NIL);
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
    uint8_t counts[CHARSET] = {0}; // possible of but gambling for memory
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


void fill_list(dict_ptr D, node_ptr x, req_ptr reqs){
    if (x != D->NIL){
        fill_list(D, x->right, reqs);

        if (check_req(x->word, reqs) == 1){
            x->next = D->head;
            D->head = x;
            ++(D->len);
        }

        fill_list(D, x->left, reqs);
    }
}


void check_guess(dict_ptr D, char *ref, char *s, int wordsize, req_ptr reqs){
    char *eval;

    eval = calculate_eval(ref, s, wordsize);
    calculate_req(s, eval, reqs);
    if (D->head == NULL) fill_list(D, D->root, reqs);
    else filter_req(D, reqs);

    puts(eval);
    printf("%d\n", D->len);
    free(eval);
}


void handle_insert(dict_ptr D, int wordsize, req_ptr reqs){
    node_ptr new;
    char *buff = (char *)malloc((wordsize + 1) * sizeof(char));

    if (D->head == NULL) fill_list(D, D->root, reqs);

    safe_fgets(buff, wordsize);
    while(buff[0] != '+'){ // +inserisci_fine
        new = insert(D, buff);
        if (reqs != NULL && check_req(buff, reqs) == 1) ordered_insert(D, new);

        getchar();
        buff = (char *)malloc((wordsize + 1) * sizeof(char));
        safe_fgets(buff, wordsize);
    }
    if (wordsize <= 15) while (getchar() != '\n');
    free(buff);
}


uint8_t new_game(dict_ptr D, int wordsize){
    req_ptr requirements = (req_ptr)malloc(sizeof(req_t));
    char *buff = (char *)malloc((wordsize + 1) * sizeof(char));
    char *ref = (char *)malloc((wordsize + 1) * sizeof(char));
    int guesses;

    generate_req(requirements, wordsize);

    safe_fgets(ref, wordsize);
    getchar();
    safe_scanf(&guesses);

    while(guesses > 0){
        safe_fgets(buff, wordsize);
    
        if(buff[0] == '+'){
            if (buff[1] == 's'){
                if (wordsize <= 16) while (getchar() != '\n');
                if (D->head == NULL) fill_list(D, D->root, requirements);
                print_list(D);
            } else if (buff[1] == 'i'){
                if (wordsize <= 17) while (getchar() != '\n');
                handle_insert(D, wordsize, requirements);
            }

        } else{
            getchar();
            if (search(D, buff) == 0) puts("not_exists");
            else if (strcmp(ref, buff) == 0) {
                puts("ok");
                break;
            } else {
                check_guess(D, ref, buff, wordsize, requirements);
                --guesses;
            }
        }
    }
    if (guesses == 0) puts("ko");

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
