#include "game.h"

static uint8_t map_charset(char);
static char *calculate_eval(char *, char *, int);

static void generate_req(req_ptr, int);
static void calculate_req(char *, char *, req_ptr);
static uint8_t check_req(char *, req_ptr);
static void filter_req(dict_ptr, req_ptr);
static void free_req(req_ptr);

static void check_guess(dict_ptr, char *, char *, int, req_ptr, FILE *);
static void handle_insert(dict_ptr, int, req_ptr, FILE *);


void safe_fgets(char *s, int size, FILE *fp){
    if (fgets(s, size + 1, fp) == NULL) exit(EXIT_FAILURE);
}
void safe_scanf(int *x, FILE *fp){
    if (fscanf(fp, "%d\n", x) == 0) exit(EXIT_FAILURE);
}

// map the scattered charset to contiguous 0-63 interval
static uint8_t map_charset(char c){
    if (c == '-') return 0;
    else if (c >= '0' && c <= '9') return (c - 47);
    else if (c >= 'A' && c <= 'Z') return (c - 53);
    else if (c >= 'a' && c <= 'z') return (c - 59);
    else return 37; // maps '_'
}


static char *calculate_eval(char *ref, char *s, int wordsize){
    char *eval = (char *)malloc((wordsize + 1) * sizeof(char));
    uint8_t counts[CHARSET] = {0}; // possible of but gambling for memory
    int i;
    
    for(i = 0; i < wordsize; ++i) eval[i] = '-';
    eval[wordsize] = '\0';

    // count char occurrences in ref
    for(i = 0; i < wordsize; ++i) ++(counts[map_charset(ref[i])]);

    // handle perfect matches and exclusions
    for (i = 0; i < wordsize; ++i){
        if (ref[i] == s[i]) {
            --(counts[map_charset(ref[i])]);
            eval[i] = '+';
        } else if (counts[map_charset(s[i])] == 0) eval[i] = '/';    
    }

    // handle imperfect matches
    for (i = 0; i < wordsize; ++i){
        if (eval[i] == '-'){
            if (counts[map_charset(s[i])] > 0){
                eval[i] = '|';
                --(counts[map_charset(s[i])]);
            } else eval[i] = '/';
        }
    }
    return eval;
}


static void generate_req(req_ptr reqs, int wordsize){
    char *matches;
    uint8_t *p;
    int i, j;

    matches = (char *)malloc((wordsize + 1) * sizeof(char));
    for (i = 0; i < wordsize; ++i) matches[i] = '*';
    matches[wordsize] = '\0';
    reqs->match = matches;

    for (i = 0; i < CHARSET; ++i) {
        p = (uint8_t *)malloc((wordsize) * sizeof(uint8_t));
        for (j = 0; j < wordsize; ++j) p[j] = 1;

        reqs->pos[i] = p;        
        reqs->occ[i] = -1;
    }
}


static void calculate_req(char *s, char *eval, req_ptr reqs){
    int8_t counts[CHARSET]; // treated the same as reqs->occ
    char c;
    int i, index;

    for (i = 0; i < CHARSET; ++i) counts[i] = -1;

    for (i = 0; s[i] != '\0'; ++i){
        c = eval[i];
        index = map_charset(s[i]);

        if (c == '+'){ // when matching, raise bounds
            reqs->match[i] = s[i];
            if (counts[index] < 0) --(counts[index]);
            else ++(counts[index]);
        } else if (c == '|'){ // c is somewhere else
            if (counts[index] < 0) --(counts[index]);
            reqs->pos[index][i] = 0;
        } else if (c == '/'){ // c cannot be placed anywhere
            if (counts[index] < 0) {
                counts[index] = -(counts[index]) - 1;
                reqs->pos[index][i] = 0;
            }
        }
    }

    // evaluate compatibility of new bounds with previous ones
    for (i = 0; i < CHARSET; ++i){
        if (reqs->occ[i] < 0 &&
            (counts[i] >= 0 || counts[i] < reqs->occ[i])
        ) reqs->occ[i] = counts[i];
    }
}


