#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CHARSET 64
#define PRUNE 0
#define TEMP_PRUNE 1
#define NO_PRUNE 2

typedef struct trie {
    struct trie *next;
    struct trie *branch;
    char *status;
} trie_t;

typedef struct reqs {
    char *ref;
    char *match;
    int8_t occs[CHARSET];
    uint8_t *pos[CHARSET];
} req_t;

void safe_fgets(char *, uint8_t);
void safe_scanf(uint8_t *);

trie_t *generate_branch(char);
trie_t *get_child(trie_t *, char);
trie_t *add_child(trie_t *, char *);
trie_t *insert_leaf(trie_t *, char *, char);
void split_leaves(trie_t *, char *);
trie_t *insert(trie_t *, char *);
int search(trie_t *, char *);
void print(trie_t *, char *, uint8_t);
void print_trie(trie_t *, uint8_t);
void clear_trie(trie_t *);

req_t *generate_reqs(uint8_t);
void free_reqs(req_t *);
void eval_guess(char *, uint8_t , req_t *);
uint8_t check_leaf(char *, req_t *, uint8_t);
int prune_trie(trie_t *, req_t *, uint8_t);
trie_t *handle_insert(trie_t *, uint8_t);
trie_t *initial_read(trie_t *, uint8_t);
trie_t *new_game(trie_t *, uint8_t);

uint8_t conversion_table[128] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 0, 64, 64, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 64,
    64, 64, 64, 64, 64, 64, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 64, 64, 64, 64, 37, 64,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 61, 62, 63, 64, 64, 64, 64, 64
};
uint8_t insert_flag = 0;


int main(){
    trie_t *trie = NULL;
    uint8_t wordsize;

    safe_scanf(&wordsize);

    trie = initial_read(trie, wordsize);

    while(1) trie = new_game(trie, wordsize);
}

