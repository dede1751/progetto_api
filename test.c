#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CHARSET 64
#define PRUNE 1
#define NO_PRUNE 0

typedef struct trie {
    struct trie *next;
    struct trie *branch;
    char *status;
} trie_t;


// needed for errorless compilation
void safe_fgets(char *, int);
void safe_scanf(int *);

trie_t *generate_branch(char);

trie_t *get_child(trie_t *, char);
trie_t *add_child(trie_t *, char *);
trie_t *insert(trie_t *, char *);
trie_t *insert_leaf(trie_t *, char *, char);
void split_leaves(trie_t *, char *);

trie_t *initial_read(trie_t *, int);

int main(){
    trie_t *trie = NULL;
    int wordsize;

    safe_scanf(&wordsize);

    trie = initial_read(trie, wordsize);

    exit(EXIT_SUCCESS);
}

void safe_fgets(char *s, int size){
    if (fgets(s, size + 1, stdin) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(int *x){
    if (scanf("%d\n", x) == 0) exit(EXIT_FAILURE);
}


trie_t *generate_branch(char c){
    trie_t *new = (trie_t *)malloc(sizeof(trie_t));
    char *status = (char *)malloc(2*sizeof(char));

    new->next = NULL;
    new->branch = NULL;
    status[0] = NO_PRUNE;
    status[1] = c;
    new->status = status;

    return new;
}

trie_t *get_child(trie_t *trie, char tgt){
    int c;

    for (; trie != NULL; trie = trie->next){
        c = (trie->status)[1];
        if (c > tgt) return NULL;
        else if (c == tgt) return trie;
    }
    return NULL;
}

trie_t *add_child(trie_t *trie, char *status){
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

trie_t *insert_leaf(trie_t *trie, char *word, char p){
    int len = strlen(word);
    char *status = (char *)malloc((len + 2)*sizeof(char));

    // add prune and copy whole word, so status[1] is word[0] hence the index
    status[0] = p;
    memcpy(status + sizeof(char), word, (len + 1)*sizeof(char));

    return add_child(trie, status);
}

void split_leaves(trie_t *trie, char *word){
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
        else prev_branch->branch = insert_leaf(trie, word, NO_PRUNE);      // root won't change
    } else split_leaves(child, word + sizeof(char)); // doesn't affect root
    return root;
}

trie_t *initial_read(trie_t *trie, int wordsize){
    char *buff = (char *) malloc((wordsize + 1) * sizeof(char));

    safe_fgets(buff, wordsize);
    while (buff[0] != '+'){
        trie = insert(trie, buff);

        getchar(); //trailing \n
        safe_fgets(buff, wordsize);
    }

    if (buff[1] == 'n'){    // +nuova_partita
        if (wordsize <= 14) while (getchar() != '\n');
    } else {                // +inserisci_inizio
        if (wordsize <= 17) while (getchar() != '\n');

        safe_fgets(buff, wordsize);
        while (buff[0] != '+'){
            trie = insert(trie, buff);

            getchar(); //trailing \n
            safe_fgets(buff, wordsize);
        }
        // dump remaining of +inserisci_fine, dump +nuova_partita
        if (wordsize <= 15) while (getchar() != '\n');
        while(getchar() != '\n');
    }
    free(buff);

    return trie;
}
