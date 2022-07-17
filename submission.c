#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CHARSET 64
#define PRUNE 1
#define NO_PRUNE 0


typedef struct trie {
    struct trie *next;
    struct trie *branch;
    char *status;
} trie_t;

typedef struct reqs {
    char *match;
    int8_t occs[CHARSET];
    uint8_t *pos[CHARSET];
} req_t;

void safe_scanf(int *);
void safe_fgets(char *, int);
int map_charset(char);

trie_t *generate_branch(char);
trie_t *get_child(trie_t *, char);
trie_t *add_child(trie_t *, char *);
trie_t *insert_leaf(trie_t *, char *, char);
void split_leaves(trie_t *, char *);
trie_t *insert(trie_t *, char *);

int search(trie_t *, char *);

void print(trie_t *, char *, int);
void print_trie(trie_t *, int);
void clear_trie(trie_t *);

req_t *generate_reqs(int);
void free_req(req_t *);

void calculate_occs(char *, char *, int *);
char *analyze_guess(char *, char *, int , int *, req_t *);

int check_prev(char *, char *, char *, int *, int);
int check_leaf_prev(trie_t *, char *, char *, int *, int);
int prune_prev(trie_t *, char *, char *, int *, int);
void handle_simple_guess(trie_t *, int, char *, char *, req_t *);

int check_full(char *, req_t *, int);
int check_leaf_full(trie_t *, req_t *, int);
int prune_full(trie_t *, req_t *, int);
void handle_full_guess(trie_t *, int, char *, char *, req_t *);

trie_t *handle_insert(trie_t *, int);

trie_t *initial_read(trie_t *, int);
trie_t *new_game(trie_t *, int);



int main(){
    trie_t *trie = NULL;
    int wordsize;

    safe_scanf(&wordsize);

    trie = initial_read(trie, wordsize);

    do {
        trie = new_game(trie, wordsize);
    } while (trie != NULL);

    exit(EXIT_SUCCESS);
}


