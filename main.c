#include "structures.h"
#include "game.h"


int main(){
    node_t *trie = initialize_trie();
    int wordsize;
    // char example_ref[6]    = "LH29F";
    // char example_guess1[6] = "1GhpD";
    // char example_guess2[6] = "0Iflt";
    // char example_guess3[6] = "CldNC";

    // char *eval;
    // int count;

    safe_scanf(&wordsize);

    initial_read(trie, wordsize);
    //print_trie(trie, wordsize);

    // eval = calculate_eval(example_ref, example_guess1, 5);
    // puts(eval);
    // count = prune_trie(trie, example_guess1, eval);
    // printf("%d", count);

    // eval = calculate_eval(example_ref, example_guess2, 5);
    // puts(eval);
    // count = prune_trie(trie, example_guess2, eval);
    // printf("%d", count);

    // eval = calculate_eval(example_ref, example_guess3, 5);
    // puts(eval);
    // count = prune_trie(dict, example_guess3, eval);
    // printf("%d", count);
    //print_trie(dict);

    exit(EXIT_SUCCESS);
}
