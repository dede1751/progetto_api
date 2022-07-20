#include "game.h"

static void safe_fgets(char *, int);

static req_t *generate_reqs(int);
static void free_req(req_t *);

static void calculate_occs(char *, char *, int *);
static void analyze_guess(char *, char *, char *, int , int *, req_t *);

static int check_prev(char *, char *, char *, int *, int);
static int prune_prev(trie_t *, char *, char *, int *, int);

static int check_full(char *, req_t *, int);
static int prune_full(trie_t *, req_t *, int);

static trie_t *handle_insert(trie_t *, int);

/**
 * @brief Convert characters to 0-63 interval  
 * 
 *         index = conversion_table[(int) letter]
 * 
 *  This little change got me 30L, kcachegrind reported a 5% speedup after using
 *  this to replace an equally simple function which simply saw which interval
 *  the letter is in and subtracted the appropriate amount (something like -54
 *  if A-Z etc.)
 */
static int conversion_table[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1,
    -1, -1, -1, -1, -1, -1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, -1, -1, -1, -1, 37, -1,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 61, 62, 63, -1, -1, -1, -1, -1
};


// compiler throws errors without these
static void safe_fgets(char *s, int size){
    if (fgets(s, size + 1, stdin) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(int *x){
    if (scanf("%d\n", x) == 0) exit(EXIT_FAILURE);
}

/**
 * @brief Allocate and initialize requirements struct
 * @param wordsize  size of the words in the trie
 * @return req_t*   pointer to the requirements struct
 */
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

/**
 * @brief Free the requirements struct and its contents
 * @param reqs      pointer to the struct to free
 */
static void free_req(req_t *reqs){
    int i;
    
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free((reqs->pos)[i]);
    free(reqs);
}

/**
 * @brief Calculate the minimum/exact occurrences deduced from a guess
 * @param s         guess string
 * @param eval      evaluation of the guess
 * @param occs      string to write occs to 
 */
static void calculate_occs(char *s, char *eval, int *occs){
    int i, index;

    // set all occs to -1 (unbound)
    for (i = 0; i < CHARSET; ++i) occs[i] = -1;

    for (i = 0; s[i] != '\0'; ++i){
        index = conversion_table[(int) s[i]];

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
 *      - 1st pass, REF  --> matching characters, count letters in ref
 *      - 2nd pass, S    --> impossible positions whenever we don't have match
 *                           computes '|' and '/' of eval using counts
 *      - 3rd pass, S    --> after computing occs, replace constraints in reqs
 *                           if occs contains stricter bounds
 * 
 * @param ref       reference string.
 * @param s         guess string.
 * @param eval      String to write eval to
 * @param wordsize  size of the words in input
 * @param occs      int[64] array, passed to use later for pruning.
 * @param reqs      pointer to the requirements struct.
 */
static void analyze_guess(char *ref, char *s, char *eval, int wordsize, int *occs, req_t *reqs){
    uint8_t counts[CHARSET] = {0};
    int i, index;

    // count char occurrences in ref and handle perfect matches
    for(i = 0; i < wordsize; ++i){
        index  = conversion_table[(int) ref[i]];
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
            index = conversion_table[(int) s[i]];
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
        index = conversion_table[(int) s[i]];

        // modify reqs->occs[index] if the new bounds are "stricter"
        if ((reqs->occs)[index] < 0                                  &&
            (occs[index] >= 0 || occs[index] < (reqs->occs)[index])
        ) (reqs->occs)[index] = occs[index];
    }
}

/**
 * @brief Recursively check word suffix based on previous guess
 * 
 *  Recursively travels down the suffix modifying the occs array like in
 *  prune_prev(), and when it runs out it checks that the occurrences for all
 *  the letters in the guess strings are either -1 or 0: this means that, while
 *  traveling down the trie, all exact/minimum occurrence bounds have been met
 *  and verifies the word.
 * 
 * @param sfx       suffix of the word, use a leaf node's string
 * @param s         guess string
 * @param eval      eval of the guess
 * @param occs      precomputed and possibly modified occurrence bounds imposed by eval
 * @param depth     current "level" of the trie
 * @return int      1 = word is eligible    0 = word is not eligible
 */
static int check_prev(char *sfx, char *s, char *eval, int *occs, int depth){
    int i, index, count, res;

    // once we run out of suffix, check that all occs are either -1 or 0
    if (sfx[0] == '\0'){
        for (i = 0; s[i] != '\0'; ++i){ // only iterate through guess
            index = conversion_table[(int) s[i]];
            if (occs[index] != 0 && occs[index] != -1) return 0;
        }
        return 1;
    }

    // otherwise, check that sfx[0] is compatible with the bounds
    index = conversion_table[(int) sfx[0]];
    count = occs[index];
    if (( count == 0 )                              ||  // letter can't occur
        (eval[depth] == '+' && sfx[0] != s[depth])  ||  // not exact match
        (eval[depth] != '+' && sfx[0] == s[depth])      // matches inexact match
    ) return 0;
    
    // recursively call, modifying occs array
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
/**
 * @brief Recursively prune the trie based on previous guess
 * 
 *  Travels down the trie while moving along the word, calls itself until a leaf
 *  node is found. Every time it calls itself recursively on a trie node, it
 *  modifies the occs array if the occurrences are unbound: if the word must
 *  have at least 3 "a" and we find the first "a" node, when calling the function
 *  we set occs so that, from then on, the requirement become at least 2 "a".
 *  Once a leaf is reached, the suffix is similarly checked by check_prev().
 * 
 *  This way of filtering saves a lot of time because it ignores pruned nodes,
 *  hence avoiding entire sections of the tree altogether.
 * 
 *  When called on the root of the trie, returns the total number of valid nodes
 *  it contains after pruning.
 * 
 * @param trie      root of the dictionary to prune
 * @param s         guess string
 * @param eval      eval of the guess
 * @param occs      precomputed occurrence bounds imposed by eval
 * @param depth     current "level", start at 0
 * @return int      number of nodes beneath trie that pass the bounds
 */
static int prune_prev(trie_t *trie, char *s, char *eval, int *occs, int depth){
    trie_t *curr;
    int index, count, res, total = 0;
    char target;

    target = s[depth];

    for (curr = trie; curr != NULL; curr = curr->next){
        
        if ((curr->status)[0] == NO_PRUNE){  // don't consider TEMP_PRUNE and PRUNE

            if ((eval[depth] == '+' && (curr->status)[1] == target)  ||
                (eval[depth] != '+' && (curr->status)[1] != target)
            ) {
    
                index = conversion_table[(int) (curr->status)[1]];
                count = occs[index];

                if (count == 0) (curr->status)[0] = PRUNE;
                else if (curr->branch == NULL){ // reached a leaf
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
    
                } else {                        // branch down
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

                    // temporarily prune branches without any valid leaf beneath
                    if (total == 0) (curr->status)[0] = TEMP_PRUNE;
                }
            } else (curr->status)[0] = PRUNE;
        }
    }

    return total; 
}


/**
 * @brief Recursively check word suffix based on all previous guesses
 * 
 *  Very similar to check_prev(). Main difference is the base case where it needs
 *  to iterate through the entire reqs->occs array of 64 values instead of just
 *  the guess string, to make sure that all bounds be checked.
 * 
 * @param sfx       suffix of the word, use a leaf node's string
 * @param reqs      requirements struct pointer
 * @param depth     current "level" of the trie
 * @return int      1 = word is eligible    0 = word is not eligible
 */
static int check_full(char *sfx, req_t *reqs, int depth){
    int i, index, count, res;

    // once we run out of suffix, check that all occs are either -1 or 0
    if (sfx[0] == '\0'){
        for (i = 0; i < CHARSET; ++i){  // iterate through all of occs
            if ((reqs->occs)[i] != 0 && (reqs->occs)[i] != -1) return 0;
        }
        return 1;
    }

    // otherwise, check that sfx[0] is compatible with the bounds
    index = conversion_table[(int) sfx[0]];
    count = (reqs->occs)[index];
    if (( count == 0 )                                                      ||  // letter can't occur
        ((reqs->match)[depth] != '*' && sfx[0] != (reqs->match)[depth])     ||  // inexact match
        ((reqs->pos)[index][depth] == 0)                                        // position unavailable
    ) return 0;
    
    // recursively call, modifying the occs array
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

/**
 * @brief Recursively prune the trie based on all previous guesses
 * 
 *  Very similar to prune_prev(), the only difference is having to check bounds
 *  against the requirements struct instead of the occs array and the guess/eval.
 *  reqs->occs is fundamentally equivalent to occs in the other function.
 *  When reaching leaves it calls check_full().
 * 
 *  When called on the root of the trie, returns the total number of valid nodes
 *  it contains after pruning.
 * 
 * @param trie      root of the dictionary to prune
 * @param reqs      requirements struct pointer
 * @param depth     current "level", start at 0
 * @return int      number of nodes beneath trie that pass the bounds
 */
static int prune_full(trie_t *trie, req_t *reqs, int depth){
    trie_t *curr;
    int index, count, res, total = 0;
    char target;

    target = (reqs->match)[depth];

    for (curr = trie; curr != NULL; curr = curr->next){
        
        if ((curr->status)[0] != PRUNE){  // only consider TEMP_PRUNE and NO_PRUNE

            if ((target != '*' && (curr->status)[1] == target)  || (target == '*')) {
    
                index = conversion_table[(int) (curr->status)[1]];
                count = (reqs->occs)[index];

                if (count == 0 || (reqs->pos)[index][depth] == 0) (curr->status)[0] = PRUNE;
                else if (curr->branch == NULL) {    // reached a leaf
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
    
                } else {                            // branch down
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

                    // resets nodes that were temporarily pruned when they get valid leaves
                    if ((curr->status)[0] == TEMP_PRUNE && total > 0) (curr->status)[0] = NO_PRUNE;
                }
            } else (curr->status)[0] = PRUNE;
        }
    }

    return total; 
}

/**
 * @brief Reads the first word list
 * 
 *  For some reason an insertion command can be found between the initial dict
 *  and the first game, this function behaves as if the words to insert where
 *  in the initial dictionary and simply dumps the +inserisci_inizio and 
 *  +inserisci_fine stirngs.
 * 
 * @param trie      root of the trie to insert the words into (should be NULL)
 * @param wordsize  size of the words to read
 * @return trie_t*  root of the trie after insertion
 */
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

/**
 * @brief Reads and inserts words into dictionary until +inserisci_fine
 * @param trie      root of the trie to insert the words into
 * @param wordsize  size of the words to read
 * @return trie_t*  root of the trie after insertion
 */
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

/**
 * @brief Performs a full game loop
 * 
 *  By far the ugliest function in the entire project, but very functional.
 *  Each iteration reads an input line, which can either be a command or a guess.
 *  
 *      +inserisci_inizio: inserts to the dictionary and sets an insert flag
 *      +stampa_filtrate:  prints the dictionary. if the insert flag was set
 *                         it first does a full prune of the dictionary
 * 
 *  Guesses are first checked against the ref string, then searched in the
 *  dictionary, and only then the evaluation is computed. If the insert flag
 *  was set, a full_prune() is executed and the flag is reset, else as long
 *  as the prune flag is set a simpler prev_prune() is performed. The prune
 *  flag is used to avoid pruning when there is only one eligible word in the
 *  dictionary (ref) but this is overridden by the insert flag (although prune
 *  will be once again set to 0 if after a full_prune the count remains 1)
 * 
 *  At the end of the game it also handles a possible +inserisci_inizio before
 *  the next match. If +nuova_partita is read, it frees requirements and resets
 *  the trie. If the input is over it returns NULL and does not free anything.
 * 
 * @param trie      root of the dictionary
 * @param wordsize  size of the words in the trie
 * @return trie_t*  root of the updated dictionary or NULL if input is over
 */
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
    
            if (buff[1] == 's'){                        // +stampa_filtrate
                if (wordsize <= 16) while (getchar() != '\n');

                if (insert_flag) {
                    count = prune_full(trie, reqs, 0);

                    insert_flag = 0;                    // reset insert flag
                    if (count == 1) prune_flag = 0;     // set prune flag
                }
                print_trie(trie, wordsize);

            } else if (buff[1] == 'i'){                 // +inserisci_inizio
                if (wordsize <= 17) while (getchar() != '\n');
    
                trie = handle_insert(trie, wordsize);
                insert_flag = 1;                        // set insert flag
                if (!prune_flag) prune_flag = 1;        // reset prune flag
            }

        } else{

            getchar(); // trailing newline for words

            if (strcmp(ref, buff) == 0) {               // guessed correctly
                puts("ok");
                break;
            } else if (search(trie, buff) == 0) {       // word not in dict
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