static uint8_t check_req(char *s, req_ptr reqs){
    int16_t counts[CHARSET] = {0};
    int i, index, occs;

    for (i = 0; s[i] != '\0'; ++i){
        index = map_charset(s[i]);
        if ((reqs->match[i] != '*' && s[i] != reqs->match[i]) || // not the matched letter
            reqs->pos[index][i] == 0 || // position is not correct for the letter
            reqs->occ[index] == 0 // letter does not appear in ref
        ) return 0;
        ++(counts[index]);
    }

    for (i = 0; i < CHARSET; ++i){
        occs = reqs->occ[i];
        if ((occs >  0  && counts[i] !=  occs) ||    // not exact amount
            (occs < -1  && counts[i]  < (-occs - 1)) // less than minimum amount
        ) return 0;
    }

    return 1;
}


static void filter_req(dict_ptr D, req_ptr reqs){
    node_ptr curr, prev, temp;

    curr = D->head;
    prev = NULL;
    while (curr != NULL){
        if (check_req(curr->word, reqs) == 0){ // doesn't match requirements
            if (prev != NULL) prev->next = curr->next;
            else D->head = curr->next;
            temp = curr;
            curr = curr->next;

            temp->next = NULL;
            --(D->len);
        } else{
            prev = curr;
            curr = curr->next;
        }
    }
}

static void free_req(req_ptr reqs){
    int i;
    
    free(reqs->match);
    for(i = 0; i < CHARSET; ++i) free(reqs->pos[i]);
    free(reqs);
}

static void check_guess(dict_ptr D, char *ref, char *s, int wordsize, req_ptr reqs, FILE *output){
    char *eval;

    // checking guesses is O(l*m), but l is rapidly decreasing
    eval = calculate_eval(ref, s, wordsize); // O(2m + 64)   m = wordsize
    calculate_req(s, eval, reqs); // O(m + 128)
    filter_req(D, reqs);          // O(l * (m + 64)) l = L->len

    fputs(eval, output);
    fputs("\n",output);
    fprintf(output, "%d\n", D->len);
    free(eval);
}


static void handle_insert(dict_ptr D, int wordsize, req_ptr reqs, FILE *input){
    node_ptr new;
    char *buff = (char *)malloc((wordsize + 1) * sizeof(char));

    safe_fgets(buff, wordsize, input);
    while(buff[0] != '+'){ // +inserisci_fine
        new = insert(D, buff);
        if (reqs != NULL && check_req(buff, reqs) == 1) ordered_insert(D, new);

        fgetc(input);
        buff = (char *)malloc((wordsize + 1) * sizeof(char));
        safe_fgets(buff, wordsize, input);
    }
    if (wordsize <= 15) while (fgetc(input) != '\n');
    free(buff);
}


uint8_t new_game(dict_ptr D, int wordsize, FILE *input, FILE *output){
    req_ptr requirements = (req_ptr)malloc(sizeof(req_t));
    char *buff = (char *)malloc((wordsize + 1) * sizeof(char));
    char *ref = (char *)malloc((wordsize + 1) * sizeof(char));
    int guesses;

    fill_list(D, D->root); // fill list in order
    generate_req(requirements, wordsize);  // instantiate requirements struct

    safe_fgets(ref, wordsize, input);
    fgetc(input);
    safe_scanf(&guesses, input); // grab reference word and number of guesses

    while(guesses > 0){
        safe_fgets(buff, wordsize, input);
    
        if(buff[0] == '+'){
            if (buff[1] == 's'){ // +stampa_filtrate
                if (wordsize <= 16) while (fgetc(input) != '\n');
                print_list(D, output);
            } else if (buff[1] == 'i'){ // +inserisci_inizio
                if (wordsize <= 17) while (fgetc(input) != '\n');
                handle_insert(D, wordsize, requirements, input);
            }

        } else{
            fgetc(input); // trailing newline for words
            if (search(D, buff) == 0) fputs("not_exists\n", output); // string not in tree
            else if (strcmp(ref, buff) == 0) { // guessed correctly
                fputs("ok\n", output);
                break;
            } else {
                check_guess(D, ref, buff, wordsize, requirements, output);
                --guesses;
            }
        }
    }
    if (guesses == 0) fputs("ko\n", output);

    // garbage collection
    reset_list(D);
    free_req(requirements);
    free(buff);
    free(ref);

    if (fgetc(input) == EOF) return 0;
    else {
        buff = (char *)malloc(18*sizeof(char));
        safe_fgets(buff, 17, input);
        if (buff[0] == 'i'){
            handle_insert(D, wordsize, NULL, input); // only add words to tree
            safe_fgets(buff, 18, input);
        }
        free(buff);
    } return 1;
}
