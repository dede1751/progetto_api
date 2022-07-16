#ifndef GAME_H_
#define GAME_H_

#include "trie.h"

// /*
//  *      REQUIREMENTS:
//  *  - match[i] = '*'    if the exact value of the i-th position hasn't been found
//  *    match[i] = char   if char has been discovered
//  * 
//  *  - occ[i]:
//  *         -1     --->    number of occurrences of i-th char is not bound
//  *          0     --->    i-th char does not appear
//  *        x < -1  --->    i-th char appears at least (-x - 1) times, 
//  *                        e.g. if -3 at least twice
//  *        x > 0   --->    i-th char appears exactly x times.
//  *
//  *  - pos[char][i] = 1  char can occupy i-th position
//  *    pos[char][i] = 0  char cannot occupy i-th position
//  */

// typedef struct reqs {
//     char *match;
//     int8_t occ[CHARSET];
//     uint8_t *pos[CHARSET];
// } req_t;

// typedef req_t *req_ptr;

// /*
//  *      NEW_GAME:
//  *  - fills the list from the tree and generates a requirements pointer
//  *  - accepts guesses and commands and reacts accordingly
//  *  - once the game terminates, if the list is further expanded it handles that
//  *    and returns:
//  *      0  --> no new game command was received
//  *      1  --> initiate a new game
//  *  - handles emptying the list and freeing the requirements
//  */

// needed for errorless compilation
void safe_fgets(char *, int);
void safe_scanf(int *);

/* Maps/Inverts map on 0-63 interval according to ASCII order */
uint8_t map_charset(char);
char unmap_charset(int);

char *calculate_eval(char *, char *, int, char *);
int prune_trie(trie_t *, char *, char *);

trie_t *initial_read(trie_t *, int);
// uint8_t new_game(dict_ptr, int, FILE *, FILE *);

#endif
