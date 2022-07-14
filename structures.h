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
 *  - PRUNE   1b = prune | 6b = size | 1b = ...
 *      
 *      *size  --> size of the trie->nodes array, it is allocated to be small
 *                 enough to only hold its highest index
 *      *prune --> 0 = entire branch unsuitable
 *                 1 = branch still suitable
 *                 trie->prune |= 0x01  set prune bit
 *                 trie->prune &= 0xFE  clear prune bit
 * 
 *    - leaves will only have the prune bit set, while size always 0
 *    - the prune bit is used when filtering the tree, by pruning the root of
 *      a branch the rest of it does not need to be verified. on top of that
 *      adding new words to a pruned branch means those words won't have to be
 *      checked in the future.
 * 
 *  - SUFFIX
 *
 *    - to save space, the trie is compressed at the leaves. instead of building
 *      a chain of nodes linking to a single nodes the rest of the word is saved
 *      in the suffix
 * 
 *  - NODES
 * 
 *    - list of up to 64 pointers to children.
 *    - size = (trie->prune)>>1
 * 
 */

typedef struct n {
    struct n **nodes;
    char *suffix;
    uint8_t prune;
} node_t;


/* Maps/Inverts map on 0-63 interval according to ASCII order */
uint8_t map_charset(char);
char unmap_charset(int);

node_t *generate_trie();

void insert(node_t *, char *);
uint8_t search(node_t *, int, char *);
int prune_trie(node_t *, char *, char *);

void print_trie(node_t *, int);

#endif