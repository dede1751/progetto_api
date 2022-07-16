#include "trie.h"
#include "game.h"
#include <time.h>


int main(){
    trie_t *trie = NULL;
    int wordsize;
    char example_ref[6]    = "LH29F";
    char example_guess1[6] = "1GhpD";
    char example_guess2[6] = "0Iflt";
    // char example_guess3[6] = "CldNC";

    char *eval;
    int count;

    safe_scanf(&wordsize);
    eval = (char *)calloc(wordsize + 1, sizeof(char));

    trie = initial_read(trie, wordsize);
    count = count_trie(trie);
    // print_trie(trie, wordsize);

    eval = calculate_eval(example_ref, example_guess1, 5, eval);
    puts(eval);
    count = prune_trie(trie, example_guess1, eval);
    printf("%d", count);

    eval = calculate_eval(example_ref, example_guess2, 5, eval);
    puts(eval);
    count = prune_trie(trie, example_guess2, eval);
    printf("%d", count);

    // eval = calculate_eval(example_ref, example_guess3, 5);
    // puts(eval);
    // count = prune_trie(dict, example_guess3, eval);
    // printf("%d", count);
    //print_trie(dict);
    
    // free_trie(trie);

    exit(EXIT_SUCCESS);
}
