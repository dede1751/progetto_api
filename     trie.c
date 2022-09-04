#include "trie.h"

static trie_t *generate_branch(char);

static trie_t *get_child(trie_t *, char);
static trie_t *add_child(trie_t *, char *);
static trie_t *insert_leaf(trie_t *, char *, char);
static void split_leaves(trie_t *, char *);

static void print(trie_t *, char *, uint8_t);


/**
 * @brief Allocates a branch node
 * @param c         letter in the branch
 * @return trie_t*  branch node
 */
static trie_t *generate_branch(char c){
    trie_t *new = (trie_t *)malloc(sizeof(trie_t));
    char *status = (char *)malloc(2*sizeof(char));

    new->next = NULL;
    new->branch = NULL;
    status[0] = NO_PRUNE;
    status[1] = c;
    new->status = status;

    return new;
}

/**
 * @brief Get the node for letter "tgt" at the current height
 * @param trie      trie node (first node of the "level")
 * @param tgt       letter to search for
 * @return trie_t*  trie node for tgt or NULL if it's not found
 */
static trie_t *get_child(trie_t *trie, char tgt){
    char c;

    for (; trie != NULL; trie = trie->next){
        c = (trie->status)[1];
        if (c > tgt) return NULL;       // surpassed tgt, not found
        else if (c == tgt) return trie; // found
    }
    return NULL;
}

/**
 * @brief Allocates new trie node and places it in the correct spot
 * 
 *  Allocates the new node, navigates in the level until the correct ordered
 *  spot is reached and links it to the others. The status string must be
 *  externally supplied.
 * 
 * @param trie      trie node (first node of the "level")
 * @param status    status string of the new node
 * @return trie_t*  first node of the level (can be different from trie)
 */
static trie_t *add_child(trie_t *trie, char *status){
    trie_t *prev = NULL, *curr = trie, *new = (trie_t *)malloc(sizeof(trie_t));
    char tgt = status[1];

    // find correct spot
    while (curr != NULL && ((curr->status)[1] < tgt)){
        prev = curr;
        curr = curr->next;
    }

    // add new trie node, return head of the list
    new->next = curr;
    new->branch = NULL;
    new->status = status;

    if (prev == NULL) return new;
    else {
        prev->next = new;
        return trie;
    }
}

/**
 * @brief Allocates and inserts in the correct place a leaf node for word.
 * @param trie      trie node (first node of the "level")
 * @param word      suffix of the word being inserted
 * @param p         prune value for the new leaf
 * @return trie_t*  first node of the level (can be different from trie)
 */
static trie_t *insert_leaf(trie_t *trie, char *word, char p){
    int len = strlen(word);
    char *status = (char *)malloc((len + 2)*sizeof(char));

    // add prune and copy whole word, so status[1] is word[0] hence the index
    status[0] = p;
    memcpy(status + sizeof(char), word, (len + 1)*sizeof(char));

    return add_child(trie, status);
}

/**
 * @brief Transforms a leaf into a branch to add the new word.
 * 
 *  When traveling down the tree, leaves can be found along our word's "path".
 *  Words like "abcd" and "abef" will collide at "b" if inserted in that order.
 *  In this case, when inserting "abef", we will get "abcd" from get_child().
 *  We substitute the "abcd" node with a branch for "b", while also saving the
 *  prune value for the leaf (this is important, we don't know if the pruning
 *  was done solely on the "b" or due to the rest of the word). We then add
 *  both word and the old leaf's status as leaves in the level below "b", and
 *  the old leaf keeps its prune value.
 * 
 * @param trie      leaf node to split
 * @param word      suffix of the word to insert
 */
