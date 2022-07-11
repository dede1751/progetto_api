#include "structures.h"
#include "game.h"


int main(){
    dict_ptr word_dict = generate_dict();
    int wordsize, restart;
    FILE *input = fopen("/home/andrea/Documents/progetto_api/dum_e/upto18.txt", "r");
    FILE *output = fopen("/home/andrea/Documents/progetto_api/dum_e/dump.txt", "w");

    // save initial wordlist on the tree
    safe_scanf(&wordsize, input);
    initial_read(word_dict, wordsize, input);

    do { // main game loop
        restart = new_game(word_dict, wordsize, input, output);
    } while(restart == 1);

    free_dict(word_dict);
    exit(EXIT_SUCCESS);
}
