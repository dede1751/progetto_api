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
 *    occurrences must be counted as total occurrences. this is because of the
 *    following situation can't be solved efficiently when counting "free" occs:
 * 
 *      ref = "abc"
 *      s1  = "add" --> eval = "+//"   occs["a"] = -1
 *      s2  = "dad" --> eval = "|//"   occs["a"] = -2
 * 
 *    now we would have to check the whole (reqs->match) string to see that 'a'
 *    appears once in there and hence we don't actually have a "free" occurrence
 *    of 'a'. this would be far too inefficient
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
void safe_scanf(int *);

trie_t *initial_read(trie_t *, int);
trie_t *new_game(trie_t *, int);

#endif
