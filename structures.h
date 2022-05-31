#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
 *      STRUCTURES:
 *  - Tree (RB):
 *      * permanent structure, always contains the full dictionary
 *      * lookup and insertion are logarithmic
 *  - List (single-link):
 *      * linked at game runtime, rapidly reduces in size due to constraints
 *        introduced by each guess
 *      * only used for keeping track of which words are still plausible
 *      * lookup and insertion are linear
 *      * length is kept track of within the structure to avoid having to count
 * 
 *  - the structures are built on top of the same nodes. The list is traversed
 *    starting from T->head and accessing the next fields, the tree from T->root
 *    and using left/right fields
 */

typedef struct node{
    char *word;
    uint8_t color; // 0 = black 1 = red
    struct node *left;
    struct node *right;
    struct node *parent;
    struct node *next;
} node_t;

typedef node_t *node_ptr;

typedef struct dict {
    node_ptr root;
    node_ptr NIL;
    node_ptr head;
    int len;
} dict_t;

typedef dict_t *dict_ptr;

dict_ptr generate_dict();

/*
 *      RB TREE OPERATIONS      ---- O(logn):
 *  - insertion is self balanced using insert_fixup, returns new node
 *  - search based on strcmp
 */
node_ptr insert(dict_ptr, char*);
uint8_t search(dict_ptr, char*);

/*
 *      LINKED LIST OPERATIONS ---- O(n):
 *  - list is single linked and reuses string pointers for small footprint
 *     (double link unnecessary because deletion is done sequentially over
 *      the whole list)
 *  - fill_list parses the tree in reverse order and sequentially builds the
 *     list from it
 *  - for further insertions ordered-insert is used
 */
void fill_list(dict_ptr, node_ptr);
void ordered_insert(dict_ptr, node_ptr);
void print_list(dict_ptr, FILE *);
    
/*
 *      GARBAGE COLLECTION:
 *  - reset_list empties the list (head = NULL, len = 0) without deallocating
 *    the list pointer
 *  - free_dict deallocates the entire tree struct and the list pointer
 *    (program assumes it has already been emptied)
 */
void reset_list(dict_ptr);
void free_dict(dict_ptr);

#endif