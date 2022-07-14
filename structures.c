#include "structures.h"

static node_t *generate_leaf(char *, int);
static node_t *generate_branch();
static node_t **realloc_null(node_t **, size_t, size_t);

static void resize_children(node_t *, int);
static void insert_leaf(node_t *, char *, int);
static void split_leaves(node_t *, char *);

// static int rec_prune(node_t *, char *, char *, int *, int);
static void rec_print(node_t *, char *, int);


uint8_t map_charset(char c){
    if (c == '-') return 0;
    else if (c >= '0' && c <= '9') return (c - 47);
    else if (c >= 'A' && c <= 'Z') return (c - 54);
    else if (c >= 'a' && c <= 'z') return (c - 59);
    else return 37; // maps '_'
}
char unmap_charset(int x){
    if (x == 0) return '-';
    else if (x >= 1 && x <= 10) return (x + 47);
    else if (x >= 10 && x <= 36) return (x + 54);
    else if (x >= 38 && x <= 63) return (x + 59);
    else return '_';
}


// initializes children to NULL, adds suffix if specified
static node_t *generate_leaf(char *sfx, int len){
    node_t *new = (node_t *)malloc(sizeof(node_t));

    // init prune and size to 0, suffix at least '\0'
    if (len == 0) new->status = (char *)calloc(2, sizeof(char));
    else {
        // add sfx to status (alloc 1 for prune, len+1 for sfx)
        new->status = (char *)calloc(len + 2, sizeof(char));
        memcpy((new->status) + 1, sfx, (len + 1)*sizeof(char));
    }

    new->nodes = NULL;
    return new;
}

static node_t *generate_branch(){
    node_t *new = (node_t *)malloc(sizeof(node_t));

    // init prune and size to 0, suffix blank
    new->status = (char *)calloc(1, sizeof(char));
    new->nodes = NULL;
    return new;
}

// initializes full children array (all set to NULL)
node_t *initialize_trie(){
    node_t *new = (node_t *)malloc(sizeof(node_t));
    new->status = (char *)malloc(sizeof(char));

    new->nodes = (node_t **)calloc(CHARSET, sizeof(node_t *));
    *(new->status) = (CHARSET - 1)<<1; // set size to 63
    return new;
}

// reallocates children array, sets to 0 only the new allocated memory
static node_t **realloc_null(node_t **nodes, size_t old_size, size_t new_size){
    node_t **new = realloc(nodes, new_size);

    memset(((char *)new) + old_size, 0, new_size - old_size);
    return new;
}

/*
 *          INSERTION:
 *
 */

static void resize_children(node_t *trie, int size){
    int tmp = *(trie->status)>>1;

    if (tmp < size){
        // realloc and set new memory to 0
        trie->nodes = realloc_null(
            trie->nodes,
            (tmp  + 1) * sizeof(node_t *),
            (size + 1) * sizeof(node_t *)
        );

        *(trie->status) &= 0x01;     // reset all but prune bit
        *(trie->status) += size<<1;  // set new size
    }
}

// word_sfx always contains at least one char before '\0'
static void insert_leaf(node_t *trie, char *word_sfx, int p){
    node_t *new;
    int len = strlen(word_sfx + 1);

    new = generate_leaf(word_sfx + 1, len);  // copy suffix if len > 0

    *(new->status) = p;
    trie->nodes[map_charset(*word_sfx)] = new;
}

static void expand_branch(node_t *trie, int index){

    trie->nodes = (node_t **)calloc(index + 1, sizeof(node_t *));
    trie->status = (char *)
    *(trie->status) = index<<1; // set prune to 0 and save new size

    trie->nodes[index] = generate_branch();

}

static void split_leaves(node_t *trie, char *word_sfx){
    char *old_sts = trie->status, *old_sfx = (trie->status) + 1;
    char prune = *(trie->status);
    int index;

    // continue making branches until sfx and word_sfx stop matching
    for (; *old_sfx == *word_sfx; ++old_sfx, ++word_sfx){
        index = map_charset(*old_sfx);

        trie->nodes = (node_t **)calloc(index + 1, sizeof(node_t *));
        *(trie->status) = index<<1; // set prune to 0 and save new size

        trie->nodes[index] = generate_branch();
        trie = trie->nodes[index];
    }

    // init children array to the max between two indices
    index = MAX(map_charset(*old_sfx), map_charset(*word_sfx));
    trie->nodes = (node_t **)calloc(index + 1, sizeof(node_t *));
    *(trie->status) = index<<1; // set prune to 0 and save new size


    insert_leaf(trie, old_sfx, prune);
    insert_leaf(trie, word_sfx, 0);

    free(old_sts);
}

