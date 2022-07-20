#include "trie.h"
#include "game.h"


int main(){
    trie_t *trie = NULL;
    int wordsize;

    safe_scanf(&wordsize);

    trie = initial_read(trie, wordsize);

    do {
        trie = new_game(trie, wordsize);
    } while (trie != NULL);

    exit(EXIT_SUCCESS);
}
