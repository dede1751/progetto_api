#include "structures.h"
#include "game.h"

/*
    - POSSIBLE OPTIMIZATIONS: 
  * init list after the first guess
  * hash table for faster search
  * depending on wordsize adjust reqs->occ
*/

int main(){
    dict_ptr word_dict = generate_dict();
    char *buff;
    int wordsize, restart;
    FILE *input = fopen("/Users/andreasgobbi/Documents/uni_works/progetto_api/dum_e/gen_large.txt", "r");
    FILE *output = fopen("/Users/andreasgobbi/Documents/uni_works/progetto_api/dum_e/dump.txt", "w");

    // save initial wordlist on the tree
    safe_scanf(&wordsize, input);
  
    buff = (char *) malloc((wordsize + 1) * sizeof(char));
    safe_fgets(buff, wordsize, input);
    while (buff[0] != '+'){
        if (insert(word_dict, buff) == NULL) exit(EXIT_FAILURE);

        buff = (char *) malloc((wordsize + 1) * sizeof(char));
        fgetc(input); //trailing \n
        safe_fgets(buff, wordsize, input);
    }
    if (wordsize <= 14) while (fgetc(input) != '\n'); // dumps leftover

    do { // main game loop
        restart = new_game(word_dict, wordsize, input, output);
    } while(restart == 1);

    free_dict(word_dict);
    exit(EXIT_SUCCESS);
}
