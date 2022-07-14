#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CHARSET 64
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


/*
 *      TRIE:
 *
 *  * STATUS  1B = prune | ?B = suffix
 *  |
 *  | - to save space, the trie is compressed at the leaves. instead of building
 *  |   a chain of nodes linking to a single nodes the rest of the word is saved
 *  |   in the suffix
 *  | - for optimal packing, the prune byte is saved within the suffix at the
 *  |   first address. this is because keeping it within the struct would push
 *  |   its size to 17B which would then be rounded to 24B, completely wasting
 *  |   7B for padding between structs
 *  |
 *  |
 *  * PRUNE   1b = prune | 6b = size | 1b = ...
 *      
 *      *size  --> size of the trie->nodes array, it is allocated to be small
 *                 enough to only hold its highest index
 *      *prune --> 0 = entire branch unsuitable
 *                 1 = branch still suitable
 *                 *(trie->status) |= 0x01  set prune bit
 *                 *(trie->status) &= 0xFE  clear prune bit
 * 
 *    - leaves will only have the prune bit set, while size always 0
 *    - the prune bit is used when filtering the tree, by pruning the root of
 *      a branch the rest of it does not need to be verified. on top of that
 *      adding new words to a pruned branch means those words won't have to be
 *      checked in the future.
 * 
 *
 *  * NODES
 * 
 *    - list of up to 64 pointers to children.
 *    - size = *(trie->status)>>1
 *
 *      * LEAVES:
 *   1) trie->nodes == NULL
 *   2) trie->status contains the prune char and at least one suffix char ('\0')
 * 
 *      * BRANCHES:
 *   1) trie->nodes != NULL
 *   2) trie->status only contains the prune char, no extra suffix space
 */

typedef struct n {
    struct n **nodes;
    char *status;
} node_t;


/* Maps/Inverts map on 0-63 interval according to ASCII order */
uint8_t map_charset(char);
char unmap_charset(int);

node_t *initialize_trie();

void insert(node_t *, char *);
uint8_t search(node_t *, int, char *);
//int prune_trie(node_t *, char *, char *);

void print_trie(node_t *, int);

#endif