void insert(node_t *trie, char *word){
    int index = map_charset(*word);

    // navigate through word to first NULL node
    while (
        trie->nodes != NULL          &&  // not a leaf node
        *(trie->status)>>1 >= index  &&  // index is accessible
        trie->nodes[index] != NULL       // child exists
        ){

        trie = trie->nodes[index];
        ++word;
        index = map_charset(*word);
    }

    if (trie->nodes == NULL) split_leaves(trie, word); // collided with another leaf
    else {
        resize_children(trie, index);
        insert_leaf(trie, word, 0); // no leaf already present
    }
}


uint8_t search(node_t *trie, int wordsize, char *word){
    int i, index;

    // iterate through the full word
    for (i = 0; i < wordsize; ++i){
        index = map_charset(word[i]);

        if (trie->nodes[index] == NULL) return 0;
        else trie = trie->nodes[index];
    }

    return 1;
}

/*
    OCCS[CHARSET]:
x = -1    -->  number of occurrences is not bound
x <  0    -->  occurs at least -x - 1 times (e.g. -3 > 2 occurrences)
x >= 0    -->  occurs exactly x times

occurrences do not include '+' matches
*/
// static void calculate_occs(char *s, char *eval, int *occs){
//     int i, index;

//     // set all occs to -1 (unbound)
//     for (i = 0; i < CHARSET; ++i) occs[i] = -1;

//     for (i = 0; s[i] != '\0'; ++i){
//         index = map_charset(s[i]);

//         // if '|' increase the minimum amount
//         if (eval[i] == '|' && occs[index] < 0) --(occs[index]);
//         // if '/' set the exact amount, -1 --> 0  -3 --> 2
//         else if (eval[i] == '/' && occs[index] < 0) {
//             occs[index] = -(occs[index]) - 1;
//         }
//     }
// }


// static int rec_prune(node_t *trie, char *s, char *eval, int *occs, int depth){
//     int i, index, count;

//     // branch has been pruned, ignore it
//     if (trie->prune == 1) return 0;

//     // reached a leaf
//     if (trie->nodes == NULL){
//         for (i = 0; s[i] != '\0'; ++i){
//             index = map_charset(s[i]);
//             if (occs[index] != 0 && occs[index] != -1){
//                 // not meeting exact/minimum occurrences of some letters
//                 trie->prune = 1;
//                 return 0;
//             }
//         }
//         return 1;
//     }

//     index = map_charset(s[depth]);
//     if (eval[depth] == '+'){
//         // prune all except nodes[index]
//         for (i = 0; i < CHARSET; ++i){
//             if (i != index && trie->nodes[i] != NULL){
//                 trie->nodes[i]->prune = 1;
//             }
//         }
//         // only call on correct letter, no need to adjust occurrences
//         if (trie->nodes[index] != NULL){
//             return rec_prune(trie->nodes[index], s, eval, occs, depth + 1);
//         } else return 0;

//     } else {
//         // prune only nodes[index]
//         for (i = 0, count = 0; i < CHARSET; ++i) {
//             if (trie->nodes[i] != NULL){
//                 //prune chars that do not appear or are misplaced
//                 if (occs[i] == 0 || i == index) trie->nodes[i]->prune = 1;
//                 else {
//                     if (occs[i] == -1){
//                         // occurrences unbound, do not modify array
//                         count += rec_prune(trie->nodes[i], s, eval, occs, depth + 1);
//                     } else if (occs[i] < -1){
//                         // decrease minimum occurrences
//                         ++(occs[i]);
//                         count += rec_prune(trie->nodes[i], s, eval, occs, depth + 1);
//                         --(occs[i]);
//                     } else {
//                         // decrease exact occurrences
//                         --(occs[i]);
//                         count += rec_prune(trie->nodes[i], s, eval, occs, depth + 1);
//                         ++(occs[i]);
//                     }
//                 }
//             }
//         }
//         return count;
//     }
// }

// int prune_trie(node_t *trie, char *s, char *eval){
//     int occs[CHARSET];

//     calculate_occs(s, eval, occs);
//     return rec_prune(trie, s, eval, occs, 0);
// }


static void rec_print(node_t *trie, char *word, int depth){
    char *sfx;
    int i, size;

    if (trie->nodes == NULL) {
        fputs(word, stdout); //omit newline
        sfx = (trie->status) + 1;
        if (*sfx != '\0') puts(sfx);
        else putchar('\n');
        return;
    }

    size = *(trie->status)>>1;
    for (i = 0; i  <= size; ++i){
        if (trie->nodes[i] != NULL && *(trie->nodes[i]->status) % 2 == 0){
            word[depth] = unmap_charset(i);
            rec_print(trie->nodes[i], word, depth + 1);
        }
    }
    word[depth] = '\0';
}

void print_trie(node_t *trie, int wordsize){
    char *word = (char *) calloc(wordsize + 1, sizeof(char));

    rec_print(trie, word, 0);
}