void safe_fgets(char *s, uint8_t size){
    if (fgets(s, size + 1, stdin) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(uint8_t *x){
    if (scanf("%hhu\n", x) == 0) exit(EXIT_FAILURE);
}

trie_t *generate_branch(char c){
    trie_t *new = (trie_t *)malloc(sizeof(trie_t));
    char *status = (char *)malloc(2*sizeof(char));

    new->next = NULL;
    new->branch = NULL;
    status[0] = NO_PRUNE;
    status[1] = c;
    new->status = status;

    return new;
}

trie_t *get_child(trie_t *trie, char tgt){
    char c;

    for (; trie != NULL; trie = trie->next){
        c = (trie->status)[1];
        if (c > tgt) return NULL;
        else if (c == tgt) return trie;
    }
    return NULL;
}

trie_t *add_child(trie_t *trie, char *status){
    trie_t *prev = NULL, *curr = trie, *new = (trie_t *)malloc(sizeof(trie_t));
    char tgt = status[1];

    while (curr != NULL && ((curr->status)[1] < tgt)){
        prev = curr;
        curr = curr->next;
    }

    new->next = curr;
    new->branch = NULL;
    new->status = status;

    if (prev == NULL) return new;
    else {
        prev->next = new;
        return trie;
    }
}

trie_t *insert_leaf(trie_t *trie, char *word, char p){
    int len = strlen(word);
    char *status = (char *)malloc((len + 2)*sizeof(char));

    status[0] = p;
    memcpy(status + sizeof(char), word, (len + 1)*sizeof(char));

    return add_child(trie, status);
}

void split_leaves(trie_t *trie, char *word){
    trie_t *tmp_trie = trie;
    char *tmp_sts = trie->status, *sfx = trie->status + 2*sizeof(char);

    for (; *sfx == *word; sfx += sizeof(char), word += sizeof(char)){
        trie->branch = generate_branch(word[0]);
        trie = trie->branch;
    }

    trie->branch = insert_leaf(NULL, word, NO_PRUNE);
    trie->branch = insert_leaf(trie->branch, sfx, tmp_sts[0]);

    tmp_sts[0] = NO_PRUNE;
    tmp_trie->status = realloc(tmp_sts, 2*sizeof(char));
}


trie_t *insert(trie_t *root, char *word){
    trie_t *child = get_child(root, word[0]), *trie = root, *prev_branch = NULL;

    while(child != NULL && child->branch != NULL){
        prev_branch = child;
        trie = child->branch;
    
        word += sizeof(char);
        child = get_child(trie, word[0]);
    }

    if (child == NULL) {
        if (prev_branch == NULL) return insert_leaf(trie, word, NO_PRUNE);
        else prev_branch->branch = insert_leaf(trie, word, NO_PRUNE);
    } else split_leaves(child, word + sizeof(char));
    return root;
}

int search(trie_t *root, char *word){
    root = get_child(root, word[0]);

    while (root != NULL && root->branch != NULL){
        word += sizeof(char);
        root = get_child(root->branch, word[0]);
    }

    if (root == NULL) return 0;
    else {
        if (strcmp((root->status) + sizeof(char), word) == 0) return 1;
        else return 0;
    }
}

void print(trie_t *trie, char *word, uint8_t depth){

    while (trie != NULL){
        if ((trie->status)[0] == NO_PRUNE){
            if (trie->branch == NULL) {
                fputs(word, stdout);
                puts((trie->status) + sizeof(char));
            } else {
                word[depth] = (trie->status)[1];
                print(trie->branch, word, depth + 1);
                word[depth] = '\0';
            }
        }
        trie = trie->next;
    }
}

void print_trie(trie_t *trie, uint8_t wordsize){
    char *word = (char *) calloc(wordsize + 1, sizeof(char));

    print(trie, word, 0);
    free(word);
}

void clear_trie(trie_t *trie){
    if (trie->next != NULL) clear_trie(trie->next);
    if (trie->branch != NULL) clear_trie(trie->branch);

    (trie->status)[0] = NO_PRUNE;
}

req_t *generate_reqs(uint8_t wordsize){
    req_t *reqs = (req_t *) malloc(sizeof(req_t));
    char *match, *ref;
    uint8_t *p, i, j;

    ref = (char *)malloc((wordsize + 1) * sizeof(char));
    safe_fgets(ref, wordsize);
    getchar();
    reqs->ref = ref;

    match = (char *)malloc((wordsize + 1) * sizeof(char));
    for (i = 0; i < wordsize; ++i) match[i] = '*';
    match[wordsize] = '\0';
    reqs->match = match;

    for (i = 0; i < CHARSET; ++i) {
        p = (uint8_t *)malloc((wordsize) * sizeof(uint8_t));
        for (j = 0; j < wordsize; ++j) p[j] = 1;

        (reqs->pos)[i]  = p;        
        (reqs->occs)[i] = -1;
    }

    return reqs;
}

void free_reqs(req_t *reqs){
    uint8_t i;
    
    free(reqs->ref);
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free((reqs->pos)[i]);
    free(reqs);
}

void eval_guess(char *s, uint8_t wordsize, req_t *reqs){
    int8_t occs[CHARSET] = {0};
    char eval[wordsize + 1], *ref = reqs->ref;
    uint8_t i, index;

    for(i = 0; i < wordsize; ++i){
        index  = conversion_table[(int) ref[i]];

        if (ref[i] != s[i]) ++(occs[index]);
        else {
            eval[i] = '+';
            (reqs->match)[i] = s[i];
        }
    }

    for (i = 0; i < wordsize; ++i){
        if (eval[i] != '+'){
            index = conversion_table[(int) s[i]];
            if (occs[index] == 0) eval[i] = '/';
            else {
                eval[i] = '|';
                --(occs[index]);
            }

            (reqs->pos)[index][i] = 0;
        }    
    }
    eval[wordsize] = '\0';

    for (i = 0; i < CHARSET; ++i) occs[i] = -1;
    for (i = 0; s[i] != '\0'; ++i){
        index = conversion_table[(int) s[i]];

        if (eval[i] != '/' && occs[index] < 0) {
            --(occs[index]);
        } else if (eval[i] == '+' && occs[index] >= 0) {
            ++(occs[index]);
        } else if (eval[i] == '/' && occs[index] < 0) {
            occs[index] = -(occs[index]) - 1;
        }
    }

    for (i = 0; i < wordsize; ++i){
        index = conversion_table[(int) s[i]];

        if ((reqs->occs)[index] < 0                                  &&
            (occs[index] >= 0 || occs[index] < (reqs->occs)[index])
        ) (reqs->occs)[index] = occs[index];
    }

    puts(eval);
}

uint8_t check_leaf(char *sfx, req_t *reqs, uint8_t depth){
    uint8_t index, res;
    int8_t count;
    char* ref;

    if (sfx[0] == '\0'){
        for (ref = (reqs->ref); *ref != '\0'; ++ref){
            index = conversion_table[(int) *ref];
            count = (reqs->occs)[index];
            if (count != 0 && count != -1) return 0;
        }
        return 1;
    }

    index = conversion_table[(int) sfx[0]];
    count = (reqs->occs)[index];
    if (( count == 0 )                                                      ||
        ((reqs->match)[depth] != '*' && sfx[0] != (reqs->match)[depth])     ||
        ((reqs->pos)[index][depth] == 0)
    ) return 0;

    if (count == -1){
        res = check_leaf(sfx + sizeof(char), reqs, depth + 1);
    } else if (count < -1){
        ++((reqs->occs)[index]);
        res = check_leaf(sfx + sizeof(char), reqs, depth + 1);
        --((reqs->occs)[index]);
    } else {
        --((reqs->occs)[index]);
        res = check_leaf(sfx + sizeof(char), reqs, depth + 1);
        ++((reqs->occs)[index]);
    }

    return res;
}

int prune_trie(trie_t *trie, req_t *reqs, uint8_t depth){
    trie_t *curr;
    uint8_t index, res;
    int8_t count;
    int total = 0;
    char target;

    for (curr = trie; curr != NULL; curr = curr->next){
        
        if (((curr->status)[0] == NO_PRUNE)                   ||
            (insert_flag && (curr->status)[0] == TEMP_PRUNE)
        ){

            target = (reqs->match)[depth];
            if ((target != '*' && (curr->status)[1] == target) || (target == '*')) {
    
                index = conversion_table[(int) (curr->status)[1]];
                count = (reqs->occs)[index];

                if (count == 0 || (reqs->pos)[index][depth] == 0) (curr->status)[0] = PRUNE;
                else if (curr->branch == NULL) {
                    if (count == -1){
                        res = check_leaf(curr->status + 2*sizeof(char), reqs, depth + 1);
                    } else if (count < -1){
                        ++((reqs->occs)[index]);
                        res = check_leaf(curr->status + 2*sizeof(char), reqs, depth + 1);
                        --((reqs->occs)[index]);
                    } else {
                        --((reqs->occs)[index]);
                        res = check_leaf(curr->status + 2*sizeof(char), reqs, depth + 1);
                        ++((reqs->occs)[index]);
                    }

                    if (res == 0) (curr->status)[0] = PRUNE;
                    total += res;
    
                } else {
                    if (count == -1){
                        total += prune_trie(curr->branch, reqs, depth + 1);
                    } else if (count < -1){
                        ++((reqs->occs)[index]);
                        total += prune_trie(curr->branch, reqs, depth + 1);
                        --((reqs->occs)[index]);
                    } else {
                        --((reqs->occs)[index]);
                        total += prune_trie(curr->branch, reqs, depth + 1);
                        ++((reqs->occs)[index]);
                    }

                    if ((curr->status)[0] == TEMP_PRUNE && total > 0) (curr->status)[0] = NO_PRUNE;
                    else if (total == 0) (curr->status)[0] = TEMP_PRUNE;
                }
            } else (curr->status)[0] = PRUNE;
        }
    }
    return total;
}

trie_t *handle_insert(trie_t *trie, uint8_t wordsize){
    char buff[wordsize + 1];

    safe_fgets(buff, wordsize);
    while (buff[0] != '+'){
        trie = insert(trie, buff);

        getchar();
        safe_fgets(buff, wordsize);
    }
    if (wordsize <= 15) while (getchar() != '\n');

    return trie;
}

trie_t *initial_read(trie_t *trie, uint8_t wordsize){
    char *buff = (char *) malloc((wordsize + 1) * sizeof(char));

    safe_fgets(buff, wordsize);
    while (buff[0] != '+'){
        trie = insert(trie, buff);

        getchar();
        safe_fgets(buff, wordsize);
    }

    if (buff[1] == 'n'){
        if (wordsize <= 14) while (getchar() != '\n');
    } else {
        if (wordsize <= 17) while (getchar() != '\n');
        trie = handle_insert(trie, wordsize);
        while(getchar() != '\n');
    }
    free(buff);

    return trie;
}

trie_t *new_game(trie_t *trie, uint8_t wordsize){
    req_t *reqs;
    uint8_t guesses;
    int count = 0;
    char buff[wordsize + 1];
    char dump[18];

    insert_flag = 0;
    reqs = generate_reqs(wordsize);
    safe_scanf(&guesses);

    while(guesses > 0){
        safe_fgets(buff, wordsize);
        if(buff[0] == '+'){
            if (buff[1] == 's'){
                if (wordsize <= 16) while (getchar() != '\n');

                if (insert_flag) {
                    count = prune_trie(trie, reqs, 0);
                    insert_flag = 0;
                }
                print_trie(trie, wordsize);

            } else if (buff[1] == 'i'){
                if (wordsize <= 17) while (getchar() != '\n');

                trie = handle_insert(trie, wordsize);
                insert_flag = 1;
                count = 0;
            }
        } else{
            getchar();
            if (strcmp(reqs->ref, buff) == 0) {
                puts("ok");
                break;
            } else if (search(trie, buff) == 0) {
                puts("not_exists");
            } else {
                eval_guess(buff, wordsize, reqs);

                if (count != 1) count = prune_trie(trie, reqs, 0);
                if (insert_flag) insert_flag = 0;
                
                printf("%d\n", count);
                --guesses;
            }
        }
    }
    if (guesses == 0) puts("ko");

    if (getchar() == EOF) exit(EXIT_SUCCESS);
    else {
        safe_fgets(dump, 17);

        if (dump[0] == 'i'){
            trie = handle_insert(trie, wordsize);
            safe_fgets(dump, 17);
        }

        free_reqs(reqs);
        clear_trie(trie);
    } return trie;
}
