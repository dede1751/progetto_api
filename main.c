#include "trie.h"
#include "game.h"
#include <time.h>


int main(){
    trie_t *trie = NULL;
    req_t *reqs = (req_t *) malloc(sizeof(req_t));
    int wordsize;

    // test3
    // char example_ref[6]    = "CACAC";
    // char example_guess1[6] = "GbddC";
    // char example_guess2[6] = "AAbld";
    // char example_guess3[6] = "CldNC";
    // char example_guess4[6] = "CAGml";
    // char example_guess5[6] = "AACCd";

    // test1
    // char example_ref[6]    = "2rj9R";
    // char example_guess1[6] = "2PFdd";
    // char example_guess2[6] = "2rz9R";
    // char example_guess3[6] = "2rq9R";

    // upto18
    char example_ref[6]    = "LH29F";
    char example_guess1[6] = "1GhpD";
    char example_guess2[6] = "0Iflt";


    safe_scanf(&wordsize);
    generate_req(wordsize, reqs);

    trie = initial_read(trie, wordsize);

    handle_simple_guess(trie, wordsize, example_ref, example_guess1, reqs);
    // handle_simple_guess(trie, wordsize, example_ref, example_guess2, reqs);
    // handle_simple_guess(trie, wordsize, example_ref, example_guess3, reqs);
    // handle_simple_guess(trie, wordsize, example_ref, example_guess4, reqs);

    clear_trie(trie);

    handle_full_guess(trie, wordsize, example_ref, example_guess2, reqs);

    print_trie(trie, wordsize);

    exit(EXIT_SUCCESS);
}
