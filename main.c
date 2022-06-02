#include "structures.h"
#include "game.h"

/*
    - POSSIBLE OPTIMIZATIONS: 
  * init list after the first guess
  * hash table for faster search
  * depending on wordsize adjust reqs->occ
*/

double insert_time = 0.0;
double insert_ord_time = 0.0;
double search_time = 0.0;
double fill_time = 0.0;
double filter_time = 0.0;
double total_time = 0.0;
double print_time = 0.0;

int main(){
    dict_ptr word_dict = generate_dict();
    char *buff;
    int wordsize, restart;
    FILE *input = fopen("/Users/andreasgobbi/Documents/uni_works/progetto_api/dum_e/gen_mid.txt", "r");
    FILE *output = fopen("/Users/andreasgobbi/Documents/uni_works/progetto_api/dum_e/dump.txt", "w");
    int i;
    clock_t start = clock();

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

    i = 1;
    do { // main game loop
        restart = new_game(word_dict, wordsize, input, output);
        printf("\nGAME %d FINISHED", i);
        ++i;
    } while(restart == 1);

    free_dict(word_dict);

    clock_t stop = clock();
    total_time = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    printf("\n\nTOTAL TIME:%lf\nINSERT: %lf\nORDERED INS: %lf\nSEARCH: %lf\nFILL: %lf\nFILTER: %lf\nPRINT: %lf\n",
    total_time, insert_time, insert_ord_time, search_time, fill_time, filter_time, print_time);
    //scanf("c");
    exit(EXIT_SUCCESS);
}