static void split_leaves(trie_t *trie, char *word){
    trie_t *tmp_trie = trie;
    char *tmp_sts = trie->status, *sfx = trie->status + 2*sizeof(char);

    // navigate down as long as word and sfx are the same
    for (; *sfx == *word; sfx += sizeof(char), word += sizeof(char)){
        trie->branch = generate_branch(word[0]);
        trie = trie->branch;
    }

    // at some point they must differ, add them as leaves to trie->branch.
    trie->branch = insert_leaf(NULL, word, NO_PRUNE);
    trie->branch = insert_leaf(trie->branch, sfx, tmp_sts[0]);

    // reallocate initial leaf to be unpruned branch (only 2 chars in status)
    tmp_sts[0] = NO_PRUNE;
    tmp_trie->status = realloc(tmp_sts, 2*sizeof(char));
}

/**
 * @brief Inserts string into trie and returns updated trie
 * 
 *  First travels down the trie using get_child() and either reaches a leaf or
 *  does not find an existing path. In the first case it splits the leaf, in the
 *  second it simply adds the remaining suffix of the word as a leaf.
 * 
 * @param root      root of the trie to insert the string in
 * @param word      word to save on the trie
 * @return trie_t*  returns the new root
 */
trie_t *insert(trie_t *root, char *word){
    trie_t *child = get_child(root, word[0]), *trie = root, *prev_branch = NULL;

    // iterate down as long as child is found and it's a branch
    while(child != NULL && child->branch != NULL){
        prev_branch = child;
        trie = child->branch;
    
        word += sizeof(char);
        child = get_child(trie, word[0]);
    }

    if (child == NULL) {
        if (prev_branch == NULL) return insert_leaf(trie, word, NO_PRUNE); // root might change
        else prev_branch->branch = insert_leaf(trie, word, NO_PRUNE);      // root won't change
    } else split_leaves(child, word + sizeof(char)); // doesn't affect root
    return root;
}

/**
 * @brief Searches trie for target string
 * 
 *  Travels down like insert, but either does not find a path or simply checks
 *  the suffix whenever it finds a leaf.
 * 
 * @param root      root of the trie to search the string in
 * @param word      word to search in the trie
 * @return int      1 = found  0 = not found
 */
int search(trie_t *root, char *word){
    root = get_child(root, word[0]);

    // descend down branch until leaf or NULL
    while (root != NULL && root->branch != NULL){
        word += sizeof(char);
        root = get_child(root->branch, word[0]);
    }

    if (root == NULL) return 0;
    else { // check that the suffix matches the rest of the word
        if (strcmp((root->status) + sizeof(char), word) == 0) return 1;
        else return 0;
    }
}

/**
 * @brief Recursively prints the trie (only not pruned nodes)
 *
 *  Iterate through the current "level" of the trie:
 * 
 *      - Base case -->  leaf node, print word+status
 *      - Rec call  -->  add current letter to word, call print one branch down
 * 
 * @param trie      first node of current "level"
 * @param word      prefix of the word to print
 * @param depth     current "level" (also length of word)
 */
static void print(trie_t *trie, char *word, uint8_t depth){

    while (trie != NULL){
        if ((trie->status)[0] == NO_PRUNE){
            if (trie->branch == NULL) {
                fputs(word, stdout); //omit newline
                puts((trie->status) + sizeof(char)); // always at least one letter
            } else {
                word[depth] = (trie->status)[1];
                print(trie->branch, word, depth + 1);
                word[depth] = '\0';
            }
        }
        trie = trie->next;
    }
}

/**
 * @brief Wrapper for print
 * @param trie      root of the trie to print
 * @param wordsize  size of the words in the trie
 */
void print_trie(trie_t *trie, uint8_t wordsize){
    char *word = (char *) calloc(wordsize + 1, sizeof(char));

    print(trie, word, 0);
    free(word);
}

/**
 * @brief Recursively set all trie nodes to NO_PRUNE
 * @param trie      root of the trie to clear
 */
void clear_trie(trie_t *trie){

    if (trie->next != NULL) clear_trie(trie->next);
    if (trie->branch != NULL) clear_trie(trie->branch);

    (trie->status)[0] = NO_PRUNE;
}
