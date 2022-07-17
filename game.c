#include "game.h"


static void calculate_occs(char *, char *, int *);
static char *analyze_guess(char *, char *, int , int *, req_t *);

static int check_prev(char *, char *, char *, int *, int);
static int check_leaf_prev(trie_t *, char *, char *, int *, int);
static int prune_prev(trie_t *, char *, char *, int *, int);

static int check_full(char *, req_t *, int);
static int check_leaf_full(trie_t *, req_t *, int);
static int prune_full(trie_t *, req_t *, int);

// static void generate_req(req_t *, int);
// static void free_req(req_t *);



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
char unmap_charset(int x){
    if (x == 0) return '-';
    else if (x >= 1 && x <= 10) return (x + 47);
    else if (x >= 10 && x <= 36) return (x + 54);
    else if (x >= 38 && x <= 63) return (x + 59);
    else return '_';
}


static void calculate_occs(char *s, char *eval, int *occs){
    int i, index;

    // set all occs to -1 (unbound)
    for (i = 0; i < CHARSET; ++i) occs[i] = -1;

    for (i = 0; s[i] != '\0'; ++i){
        index = map_charset(s[i]);

        // if '|' increase the minimum amount
        if (eval[i] == '|' && occs[index] < 0) --(occs[index]);
        // if '/' set the exact amount, -1 --> 0  -3 --> 2
        else if (eval[i] == '/' && occs[index] < 0) {
            occs[index] = -(occs[index]) - 1;
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
 * @param wordsize  Length of the strings.
 * @param occs      int[64] array, passed to use later for pruning.
 * @param reqs      Pointer to the requirements struct.
 * @return Void.
 */
static char *analyze_guess(char *ref, char *s, int wordsize, int *occs, req_t *reqs){
    char *eval = (char *)calloc(wordsize + 1, sizeof(char));
    uint8_t counts[CHARSET] = {0};
    int i, index;
    
    // count char occurrences in ref and handle perfect matches
    for(i = 0; i < wordsize; ++i){
        index  = map_charset(ref[i]);
        if (ref[i] != s[i]) ++(counts[index]);
        else {
            eval[i] = '+';

            // first time we find an exact match, we reduce previous bounds
            if ((reqs->match)[i] == '*'){
                (reqs->match)[i] = s[i];
                if ((reqs->occs)[index] > 0) --((reqs->occs)[index]);
                else if ((reqs->occs)[index] < -1) ++((reqs->occs)[index]);
            }
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

            // already get rid of impossible positions according to eval
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
            (occs[index] >= 0 || (reqs->occs)[index] > occs[index])
        ) (reqs->occs)[index] = occs[index];
    }

    puts(eval);
    return eval;
}


void generate_req(int wordsize, req_t *reqs){
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
}


void free_req(req_t *reqs){
    int i;
    
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free((reqs->pos)[i]);
    free(reqs);
}


static int check_prev(char *sfx, char *s, char *eval, int *occs, int depth){
    int i, index;

    // once we run out of suffix, check that all occs are either -1 or 0
    if (sfx[0] == '\0'){
        for (i = 0; s[i] != '\0'; ++i){
            index = map_charset(s[i]);
            if (occs[index] != 0 && occs[index] != -1) return 0;
        }
        return 1;
    }

    if (eval[depth] == '+'){ // in case of '+', continue only if it's an exact match
        if (sfx[0] == s[depth]) {
            return check_prev(sfx + sizeof(char), s, eval, occs, depth + 1);
        } else return 0;

    } else { // in case of '|', continue only if it's not a match
        index = map_charset(sfx[0]);

        if (sfx[0] == s[depth] || occs[index] == 0) return 0;
        else {

            if (occs[index] == -1){
                // occurrences unbound, do not modify array
                return check_prev(sfx + sizeof(char), s, eval, occs, depth + 1);
            } else if (occs[index] < -1){
                // decrease minimum occurrences
                ++(occs[index]);
                return check_prev(sfx + sizeof(char), s, eval, occs, depth + 1);
                --(occs[index]);
            } else {
                // decrease exact occurrences
                --(occs[index]);
                return check_prev(sfx + sizeof(char), s, eval, occs, depth + 1);
                ++(occs[index]);
            }
        }
    }
}

static int check_leaf_prev(trie_t *trie, char *s, char *eval, int *occs, int depth){
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
    if (res == 1) {
        return res;
    }

    return res;
}

static int prune_prev(trie_t *trie, char *s, char *eval, int *occs, int depth){
    trie_t *curr;
    int index, target, total = 0;

    // target letter
    target = s[depth];

    if (eval[depth] == '+'){ // prune all except target letter's node
        for (curr = trie; curr != NULL; curr = curr->next){
            if ((curr->status)[1] == target && (curr->status)[0] == NO_PRUNE){
                if (curr->branch == NULL) total = check_leaf_prev(curr, s, eval, occs, depth);
                else total = prune_prev(curr->branch, s, eval, occs, depth + 1);
            } else (curr->status)[0] = PRUNE; // not target, prune
        }

    } else { // prune only target letter's node
        for (curr = trie; curr != NULL; curr = curr->next){
            if ((curr->status)[0] == NO_PRUNE){ // don't consider pruned nodes
                index = map_charset((curr->status)[1]);

                // if curr is target or has no occurrences left, prune it.
                if ((curr->status)[1] == target || occs[index] == 0) (curr->status)[0] = PRUNE;
                else {
                    if (curr->branch == NULL) total += check_leaf_prev(curr, s, eval, occs, depth);
                    else { // decrement occs array (like in previous function)
                        if (occs[index] == -1){
                            total += prune_prev(curr->branch, s, eval, occs, depth + 1);
                        } else if (occs[index] < -1){
                            ++(occs[index]);
                            total += prune_prev(curr->branch, s, eval, occs, depth + 1);
                            --(occs[index]);
                        } else {
                            --(occs[index]);
                            total += prune_prev(curr->branch, s, eval, occs, depth + 1);
                            ++(occs[index]);
                        }
                    }
                }
            }
        }
    }

    return total;
}

void handle_simple_guess(trie_t *trie, int wordsize, char *ref, char *s, req_t *reqs){
    char *eval;
    int occs[CHARSET], count;

    if (strcmp(ref, s) == 0) puts("ok");
    else if (search(trie, s) == 0) puts("not_exists");
    else{
        eval = analyze_guess(ref, s, wordsize, occs, reqs);
        count = prune_prev(trie, s, eval, occs, 0);

        printf("%d\n", count);
        free(eval);
    }
}


static int check_full(char *sfx, req_t *reqs, int depth){
    int i, index;

    // once we run out of suffix, check that all occs are either -1 or 0
    if (sfx[0] == '\0'){
        for (i = 0; i < CHARSET; ++i){
            if ((reqs->occs)[i] != 0 && (reqs->occs)[i] != -1) return 0;
        }
        return 1;
    }

    if ((reqs->match)[depth] != '*'){ // in case of exact match
        if (sfx[0] == (reqs->match)[depth]) {
            return check_full(sfx + sizeof(char), reqs, depth + 1);
        } else return 0;

    } else { // inexact match
        index = map_charset(sfx[0]);

        if ((reqs->pos)[index][depth] == 0 || // position unavailable for char
            (reqs->occs)[index] == 0          // char does not appear
        ) return 0;

        else {
            if ((reqs->occs)[index] == -1){
                return check_full(sfx + sizeof(char), reqs, depth + 1);
            } else if ((reqs->occs)[index] < -1){
                ++((reqs->occs)[index]);
                return check_full(sfx + sizeof(char), reqs, depth + 1);
                --((reqs->occs)[index]);
            } else {
                --((reqs->occs)[index]);
                return check_full(sfx + sizeof(char), reqs, depth + 1);
                ++((reqs->occs)[index]);
            }
        }
    }
}

static int check_leaf_full(trie_t *trie, req_t *reqs, int depth){
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

static int prune_full(trie_t *trie, req_t *reqs, int depth){
    trie_t *curr;
    int index, target, total = 0;

    target = (reqs->match)[depth];

    if (target != '*'){ // prune all except target letter's node
        for (curr = trie; curr != NULL; curr = curr->next){
            if ((curr->status)[1] == target && (curr->status)[0] == NO_PRUNE){
                if (curr->branch == NULL) total = check_leaf_full(curr, reqs, depth);
                else total = prune_full(curr->branch, reqs, depth + 1);
            } else (curr->status)[0] = PRUNE; // not target, prune
        }

    } else {
        for (curr = trie; curr != NULL; curr = curr->next){
            if ((curr->status)[0] == NO_PRUNE){ // don't consider pruned nodes
                index = map_charset((curr->status)[1]);

                // if curr is target or has no occurrences left, prune it.
                if ((reqs->pos)[index][depth] == 0 || // position is unavailable
                    (reqs->occs)[index] == 0          // letter does not appear
                ) (curr->status)[0] = PRUNE;
                else {
                    if (curr->branch == NULL) total += check_leaf_full(curr, reqs, depth);
                    else {
                        if ((reqs->occs)[index] == -1){
                            total += prune_full(curr->branch, reqs, depth + 1);
                        } else if ((reqs->occs)[index] < -1){
                            ++((reqs->occs)[index]);
                            total += prune_full(curr->branch, reqs, depth + 1);
                            --((reqs->occs)[index]);
                        } else {
                            --((reqs->occs)[index]);
                            total += prune_full(curr->branch, reqs, depth + 1);
                            ++((reqs->occs)[index]);
                        }
                    }
                }
            }
        }
    }

    return total;
}

void handle_full_guess(trie_t *trie, int wordsize, char *ref, char *s, req_t *reqs){
    char *eval;
    int occs[CHARSET], count;

    if (strcmp(ref, s) == 0) puts("ok");
    else if (search(trie, s) == 0) puts("not_exists");
    else{
        eval = analyze_guess(ref, s, wordsize, occs, reqs);
        count = prune_full(trie, reqs, 0);

        printf("%d\n", count);
        free(eval);
    }
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


// uint8_t new_game(dict_ptr D, int wordsize, FILE *input, FILE *output){
//     req_ptr requirements = (req_ptr)malloc(sizeof(req_t));
//     char *buff = (char *)malloc((wordsize + 1) * sizeof(char));
//     char *ref = (char *)malloc((wordsize + 1) * sizeof(char));
//     int guesses;

//     generate_req(requirements, wordsize);  // instantiate requirements struct

//     safe_fgets(ref, wordsize, input);
//     fgetc(input);
//     safe_scanf(&guesses, input); // grab reference word and number of guesses

//     while(guesses > 0){
//         safe_fgets(buff, wordsize, input);

//         if(buff[0] == '+'){
//             if (buff[1] == 's'){ // +stampa_filtrate
//                 if (wordsize <= 16) while (fgetc(input) != '\n');
//                 if (D->head == NULL) create_list(D, requirements);
//                 print_list(D, output);
//             } else if (buff[1] == 'i'){ // +inserisci_inizio
//                 if (wordsize <= 17) while (fgetc(input) != '\n');
//                 handle_insert(D, wordsize, requirements, input);
//             }

//         } else{
//             fgetc(input); // trailing newline for words
//             if (search(D, buff) == 0) fputs("not_exists\n", output); // string not in tree
//             else if (strcmp(ref, buff) == 0) { // guessed correctly
//                 fputs("ok\n", output);
//                 break;
//             } else {
//                 check_guess(D, ref, buff, wordsize, requirements, output);
//                 --guesses;
//             }
//         }
//     }
//     if (guesses == 0) fputs("ko\n", output);

//     // garbage collection
//     reset_list(D);
//     free_req(requirements);
//     free(buff);
//     free(ref);

//     if (fgetc(input) == EOF) return 0;
//     else {
//         buff = (char *)malloc(18*sizeof(char));
//         safe_fgets(buff, 17, input);
//         if (buff[0] == 'i'){
//             handle_insert(D, wordsize, NULL, input); // only add words to tree
//             safe_fgets(buff, 17, input);
//         }
//         free(buff);
//     } return 1;
// }
