#include "structures.h"

static char unmap_charset(int);

static int rec_prune(node_ptr, char *, char *, int *, int);
static void rec_print(node_ptr, char *, int);


uint8_t map_charset(char c){
    if (c == '-') return 0;
    else if (c >= '0' && c <= '9') return (c - 47);
    else if (c >= 'A' && c <= 'Z') return (c - 54);
    else if (c >= 'a' && c <= 'z') return (c - 59);
    else return 37; // maps '_'
}
static char unmap_charset(int x){
    if (x == 0) return '-';
    else if (x >= 1 && x <= 10) return (x + 47);
    else if (x >= 10 && x <= 36) return (x + 54);
    else if (x >= 38 && x <= 63) return (x + 59);
    else return '_';
}

// initializes children to NULL and prune to 0
node_ptr generate_node(){
    node_ptr n = (node_ptr) malloc(sizeof(node_t));

    n->nodes = (node_ptr *) calloc(CHARSET, sizeof(node_ptr));
    n->prune = 0;

    return n;
}


void insert(node_ptr trie, int wordsize, char *word){
    node_ptr new;
    int i, index;

    // iterate until before the last letter
    for (i = 0; i < wordsize - 1; ++i){
        index = map_charset(word[i]);

        if (trie->nodes[index] == NULL){
            // new node
            new = generate_node();
            trie->nodes[index] = new;
            trie = new;
        } else {
            // old node
            trie = trie->nodes[index];
        }
    }

    // insert final node with new->nodes = NULL
    index = map_charset(word[i]);
    trie->nodes[index] = (node_ptr)calloc(1, sizeof(node_t));
}


uint8_t search(node_ptr trie, int wordsize, char *word){
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
static void calculate_occs(char *s, char *eval, int *occs){
    int i, index;

    // set all occs to -1 (unbound)
    for (i = 0; i < CHARSET; ++i) occs[i] = -1;

    for (i = 0; s[i] != '\0'; ++i){
        index = map_charset(s[i]);

        // if '|' increase the minimum amount
        if (eval[i] == '|' && occs[index] < 0) --(occs[index]);
        // if '/' set the exact amount, -1 --> 0  -3 --> 2
        else if (eval[i] == '/' && occs[index] < 0) {
            occs[index] = -(occs[index]) - 1;
        }
    }
}


static int rec_prune(node_ptr trie, char *s, char *eval, int *occs, int depth){
    int i, index, count;

    // branch has been pruned, ignore it
    if (trie->prune == 1) return 0;

    // reached a leaf
    if (trie->nodes == NULL){
        for (i = 0; s[i] != '\0'; ++i){
            index = map_charset(s[i]);
            if (occs[index] != 0 && occs[index] != -1){
                // not meeting exact/minimum occurrences of some letters
                trie->prune = 1;
                return 0;
            }
        }
        return 1;
    }

    index = map_charset(s[depth]);
    if (eval[depth] == '+'){
        // prune all except nodes[index]
        for (i = 0; i < CHARSET; ++i){
            if (i != index && trie->nodes[i] != NULL){
                trie->nodes[i]->prune = 1;
            }
        }
        // only call on correct letter, no need to adjust occurrences
        if (trie->nodes[index] != NULL){
            return rec_prune(trie->nodes[index], s, eval, occs, depth + 1);
        } else return 0;

    } else {
        // prune only nodes[index]
        for (i = 0, count = 0; i < CHARSET; ++i) {
            if (trie->nodes[i] != NULL){
                //prune chars that do not appear or are misplaced
                if (occs[i] == 0 || i == index) trie->nodes[i]->prune = 1;
                else {
                    if (occs[i] == -1){
                        // occurrences unbound, do not modify array
                        count += rec_prune(trie->nodes[i], s, eval, occs, depth + 1);
                    } else if (occs[i] < -1){
                        // decrease minimum occurrences
                        ++(occs[i]);
                        count += rec_prune(trie->nodes[i], s, eval, occs, depth + 1);
                        --(occs[i]);
                    } else {
                        // decrease exact occurrences
                        --(occs[i]);
                        count += rec_prune(trie->nodes[i], s, eval, occs, depth + 1);
                        ++(occs[i]);
                    }
                }
            }
        }
        return count;
    }
}

int prune_trie(node_ptr trie, char *s, char *eval){
    int occs[CHARSET];

    calculate_occs(s, eval, occs);
    return rec_prune(trie, s, eval, occs, 0);
}


static void rec_print(node_ptr trie, char *word, int depth){
    int i;

    if (trie->nodes == NULL) {
        puts(word);
        return;
    }

    for (i = 0; i < CHARSET; ++i){
        if (trie->nodes[i] != NULL && trie->nodes[i]->prune == 0){
            word[depth] = unmap_charset(i);
            rec_print(trie->nodes[i], word, depth + 1);
        }
    }
}

void print_trie(node_ptr trie, int wordsize){
    char *word = (char *) malloc((wordsize + 1) * sizeof(char));

    word[wordsize] = '\0';
    rec_print(trie, word, 0);
}
