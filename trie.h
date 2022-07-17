#ifndef TRIE_H_
#define TRIE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CHARSET 64
#define PRUNE 1
#define NO_PRUNE 2
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


/*
 *      TRIE (dynamic)
 *
 *  * STATUS  1B = prune | (1+)B = suffix
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
 *  * PRUNE
 *    - the prune bit is used when filtering the tree, by pruning the root of
 *      a branch the rest of it does not need to be verified. on top of that
 *      adding new words to a pruned branch means those words won't have to be
 *      checked in the future.
 *    - for debugging purposes it uses the values PRUNE/NO_PRUNE, treat as 1/0
 * 
 *
 *  * NODES
 * 
 *    - each node always has at least 2 chars in status. use:
 *          (node->status)[1] ---> letter that represents the node
 *    - eache level of the trie is a linked list of nodes, accessible through the
 *      next field
 *    - to travel down the trie, use the branch field.
 *
 *      * LEAVES:
 *   1) trie->branch == NULL
 *   2) trie->status contains the prune char and a suffix string of len >=1 with
 *      a terminating null character
 * 
 *      * BRANCHES:
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


trie_t *insert(trie_t *, char *);
int search(trie_t *, char *);

void print_trie(trie_t *, int);
void clear_trie(trie_t *);

#endif