void safe_fgets(char *s, int size){
    if (fgets(s, size + 1, stdin) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(int *x){
    if (scanf("%d\n", x) == 0) exit(EXIT_FAILURE);
}


int map_charset(char c){
    if (c == '-') return 0;
    else if (c >= '0' && c <= '9') return (c - 47);
    else if (c >= 'A' && c <= 'Z') return (c - 54);
    else if (c >= 'a' && c <= 'z') return (c - 59);
    else return 37; // maps '_'
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
    int c;

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

    // find correct spot
    while (curr != NULL && ((curr->status)[1] < tgt)){
        prev = curr;
        curr = curr->next;
    }

    // add new trie node, return head of the list
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

    // add prune and copy whole word, so status[1] is word[0] hence the index
    status[0] = p;
    memcpy(status + sizeof(char), word, (len + 1)*sizeof(char));

    return add_child(trie, status);
}

void split_leaves(trie_t *trie, char *word){
    trie_t *tmp_trie = trie;
    char *tmp_sts = trie->status, *sfx = trie->status + 2*sizeof(char);

    // navigate down as long as word and sfx are the same
    for (; *sfx == *word; sfx += sizeof(char), word += sizeof(char)){
        trie->branch = generate_branch(word[0]);
        trie = trie->branch;
    }

    // at some point they must differ, add them as leaves to trie->branch.
    trie->branch = insert_leaf(NULL, word, NO_PRUNE);
    trie->branch = insert_leaf(trie->branch, sfx, tmp_sts[0]);

    // reallocate initial leaf to be unpruned branch (only 2 chars in status)
    tmp_sts[0] = NO_PRUNE;
    tmp_trie->status = realloc(tmp_sts, 2*sizeof(char));
}

trie_t *insert(trie_t *root, char *word){
    trie_t *child = get_child(root, word[0]), *trie = root, *prev_branch = NULL;

    // iterate down as long as child is found and it's a branch
    while(child != NULL && child->branch != NULL){
        prev_branch = child;
        trie = child->branch;
    
        word += sizeof(char);
        child = get_child(trie, word[0]);
    }

    if (child == NULL) {
        if (prev_branch == NULL) return insert_leaf(trie, word, NO_PRUNE); // root might change
        else prev_branch->branch = insert_leaf(trie, word, NO_PRUNE);      // root won't change
    } else split_leaves(child, word + sizeof(char)); // doesn't affect root
    return root;
}


int search(trie_t *root, char *word){
    root = get_child(root, word[0]);

    // descend down branch until leaf or NULL
    while (root != NULL && root->branch != NULL){
        word += sizeof(char);
        root = get_child(root->branch, word[0]);
    }

    if (root == NULL) return 0;
    else { // check that the suffix matches the rest of the word
        if (strcmp((root->status) + sizeof(char), word) == 0) return 1;
        else return 0;
    }
}


void print(trie_t *trie, char *word, int depth){

    while (trie != NULL){
        if ((trie->status)[0] == NO_PRUNE){
            if (trie->branch == NULL) {
                fputs(word, stdout); //omit newline
                puts((trie->status) + sizeof(char)); // always at least one letter
            } else {
                word[depth] = (trie->status)[1];
                print(trie->branch, word, depth + 1);
                word[depth] = '\0';
            }
        }
        trie = trie->next;
    }
}

void print_trie(trie_t *trie, int wordsize){
    char *word = (char *) calloc(wordsize + 1, sizeof(char));

    print(trie, word, 0);
    free(word);
}


void clear_trie(trie_t *trie){

    if (trie->next != NULL) clear_trie(trie->next);
    if (trie->branch != NULL) clear_trie(trie->branch);

    (trie->status)[0] = NO_PRUNE;
}


req_t *generate_reqs(int wordsize){
    req_t *reqs = (req_t *) malloc(sizeof(req_t));
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

        (reqs->pos)[i]  = p;        
        (reqs->occs)[i] = -1;
    }

    return reqs;
}


void free_req(req_t *reqs){
    int i;
    
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free((reqs->pos)[i]);
    free(reqs);
}


void calculate_occs(char *s, char *eval, int *occs){
    int i, index;

    // set all occs to -1 (unbound)
    for (i = 0; i < CHARSET; ++i) occs[i] = -1;

    for (i = 0; s[i] != '\0'; ++i){
        index = map_charset(s[i]);

        if (eval[i] != '/' && occs[index] < 0) {            // '+' or '/'
            --(occs[index]);                                // increase minimum
        } else if (eval[i] == '+' && occs[index] >= 0) {    // '+'
            ++(occs[index]);                                // increase exact
        } else if (eval[i] == '/' && occs[index] < 0) {     // '/'
            occs[index] = -(occs[index]) - 1;               // set exact
        }
    }
}


char *analyze_guess(char *ref, char *s, int wordsize, int *occs, req_t *reqs){
    char *eval = (char *)calloc(wordsize + 1, sizeof(char));
    uint8_t counts[CHARSET] = {0};
    int i, index;
    
    // count char occurrences in ref and handle perfect matches
    for(i = 0; i < wordsize; ++i){
        index  = map_charset(ref[i]);
        if (ref[i] != s[i]) ++(counts[index]);
        else {
            eval[i] = '+';
            (reqs->match)[i] = s[i];
        }
    }

    // handle imperfect matches and exclusions
    for (i = 0; i < wordsize; ++i){
        if (eval[i] != '+'){
            index = map_charset(s[i]);
            if (counts[index] == 0) eval[i] = '/';
            else {
                eval[i] = '|';
                --(counts[index]);
            }

            // get rid of impossible positions according to eval
            (reqs->pos)[index][i] = 0;
        }    
    }

    // once eval is computed, get the occurrences it imposes
    calculate_occs(s, eval, occs);

    // use occs to fix up requirements
    for (i = 0; i < wordsize; ++i){
        index = map_charset(s[i]);

        // modify reqs->occs[index] if the new bounds are "stricter"
        if ((reqs->occs)[index] < 0                                  &&
            (occs[index] >= 0 || occs[index] < (reqs->occs)[index])
        ) (reqs->occs)[index] = occs[index];
    }

    return eval;
}

int check_prev(char *sfx, char *s, char *eval, int *occs, int depth){
    int i, index, count, res;

    // once we run out of suffix, check that all occs are either -1 or 0
    if (sfx[0] == '\0'){
        for (i = 0; s[i] != '\0'; ++i){
            index = map_charset(s[i]);
            if (occs[index] != 0 && occs[index] != -1) return 0;
        }
        return 1;
    }

    // otherwise, check that sfx[0] is compatible with the bounds
    index = map_charset(sfx[0]);
    count = occs[index];
    if (( count == 0 )                              ||
        (eval[depth] == '+' && sfx[0] != s[depth])  ||
        (eval[depth] != '/' && sfx[0] == s[depth])
    ) return 0;
    
    if (count == -1){
        res = check_prev(sfx + sizeof(char), s, eval, occs, depth + 1);
    } else if (count < -1){
        ++(occs[index]);
        res = check_prev(sfx + sizeof(char), s, eval, occs, depth + 1);
        --(occs[index]);
    } else {
        --(occs[index]);
        res = check_prev(sfx + sizeof(char), s, eval, occs, depth + 1);
        ++(occs[index]);
    }

    return res;
}

int check_leaf_prev(trie_t *trie, char *s, char *eval, int *occs, int depth){
    int res, index = map_charset((trie->status)[1]);
    int count = occs[index];

    // the first letter of status is already checked by the pruning function
    if (count == -1){
        res = check_prev(trie->status + 2*sizeof(char), s, eval, occs, depth + 1);
    } else if (count < -1){
        ++(occs[index]);
        res = check_prev(trie->status + 2*sizeof(char), s, eval, occs, depth + 1);
        --(occs[index]);
    } else {
        --(occs[index]);
        res = check_prev(trie->status + 2*sizeof(char), s, eval, occs, depth + 1);
        ++(occs[index]);
    }

    if (res == 0) (trie->status)[0] = PRUNE;

    return res;
}


int prune_prev(trie_t *trie, char *s, char *eval, int *occs, int depth){
    trie_t *curr;
    int index, count, total = 0;
    char target;

    target = s[depth];

    for (curr = trie; curr != NULL; curr = curr->next){
        
        if ((curr->status)[0] == NO_PRUNE){  // don't consider pruned nodes

            if ((eval[depth] == '+' && (curr->status)[1] == target)  ||
                (eval[depth] != '+' && (curr->status)[1] != target)
            ) {
    
                index = map_charset((curr->status)[1]);
                count = occs[index];

                if (count == 0) (curr->status)[0] = PRUNE;
                else if (curr->branch == NULL){
                    total += check_leaf_prev(curr, s, eval, occs, depth);
                } else {
                    if (count == -1){
                        total += prune_prev(curr->branch, s, eval, occs, depth + 1);
                    } else if (count < -1){
                        ++(occs[index]);
                        total += prune_prev(curr->branch, s, eval, occs, depth + 1);
                        --(occs[index]);
                    } else {
                        --(occs[index]);
                        total += prune_prev(curr->branch, s, eval, occs, depth + 1);
                        ++(occs[index]);
                    }
                }
            } else (curr->status)[0] = PRUNE;
        }
    }

    return total; 
}

void handle_simple_guess(trie_t *trie, int wordsize, char *ref, char *s, req_t *reqs){
    char *eval;
    int occs[CHARSET], count;

    eval = analyze_guess(ref, s, wordsize, occs, reqs);
    count = prune_prev(trie, s, eval, occs, 0);

    puts(eval);
    printf("%d\n", count);

    free(eval);

}


int check_full(char *sfx, req_t *reqs, int depth){
    int i, index, count, res;

    // once we run out of suffix, check that all occs are either -1 or 0
    if (sfx[0] == '\0'){
        for (i = 0; i < CHARSET; ++i){
            if ((reqs->occs)[i] != 0 && (reqs->occs)[i] != -1) return 0;
        }
        return 1;
    }

    // otherwise, check that sfx[0] is compatible with the bounds
    index = map_charset(sfx[0]);
    count = (reqs->occs)[index];
    if (( count == 0 )                                                      ||
        ((reqs->match)[depth] != '*' && sfx[0] == (reqs->match)[depth])     ||
        ((reqs->pos)[index][depth] == 0)
    ) return 0;
    
    if (count == -1){
        res = check_full(sfx + sizeof(char), reqs, depth + 1);
    } else if (count < -1){
        ++((reqs->occs)[index]);
        res = check_full(sfx + sizeof(char), reqs, depth + 1);
        --((reqs->occs)[index]);
    } else {
        --((reqs->occs)[index]);
        res = check_full(sfx + sizeof(char), reqs, depth + 1);
        ++((reqs->occs)[index]);
    }

    return res;
}

int check_leaf_full(trie_t *trie, req_t *reqs, int depth){
    int res, index = map_charset((trie->status)[1]);
    int count = (reqs->occs)[index];

    // the first letter of status is already checked by the pruning function
    if (count == -1){
        res = check_full(trie->status + 2*sizeof(char), reqs, depth + 1);
    } else if (count < -1){
        ++((reqs->occs)[index]);
        res = check_full(trie->status + 2*sizeof(char), reqs, depth + 1);
        --((reqs->occs)[index]);
    } else {
        --((reqs->occs)[index]);
        res = check_full(trie->status + 2*sizeof(char), reqs, depth + 1);
        ++((reqs->occs)[index]);
    }

    if (res == 0) (trie->status)[0] = PRUNE;
    return res;
}

int prune_full(trie_t *trie, req_t *reqs, int depth){
    trie_t *curr;
    int index, count, total = 0;
    char target;

    target = (reqs->match)[depth];

    for (curr = trie; curr != NULL; curr = curr->next){
        
        if ((curr->status)[0] == NO_PRUNE){  // don't consider pruned nodes

            if ((target != '*' && (curr->status)[1] == target)  || (target == '*')) {
    
                index = map_charset((curr->status)[1]);
                count = (reqs->occs)[index];

                if (count == 0 || (reqs->pos)[index][depth] == 0) (curr->status)[0] = PRUNE;
                else if (curr->branch == NULL) {
                    total += check_leaf_full(curr, reqs, depth);
                } else {
                    if (count == -1){
                        total += prune_full(curr->branch, reqs, depth + 1);
                    } else if (count < -1){
                        ++((reqs->occs)[index]);
                        total += prune_full(curr->branch, reqs, depth + 1);
                        --((reqs->occs)[index]);
                    } else {
                        --((reqs->occs)[index]);
                        total += prune_full(curr->branch, reqs, depth + 1);
                        ++((reqs->occs)[index]);
                    }
                }
            } else (curr->status)[0] = PRUNE;
        }
    }

    return total; 
}

void handle_full_guess(trie_t *trie, int wordsize, char *ref, char *s, req_t *reqs){
    char *eval;
    int occs[CHARSET], count;

    eval = analyze_guess(ref, s, wordsize, occs, reqs);
    count = prune_full(trie, reqs, 0);

    puts(eval);
    printf("%d\n", count);

    free(eval);
}


trie_t *initial_read(trie_t *trie, int wordsize){
    char *buff = (char *) malloc((wordsize + 1) * sizeof(char));

    safe_fgets(buff, wordsize);
    while (buff[0] != '+'){
        trie = insert(trie, buff);

        getchar(); //trailing \n
        safe_fgets(buff, wordsize);
    }

    if (buff[1] == 'n'){    // +nuova_partita
        if (wordsize <= 14) while (getchar() != '\n');
    } else {                // +inserisci_inizio
        if (wordsize <= 17) while (getchar() != '\n');

        safe_fgets(buff, wordsize);
        while (buff[0] != '+'){
            trie = insert(trie, buff);

            getchar(); //trailing \n
            safe_fgets(buff, wordsize);
        }
        // dump remaining of +inserisci_fine, dump +nuova_partita
        if (wordsize <= 15) while (getchar() != '\n');
        while(getchar() != '\n');
    }
    free(buff);

    return trie;
}


trie_t *handle_insert(trie_t *trie, int wordsize){
    char buff[wordsize + 1];

    safe_fgets(buff, wordsize);
    while (buff[0] != '+'){
        trie = insert(trie, buff);

        getchar(); //trailing \n
        safe_fgets(buff, wordsize);
    }
    if (wordsize <= 15) while (getchar() != '\n');

    return trie;
}


trie_t *new_game(trie_t *trie, int wordsize){
    req_t *reqs = generate_reqs(wordsize);
    char buff[wordsize + 1];
    char ref[wordsize + 1];
    char dump[18];
    int guesses, insert_flag = 0;

    // grab reference word and number of guesses
    safe_fgets(ref, wordsize);
    getchar();
    safe_scanf(&guesses); 

    while(guesses > 0){
        safe_fgets(buff, wordsize);

        if(buff[0] == '+'){
    
            if (buff[1] == 's'){            // +stampa_filtrate

                if (wordsize <= 16) while (getchar() != '\n');
                print_trie(trie, wordsize);
            } else if (buff[1] == 'i'){     // +inserisci_inizio

                if (wordsize <= 17) while (getchar() != '\n');
                trie = handle_insert(trie, wordsize);
                insert_flag = 1;
            }

        } else{
            getchar(); // trailing newline for words

            if (strcmp(ref, buff) == 0) { // guessed correctly
                puts("ok");
                break;
            } else if (search(trie, buff) == 0) {
                puts("not_exists");
            } else {
                if (insert_flag) { // prior command was insertion
                    handle_full_guess(trie, wordsize, ref, buff, reqs);
                    insert_flag = 0;
                } else handle_simple_guess(trie, wordsize, ref, buff, reqs);

                --guesses;
            }
        }
    }
    if (guesses == 0) puts("ko");

    if (getchar() == EOF) return NULL;
    else {
        safe_fgets(dump, 17);

        if (dump[0] == 'i'){
            trie = handle_insert(trie, wordsize);
            safe_fgets(dump, 17);
        }

        // free/clear only when restarting
        free_req(reqs);
        clear_trie(trie);
    } return trie;
}
