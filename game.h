#ifndef GAME_H_
#define GAME_H_

#include "trie.h"

/*
 *      REQUIREMENTS:
 *  - match[i] = '*'    if the exact value of the i-th position hasn't been found
 *    match[i] = char   if char has been discovered
 * 
 *  - occs[i]:
 *         -1     --->    number of occurrences of i-th char is not bound
 *          0     --->    i-th char does not appear
 *        x < -1  --->    i-th char appears at least (-x - 1) times, 
 *                        e.g. if -3 at least twice
 *        x > 0   --->    i-th char appears exactly x times.
 *        
 *    occurrences are meant as "free" occurrences, and do not include matching
 *    letters
 *
 *  - pos[char][i] = 1  char can occupy i-th position
 *    pos[char][i] = 0  char cannot occupy i-th position
 */

typedef struct reqs {
    char *match;
    int8_t occs[CHARSET];
    uint8_t *pos[CHARSET];
} req_t;


// needed for errorless compilation
void safe_fgets(char *, int);
void safe_scanf(int *);

// Maps/Inverts map on 0-63 interval according to ASCII order
int map_charset(char);
char unmap_charset(int);

void handle_simple_guess(trie_t *, int, char *, char *, req_t *);
void handle_full_guess(trie_t *, int, char *, char *, req_t *);

void generate_req(int, req_t *);
void free_req(req_t *);

trie_t *initial_read(trie_t *, int);
// uint8_t new_game(dict_ptr, int, FILE *, FILE *);

#endif
