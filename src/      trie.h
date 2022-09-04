/**
 * @file trie.h
 * @author Andrea Sgobbi
 * @date 19 July 2022
 * @brief Header containing trie definition and operations
 *
 *  The solution makes use of a trie (TST) to easily filter the words. The basic
 *  trie functionality is implemented in this module, while the rest of the more
 *  advanced pruning functions are left to the game module since they make use
 *  of the requirements struct.
 * 
 *  Trie insert/search are both in constant time but with pretty enormous constants
 *  compared to things like hash tables since a dyanmic trie means you could have
 *  to navigate up to 64 nodes to find a letter. A static, array-based approach
 *  will not work due to size constraints, even if resizing arrays. This solution
 *  barely makes the space constraints.
 * 
 *  The project specifies that all strings be the same size, so this trie does
 *  not support insertion of strings of different lengths.
 */
#ifndef TRIE_H_
#define TRIE_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CHARSET 64
#define PRUNE 3
#define TEMP_PRUNE 2
#define NO_PRUNE 1


/** @brief Dynamic trie with status array containing string and pruning info
 * 
 *      TRIE (dynamic, partially compressed at the leaves):
 *
 *  * STATUS:  1B = prune | (1+)B = suffix
 *  |
 *  | - to save space, the trie is compressed at the leaves. instead of building
 *  |   a chain of nodes linking to a single nodes the rest of the word is saved
 *  |   in the suffix
 *  | - for optimal packing, the prune byte is saved within the suffix at the
 *  |   first address. this is because keeping it within the struct would push
 *  |   its size to 25B which would then be rounded to 32B, completely wasting
 *  |   7B for padding between structs
 *  |
 *  |
 *  * PRUNE:
 *    - the prune bit is used when filtering the tree, by pruning the root of
 *      a branch the rest of it does not need to be verified. on top of that
 *      adding new words to a pruned branch means those words won't have to be
 *      checked in the future.
 *    - for debugging purposes it uses the values PRUNE/TEMP_PRUNE/NO_PRUNE, the
 *      special TEMP_PRUNE value is used to prune branches with no valid leaves
 *      under them. Since these are not properly pruned, this must be reset at
 *      every insertion, and is handled by prune_trie
 * 
 *
 *  * NODES:
 * 
 *    - each node always has at least 2 chars in status. use:
 *      (node->status)[1] ---> letter that represents the node
 *    - eache level of the trie is a linked list of nodes, accessible through the
 *      next field
 *    - to travel down the trie, use the branch field.
 *
 *      + LEAVES:
 *   1) trie->branch == NULL
 *   2) trie->status contains the prune char and a suffix string of len >=1 with
 *      a terminating null character
 * 
 *      + BRANCHES:
 *   1) trie->branch != NULL
 *   2) trie->status DOES NOT have any terminating null character, and consists
 *      of only two chars, one for prune bit and the other for the "index" of
 *      the node to navigate the trie.
 */
typedef struct trie {
    struct trie *next;
    struct trie *branch;
    char *status;
} trie_t;

/**
 * @brief Inserts string into trie and returns updated trie     O(k)
 * 
 *  To initialize the tree pass a NULL node as the root. All inserted words must
 *  be unique, this is a requirement for the project input.
 * 
 * @param root      root of the trie to insert the string in
 * @param word      word to save on the trie
 * @return trie_t*  returns the new root
 */
trie_t *insert(trie_t *, char *);

/**
 * @brief Searches trie for target string                       O(k)
 * @param root      root of the trie to search the string in
 * @param word      word to search in the trie
 * @return int      1 = found  0 = not found
 */
int search(trie_t *, char *);

/**
 * @brief Prints the trie lexicographically in column           O(n)
 * @param trie      root of the trie to print
 * @param wordsize  size of the words in the trie
 */
void print_trie(trie_t *, uint8_t);

/**
 * @brief Resets prune values in the trie to NO_PRUNE           O(n)
 * @param trie      root of the trie to reset
 */
void clear_trie(trie_t *);

#endif