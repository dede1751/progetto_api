#include "game.h"

static void safe_fgets(char *, uint8_t);

static req_t *generate_reqs(uint8_t);
static void free_reqs(req_t *);

static void eval_guess(char *, uint8_t , req_t *);

static uint8_t check_leaf(char *, req_t *, uint8_t);
static int prune_trie(trie_t *, req_t *, uint8_t);

static trie_t *handle_insert(trie_t *, uint8_t);

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
static uint8_t conversion_table[128] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 0, 64, 64, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 64,
    64, 64, 64, 64, 64, 64, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 64, 64, 64, 64, 37, 64,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 61, 62, 63, 64, 64, 64, 64, 64
};
static uint8_t insert_flag = 0;


// compiler throws errors without these
static void safe_fgets(char *s, uint8_t size){
    if (fgets(s, size + 1, stdin) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(uint8_t *x){
    if (scanf("%hhu\n", x) == 0) exit(EXIT_FAILURE);
}

/**
 * @brief Allocate and initialize requirements struct
 * @param wordsize  size of the words in the trie
 * @return req_t*   pointer to the requirements struct
 */
static req_t *generate_reqs(uint8_t wordsize){
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

/**
 * @brief Free the requirements struct and its contents
 * @param reqs      pointer to the struct to free
 */
static void free_reqs(req_t *reqs){
    uint8_t i;
    
    free(reqs->ref);
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free((reqs->pos)[i]);
    free(reqs);
}

/** @brief Prints evaluation and modifies requirements accordingly
 *
 *  Handles both evaluation and the requirements struct:
 *  evaluation is done by counting occurrences in the ref string and then, for
 *  each character in the s string, choosing '\' or '|' based on occurrences
 *  left. It then prints the eval string.
 * 
 *  Requirements calculation happens in multiple stages:
 *      - 1st pass, REF  --> matching characters, count letters in ref (not exact)
 *      - 2nd pass, S    --> impossible positions whenever we don't have match
 *                           computes '|' and '/' of eval using occs
 *      - 3rd pass, S    --> compute occs again, with min/exact distinction
 *      - 4th pass, S    --> after computing occs, replace constraints in reqs
 *                           if occs contains stricter bounds
 * 
 * @param s         guess string.
 * @param wordsize  size of the words in input
 * @param reqs      pointer to the requirements struct.
 */
static void eval_guess(char *s, uint8_t wordsize, req_t *reqs){
    int8_t occs[CHARSET] = {0};
    char eval[wordsize + 1], *ref = reqs->ref;
    uint8_t i, index;

    // count char occurrences in ref and handle perfect matches
    for(i = 0; i < wordsize; ++i){
        index  = conversion_table[(int) ref[i]];

        if (ref[i] != s[i]) ++(occs[index]);
        else {
            eval[i] = '+';
            (reqs->match)[i] = s[i];
        }
    }

    // handle imperfect matches and exclusions
    for (i = 0; i < wordsize; ++i){
        if (eval[i] != '+'){
            index = conversion_table[(int) s[i]];
            if (occs[index] == 0) eval[i] = '/';
            else {
                eval[i] = '|';
                --(occs[index]);
            }

            // get rid of impossible positions according to eval
            (reqs->pos)[index][i] = 0;
        }    
    }
    eval[wordsize] = '\0';

    // once eval is computed, get the occurrences it imposes
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

    // use occs to fix up requirements
    for (i = 0; i < wordsize; ++i){
        index = conversion_table[(int) s[i]];

        // modify reqs->occs[index] if the new bounds are "stricter"
        if ((reqs->occs)[index] < 0                                  &&
            (occs[index] >= 0 || occs[index] < (reqs->occs)[index])
        ) (reqs->occs)[index] = occs[index];
    }

    puts(eval);
}

/**
 * @brief Recursively check word suffix based on all previous guesses
 * 
 *  Recursively travels down the suffix modifying the occs array like in
 *  prune_prev(), and when it runs out it checks that the occurrences for all
 *  the letters in the ref string are either -1 or 0: this means that, while
 *  traveling down the trie, all exact/minimum occurrence bounds have been met
 *  and verifies the word.
 * 
 * @param sfx       suffix of the word, use a leaf node's string
 * @param reqs      requirements struct pointer
 * @param depth     current "level" of the trie
 * @return uint8_t  1 = word is eligible    0 = word is not eligible
 */
static uint8_t check_leaf(char *sfx, req_t *reqs, uint8_t depth){
    uint8_t index, res;
    int8_t count;
    char* ref;

    // once we run out of suffix, check that all occs are either -1 or 0
    if (sfx[0] == '\0'){
        for (ref = (reqs->ref); *ref != '\0'; ++ref){  // iterate over occs through ref
            index = conversion_table[(int) *ref];
            count = (reqs->occs)[index];
            if (count != 0 && count != -1) return 0;
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

/**
 * @brief Recursively prune the trie based on all previous guesses
 * 
 *  Travels down the trie while moving along the word, calls itself until a leaf
 *  node is found. Every time it calls itself recursively on a trie node, it
 *  modifies the occs array if the occurrences are unbound: if the word must
 *  have at least 3 "a" and we find the first "a" node, when calling the function
 *  we set occs so that, from then on, the requirement become at least 2 "a".
 *  Once a leaf is reached, the suffix is similarly checked by check_leaf().
 * 
 *  This way of filtering saves a lot of time because it ignores pruned nodes,
 *  hence avoiding entire sections of the tree altogether.
 * 
 *  When called on the root of the trie, returns the total number of valid nodes
 *  it contains after pruning.
 * 
 * @param trie      root of the dictionary to prune
 * @param reqs      requirements struct pointer
 * @param depth     current "level", start at 0
 * @return int      number of nodes beneath trie that pass the bounds
 */
static int prune_trie(trie_t *trie, req_t *reqs, uint8_t depth){
    trie_t *curr;
    uint8_t index, res;
    int8_t count;
    int total = 0;
    char target;

    for (curr = trie; curr != NULL; curr = curr->next){
        
        if (((curr->status)[0] == NO_PRUNE)                   || // valid node
            (insert_flag && (curr->status)[0] == TEMP_PRUNE)     // potentially valid
        ){

            target = (reqs->match)[depth];
            if ((target != '*' && (curr->status)[1] == target) || (target == '*')) {
    
                index = conversion_table[(int) (curr->status)[1]];
                count = (reqs->occs)[index];
    
                // prune if no occurrences left or incorrect position
                if (count == 0 || (reqs->pos)[index][depth] == 0) (curr->status)[0] = PRUNE;
                else if (curr->branch == NULL) {    // reached a leaf
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
    
                } else {                            // branch down
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

                    // resets nodes that were temporarily pruned when they get valid leaves
                    if ((curr->status)[0] == TEMP_PRUNE && total > 0) (curr->status)[0] = NO_PRUNE;
                    else if (total == 0) (curr->status)[0] = TEMP_PRUNE;
                }
            } else (curr->status)[0] = PRUNE;
        }
    }

    return total;
}

/**
 * @brief Reads and inserts words into dictionary until +inserisci_fine
 * @param trie      root of the trie to insert the words into
 * @param wordsize  size of the words to read
 * @return trie_t*  root of the trie after insertion
 */
static trie_t *handle_insert(trie_t *trie, uint8_t wordsize){
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
trie_t *initial_read(trie_t *trie, uint8_t wordsize){
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
        trie = handle_insert(trie, wordsize);
        while(getchar() != '\n');
    }
    free(buff);

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
 *  dictionary, and only then the evaluation is computed. 
 * 
 *  At the end of the game it also handles a possible +inserisci_inizio before
 *  the next match. If +nuova_partita is read, it frees requirements and resets
 *  the trie. If the input is over the program exits succesfully
 * 
 * @param trie      root of the dictionary
 * @param wordsize  size of the words in the trie
 * @return trie_t*  root of the updated dictionary
 */
trie_t *new_game(trie_t *trie, uint8_t wordsize){
    req_t *reqs;
    uint8_t guesses;
    int count = 0;
    char buff[wordsize + 1];
    char dump[18];

    insert_flag = 0;                // reset insert flag
    reqs = generate_reqs(wordsize); // init reqs, reads ref
    safe_scanf(&guesses);           // read guesses

    while(guesses > 0){
        safe_fgets(buff, wordsize);

        if(buff[0] == '+'){
    
            if (buff[1] == 's'){                        // +stampa_filtrate
                if (wordsize <= 16) while (getchar() != '\n');

                if (insert_flag) {
                    count = prune_trie(trie, reqs, 0);
                    insert_flag = 0;                    // reset insert flag
                }
                print_trie(trie, wordsize);

            } else if (buff[1] == 'i'){                 // +inserisci_inizio
                if (wordsize <= 17) while (getchar() != '\n');

                trie = handle_insert(trie, wordsize);
                insert_flag = 1;                        // set insert flag
                count = 0;                              // reset count
            }

        } else{

            getchar(); // trailing newline for words

            if (strcmp(reqs->ref, buff) == 0) {         // guessed correctly
                puts("ok");
                break;
            } else if (search(trie, buff) == 0) {       // word not in dict
                puts("not_exists");
            } else {
                eval_guess(buff, wordsize, reqs);       // print eval, get reqs

                if (count != 1) count = prune_trie(trie, reqs, 0);
                if (insert_flag) insert_flag = 0;       // fixed TEMP_PRUNE
                
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

        // free/clear only when restarting
        free_reqs(reqs);
        clear_trie(trie);
    } return trie;
}
