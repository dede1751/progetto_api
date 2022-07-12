#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CHARSET 64

/*
 *      TRIE:
 *  - A prune bit is used for keeping track of the uneligible "branches" of the
 *    trie: if our eval is "/+///" with the second letter a, we can prune all
 *    nodes on the second level of the trie except for the 'a' nodes, and do
 *    likewise for the letters we exclude with '/' in the other positions.
 *  - When sequentially processing guesses, we can prune based simply on the
 *    eval of each guess.
 *  - When we encounter an insertion command 
 */

typedef struct n {
    struct n **nodes;
    uint8_t prune;
} node_t;

typedef node_t *node_ptr;


/* Maps characters on 0-63 interval according to ASCII order */
uint8_t map_charset(char);

node_ptr generate_node();
void insert(node_ptr, int, char *);
uint8_t search(node_ptr, int, char *);

int prune_trie(node_ptr, char *, char *);

void print_trie(node_ptr, int);

#endif