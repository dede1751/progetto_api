#include "trie.h"

static trie_t *generate_branch(char);

static trie_t *get_child(trie_t *, char);
static trie_t *add_child(trie_t *, char *);
static trie_t *insert_leaf(trie_t *, char *, char);
static void split_leaves(trie_t *, char *);

static void rec_print(trie_t *, char *, int);



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

static trie_t *get_child(trie_t *trie, char tgt){
    int c;

    for (; trie != NULL; trie = trie->next){
        c = (trie->status)[1];
        if (c > tgt) return NULL;
        else if (c == tgt) return trie;
    }
    return NULL;
}

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

static trie_t *insert_leaf(trie_t *trie, char *word, char p){
    int len = strlen(word);
    char *status = (char *)malloc((len + 2)*sizeof(char));

    // add prune and copy whole word, so status[1] is word[0] hence the index
    status[0] = p;
    memcpy(status + sizeof(char), word, (len + 1)*sizeof(char));

    return add_child(trie, status);
}

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
        else prev_branch->branch = insert_leaf(trie, word, NO_PRUNE);         // root won't change
    } else split_leaves(child, word + sizeof(char)); // doesn't affect root
    return root;
}


int search(trie_t *root, char *word){
    root = get_child(root, word[0]);

    while (root != NULL && root->branch != NULL){
        word += sizeof(char);
        root = get_child(root, word[0]);
    }

    if (root == NULL) return 0;
    else {
        if (strcmp((root->status) + sizeof(char), word) == 0) return 1;
        else return 0;
    }
}


static void rec_print(trie_t *trie, char *word, int depth){

    // pruned
    if ((trie->status)[0] == 1) return;

    while (trie != NULL){
        if (trie->branch == NULL) {
            fputs(word, stdout); //omit newline
            puts((trie->status) + sizeof(char)); // always at least one letter
        } else {
            word[depth] = (trie->status)[1];
            rec_print(trie->branch, word, depth + 1);
            word[depth] = '\0';
        }
        trie = trie->next;
    }
}

void print_trie(trie_t *trie, int wordsize){
    char *word = (char *) calloc(wordsize + 1, sizeof(char));

    rec_print(trie, word, 0);
    free(word);
}

int count_trie(trie_t *trie){
    int count = 0;

    if (trie->next != NULL) count += count_trie(trie->next);
    if (trie->branch != NULL) count += count_trie(trie->branch);
    else return 1 + count;

    return count;
}


void free_trie(trie_t *trie){

    if (trie->next != NULL) free_trie(trie->next);
    if (trie->branch != NULL) free_trie(trie->branch);

    free(trie->status);
    free(trie);
}