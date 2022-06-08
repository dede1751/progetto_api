#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#define BASE_SIZE 65536
#define LIMIT_RATIO 0.5867f

/*
 *      STRUCTURES:
 *  - Hash Table:
 *      * permanent structure, always contains the full dictionary
 *      * lookup and insertion are ideally in constant time
 *      * table is rehashed with double the size when exceeding the limit load
 *        of 2 - sqrt(2)
 *  - List (single-link):
 *      * linked at game runtime, rapidly reduces in size due to constraints
 *        introduced by each guess
 *      * only used for keeping track of which words are still plausible
 *      * lookup and insertion are linear
 *      * length is kept track of within the structure to avoid having to count
 * 
 *  - the structures are built on top of the same nodes. The list is traversed
 *    starting from D->head and accessing the next fields, the tree from D->table
 *    through hashing.
 */

typedef struct n {
    char *word;
    struct n *chain;
    struct n *next;
} node_t;
typedef node_t *node_ptr;

typedef struct d {
    node_ptr *table;
    int size;
    int items;
    int collisions;

    node_ptr head;
    int len;
} dict_t;
typedef dict_t *dict_ptr;

extern double insert_time;
extern double insert_ord_time;
extern double search_time;
extern double fill_time;
extern double filter_time;
extern double total_time;
extern double print_time;

dict_ptr generate_dict();

/*
 *      HASH TABLE OPERATIONS ---- O(1 + load):
 *  - hashing is done with murmur OAAT
 *  - search based on strcmp
 */
node_ptr insert(dict_ptr, char*);
uint8_t search(dict_ptr, char*);

/*
 *      LINKED LIST OPERATIONS ---- O(n):
 *  - list is single linked and is embedded within the table nodes
 *     (double link unnecessary because deletion is done sequentially over
 *      the whole list)
 *  - out of order insertions are first buffered, ordered and then done 
 *    sequentially
 */
void quicksort(node_ptr *, node_ptr *);
void sequential_insert(dict_ptr, node_ptr *, int);
void print_list(dict_ptr, FILE *);
    

/*
 *      GARBAGE COLLECTION:
 *  - reset_list empties the list (head = NULL, len = 0)
 *  - free_dict deallocates the entire table
 *    (program assumes list has already been emptied)
 */
void reset_list(dict_ptr);
void free_dict(dict_ptr);

#endif