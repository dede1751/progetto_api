/**
 * @file game.h
 * @author Andrea Sgobbi
 * @date 19 July 2022
 * @brief Header containing game operations
 *
 *  This includes functions used by the main game loop to do the initial batch
 *  read of the dictionary and to handle each match. It also defines the req_t
 *  type used to save the various "bounds" that can be derived from each guess.
 *  
 *  This structure is only needed because insertions can happen mid match: in
 *  all other cases, say with guess_1 and guess_2, if we first filter the trie
 *  based solely on the outcome of guess_1, and filter again based on guess_2,
 *  we get the same result as merging the requirements of guess_1 and guess_2
 *  and filtering based on those (much more expensive). Insertion breaks this
 *  "transitivity" property of the filtering.
 */
#ifndef GAME_H_
#define GAME_H_

#include "trie.h"

/** @brief Struct to save constraints imposed by guesses throughout each game
 * 
 *      REQUIREMENTS:
 *
 *  - ref:              reference string for the current game
 * 
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
 *   occurrences must be counted as total occurrences. this is because the
 *   following situation can't be solved efficiently when counting "free" occs
 *   (which means excluding matching letters from the count)
 *  
 *     ref = "abc"
 *     s1  = "add" --> eval = "+//"   occs["a"] = -1
 *     s2  = "dad" --> eval = "|//"   occs["a"] = -2
 * 
 *    now we would have to check the whole (reqs->match) string to see that 'a'
 *    appears once in there and hence we don't actually have a "free" occurrence
 *    of 'a'. this would be far too inefficient
 *
 *  - pos[char][i] = 1  char can occupy i-th position
 *    pos[char][i] = 0  char cannot occupy i-th position
 */
typedef struct reqs {
    char *ref;
    char *match;
    int8_t occs[CHARSET];
    uint8_t *pos[CHARSET];
} req_t;


// needed for errorless compilation, used only for wordsize outside
void safe_scanf(uint8_t *);

/**
 * @brief Reads initial dictionary and returns filled trie
 * 
 *  Reads the entire initial dictionary until +nuova_partita, or in case of
 *  +inserisci_inizio also handles that. Returns control flow after having fully
 *  read +nuova_partita from buffer
 *
 * @param trie      init to NULL (lazy ik)
 * @param wordsize  size of the words in input(read previously)
 * @return trie_t*  root of the filled dictionary
 */
trie_t *initial_read(trie_t *, uint8_t);

/**
 * @brief Performs a full game loop
 * 
 *  Input buffer must start with ref string, and clears the input up until the
 *  next +nuova_partita(included). If the input ends it exits succesfully
 * 
 * @param trie      root of the dictionary
 * @param wordsize  size of the words in input
 * @return trie_t*  root of the updated trie
 */
trie_t *new_game(trie_t *, uint8_t);

#endif
