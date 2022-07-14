#include "structures.h"

static node_t *generate_node();
static int rec_prune(node_t *, char *, char *, int *, int);
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

// initializes everything to NULL and 0
static node_t *generate_node(){
    return (node_t *) calloc(1, sizeof(node_t));
}

// initializes full children array (all set to NULL)
node_t *generate_trie(){
    node_t *n = generate_node();

    n->nodes = (node_t **)calloc(CHARSET, sizeof(node_t *));
    n->prune = (CHARSET - 1)<<1;
    return n;
}


/*
 *          INSERTION:
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *
 */
static void init_children(node_t *trie, int size){
    trie->nodes = (node_t **)calloc(size + 1, sizeof(node_t *));
    trie->suffix = NULL;
    trie->prune = size<<1; // set prune to 0 and save size
}

static node_t **realloc_null(node_t **nodes, size_t old_size, size_t new_size){
    node_t **new = realloc(nodes, new_size);

    memset(((char *)new) + old_size, 0, new_size - old_size);
    return new;
}

static void resize_children(node_t *trie, int size){
    int tmp = (trie->prune)>>1;

    if (tmp < size){
        // realloc and set new memory to 0
        trie->nodes = realloc_null(
            trie->nodes,
            (tmp  + 1) * sizeof(node_t *),
            (size + 1) * sizeof(node_t *)
        );

        trie->prune &= 0x01;     // reset all but prune bit
        trie->prune += size<<1;  // set new size
    }
}

static void insert_leaf(node_t *trie, char *word_sfx, int p){
    node_t *new = generate_node();
    int len = strlen(word_sfx);

    // copy suffix
    if (len > 1){
        new->suffix = (char *) malloc(len * sizeof(char));
        memcpy(new->suffix, word_sfx + 1, len * sizeof(char));
    }
    new->prune = p;
    trie->nodes[map_charset(*word_sfx)] = new;
}

static void split_leaves(node_t *trie, char *word_sfx){
    char *tmp, *sfx = trie->suffix;
    uint8_t p = trie->prune;
    int index;

    // continue making internal nodes until sfx and word_sfx stop matching
    for (tmp = sfx; *sfx == *word_sfx; ++sfx, ++word_sfx){
        index = map_charset(*sfx);
        init_children(trie, index);
        trie->nodes[index] = generate_node();
        trie = trie->nodes[index];
    }

    // init children array to the max between two indices
    init_children(trie, MAX(map_charset(*sfx), map_charset(*word_sfx)));

    insert_leaf(trie, sfx, p);
    insert_leaf(trie, word_sfx, 0);
    free(tmp);
}


void insert(node_t *trie, char *word){
    int index = map_charset(*word);

    // navigate through word to first NULL node
    while (
        trie->nodes != NULL         &&  // met a leaf node
        ((trie->prune)>>1) >= index &&  // resize children array, then add leaf
        trie->nodes[index] != NULL      // add a new leaf
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


static int rec_prune(node_t *trie, char *s, char *eval, int *occs, int depth){
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

int prune_trie(node_t *trie, char *s, char *eval){
    int occs[CHARSET];

    calculate_occs(s, eval, occs);
    return rec_prune(trie, s, eval, occs, 0);
}


static void rec_print(node_t *trie, char *word, int depth){
    int i, size;

    if (trie->nodes == NULL) {
        fputs(word, stdout); //omit newline
        if (trie->suffix != NULL) puts(trie->suffix);
        else putchar('\n');
        return;
    }

    size = (trie->prune)>>1;
    for (i = 0; i  <= size; ++i){
        if (trie->nodes[i] != NULL && trie->nodes[i]->prune % 2 == 0){
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
