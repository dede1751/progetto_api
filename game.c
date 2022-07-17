#include "game.h"

static void safe_fgets(char *, int);
static int map_charset(char);

static req_t *generate_reqs(int);
static void free_req(req_t *);

static void calculate_occs(char *, char *, int *);
static void analyze_guess(char *, char *, char *, int , int *, req_t *);

static int check_prev(char *, char *, char *, int *, int);
static int prune_prev(trie_t *, char *, char *, int *, int);

static int check_full(char *, req_t *, int);
static int prune_full(trie_t *, req_t *, int);

static trie_t *handle_insert(trie_t *, int);



static void safe_fgets(char *s, int size){
    if (fgets(s, size + 1, stdin) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(int *x){
    if (scanf("%d\n", x) == 0) exit(EXIT_FAILURE);
}


static int map_charset(char c){
    if (c == '-') return 0;
    else if (c >= '0' && c <= '9') return (c - 47);
    else if (c >= 'A' && c <= 'Z') return (c - 54);
    else if (c >= 'a' && c <= 'z') return (c - 59);
    else return 37; // maps '_'
}


static req_t *generate_reqs(int wordsize){
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


static void free_req(req_t *reqs){
    int i;
    
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free((reqs->pos)[i]);
    free(reqs);
}


static void calculate_occs(char *s, char *eval, int *occs){
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


/** @brief Prints evaluation and modifies requirements accordingly
 *
 *  Handles both evaluation and the requirements struct:
 *  evaluation is done by counting occurrences in the ref string and then, for
 *  each character in the s string, choosing '\' or '|' based on occurrences
 *  left. It then prints the eval string.
 * 
 *  Requirements calculation happens in multiple stages:
 *      - matching characters are flagged on the first pass of ref
 *      - impossible positions happen whenever eval is '|' or '/', so they are
 *        computed on the second pass on s
 *      - finally we use the eval string to compute occurrence bounds, and apply
 *        those to the requirements struct where needed.
 * 
 * @param ref       The reference string.
 * @param s         The guess string.
 * @param eval      String to write eval to
 * @param wordsize  Length of the strings.
 * @param occs      int[64] array, passed to use later for pruning.
 * @param reqs      Pointer to the requirements struct.
 * @return Void.
 */
static void analyze_guess(char *ref, char *s, char *eval, int wordsize, int *occs, req_t *reqs){
    uint8_t counts[CHARSET] = {0};
    int i, index;

    // count char occurrences in ref and handle perfect matches
    for(i = 0; i < wordsize; ++i){
        index  = map_charset(ref[i]);
        if (ref[i] != s[i]){
            eval[i] = '\0';             // reset eval
            ++(counts[index]);
        } else {
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
}

static int check_prev(char *sfx, char *s, char *eval, int *occs, int depth){
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
        (eval[depth] != '+' && sfx[0] == s[depth])
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

static int prune_prev(trie_t *trie, char *s, char *eval, int *occs, int depth){
    trie_t *curr;
    int index, count, res, total = 0;
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
                    if (count == -1){
                        res = check_prev(curr->status + 2*sizeof(char), s, eval, occs, depth + 1);
                    } else if (count < -1){
                        ++(occs[index]);
                        res = check_prev(curr->status + 2*sizeof(char), s, eval, occs, depth + 1);
                        --(occs[index]);
                    } else {
                        --(occs[index]);
                        res = check_prev(curr->status + 2*sizeof(char), s, eval, occs, depth + 1);
                        ++(occs[index]);
                    }

                    if (res == 0) (curr->status)[0] = PRUNE;
                    total += res;
    
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


static int check_full(char *sfx, req_t *reqs, int depth){
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
        ((reqs->match)[depth] != '*' && sfx[0] != (reqs->match)[depth])     ||
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


static int prune_full(trie_t *trie, req_t *reqs, int depth){
    trie_t *curr;
    int index, count, res, total = 0;
    char target;

    target = (reqs->match)[depth];

    for (curr = trie; curr != NULL; curr = curr->next){
        
        if ((curr->status)[0] == NO_PRUNE){  // don't consider pruned nodes

            if ((target != '*' && (curr->status)[1] == target)  || (target == '*')) {
    
                index = map_charset((curr->status)[1]);
                count = (reqs->occs)[index];

                if (count == 0 || (reqs->pos)[index][depth] == 0) (curr->status)[0] = PRUNE;
                else if (curr->branch == NULL) {
                    if (count == -1){
                        res = check_full(curr->status + 2*sizeof(char), reqs, depth + 1);
                    } else if (count < -1){
                        ++((reqs->occs)[index]);
                        res = check_full(curr->status + 2*sizeof(char), reqs, depth + 1);
                        --((reqs->occs)[index]);
                    } else {
                        --((reqs->occs)[index]);
                        res = check_full(curr->status + 2*sizeof(char), reqs, depth + 1);
                        ++((reqs->occs)[index]);
                    }

                    if (res == 0) (curr->status)[0] = PRUNE;
                    total += res;
    
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


static trie_t *handle_insert(trie_t *trie, int wordsize){
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
    char eval[wordsize + 1];
    char dump[18];
    int occs[CHARSET];
    int guesses, count = 0, insert_flag = 0, prune_flag = 1;

    eval[wordsize] = '\0';

    // grab reference word and number of guesses
    safe_fgets(ref, wordsize);
    getchar();
    safe_scanf(&guesses); 

    while(guesses > 0){
        safe_fgets(buff, wordsize);

        if(buff[0] == '+'){
    
            if (buff[1] == 's'){            // +stampa_filtrate
                if (wordsize <= 16) while (getchar() != '\n');

                if (insert_flag) {
                    prune_full(trie, reqs, 0);
                    insert_flag = 0;
                }
                print_trie(trie, wordsize);

            } else if (buff[1] == 'i'){     // +inserisci_inizio
                if (wordsize <= 17) while (getchar() != '\n');
    
                trie = handle_insert(trie, wordsize);
                insert_flag = 1;
                if (!prune_flag) prune_flag = 1;
            }

        } else{

            getchar(); // trailing newline for words

            if (strcmp(ref, buff) == 0) {           // guessed correctly
                puts("ok");
                break;
            } else if (search(trie, buff) == 0) {   // word not in dict
                puts("not_exists");
            } else {
                // first get the occs array and eval
                analyze_guess(ref, buff, eval, wordsize, occs, reqs);

                if (insert_flag) {          // previous command was insertion
                    count = prune_full(trie, reqs, 0);
                    insert_flag = 0;
                } else if (prune_flag) {    // more than 1 possible word
                    count = prune_prev(trie, buff, eval, occs, 0);
                }

                if (count == 1) prune_flag = 0;

                puts(eval);
                printf("%d\n", count);
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
