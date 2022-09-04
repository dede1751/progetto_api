#include "trie.h"
#include "game.h"


int main(){
    trie_t *trie = NULL;
    uint8_t wordsize;

    safe_scanf(&wordsize);

    trie = initial_read(trie, wordsize);

    while(1) trie = new_game(trie, wordsize);
}
