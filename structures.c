#include "structures.h"

static void left_rotate(dict_ptr, node_ptr);
static void right_rotate(dict_ptr, node_ptr);
static void insert_fixup(dict_ptr, node_ptr);

static void free_tree(node_ptr, node_ptr);
static void empty_blocks(node_ptr);


dict_ptr generate_dict(){
    dict_ptr dict = (dict_ptr)malloc(sizeof(dict_t));
    node_ptr leaf = (node_ptr)malloc(sizeof(node_t));

    leaf->color = 0;
    leaf->word = NULL;
    leaf->left = NULL;
    leaf->right = NULL;
    leaf->parent = NULL;
    leaf->next = NULL;

    dict->NIL = leaf;
    dict->root = leaf;
    dict->head = NULL;
    dict->len = 0;

    return dict;
}

static void left_rotate(dict_ptr D, node_ptr x){
    node_ptr y = x->right;

    x->right = y->left;
    if (y->left != D->NIL) y->left->parent = x;

    y->parent = x->parent;
    if (x->parent == D->NIL) D->root = y;
    else if (x->parent->left == x) x->parent->left = y;
    else x->parent->right = y;

    y->left = x;
    x->parent = y;
}
static void right_rotate(dict_ptr D, node_ptr x){
    node_ptr y = x->left;

    x->left = y->right;
    if (y->right != D->NIL) y->right->parent = x;

    y->parent = x->parent;
    if (x->parent == D->NIL) D->root = y;
    else if (x->parent->left == x) x->parent->left = y;
    else x->parent->right = y;

    y->right = x;
    x->parent = y;
}


static void insert_fixup(dict_ptr D, node_ptr curr){
    node_ptr prev, side;

    if (curr == D->root) curr->color = 0;
    else {
        prev = curr->parent;
        if (prev->color == 1){ // black parent means no balancing needed
            if (prev == prev->parent->left){ // prev is left child
                side = prev->parent->right;

                if (side->color == 1){ // prev's right brother is red, go up the tree
                    prev->color = 0;
                    side->color = 0;
                    prev->parent->color = 1;
                    insert_fixup(D, prev->parent);

                } else { // prev's right brother is black
                    if (curr == prev->right){
                        curr = prev;
                        left_rotate(D, curr); // if curr is right child, reduce to left
                        prev = prev->parent;
                    }// curr is left child
                    prev->color = 0;
                    prev->parent->color = 1;
                    right_rotate(D, prev->parent);
                }

            } else if (prev == prev->parent->right){ // same steps, reverse left/right
                side = prev->parent->left;

                if (side->color == 1){
                    prev->color = 0;
                    side->color = 0;
                    prev->parent->color = 1;
                    insert_fixup(D, prev->parent);

                } else {
                    if (curr == prev->left){
                        curr = prev;
                        right_rotate(D, curr);
                        prev = curr->parent;
                    }
                    prev->color = 0;
                    prev->parent->color = 1;
                    left_rotate(D, prev->parent);
                }
            }
        }
    }
};

node_ptr insert(dict_ptr T, char *s){
    node_ptr curr, prev, new;
    int i;

    curr = T->root;
    prev = T->NIL;
    while(curr != T->NIL){
        prev = curr;
        i = strcmp(s, curr->word);
        if (i > 0) curr = curr->right;
        else curr = curr->left;
    }

    new = (node_ptr)malloc(sizeof(node_t));
    new->word = s;
    new->parent = prev;
    new->left = T->NIL;
    new->right = T->NIL;
    new->next = NULL;
    new->color = 1;

    if (prev == T->NIL) T->root = new;
    else if (i > 0) prev->right = new;
    else prev->left = new;

    insert_fixup(T, new);
    return new;
}


uint8_t search(dict_ptr D, char* s){
    node_ptr curr;
    int i;

    curr = D->root;
    while(curr != D->NIL){
        i = strcmp(s, curr->word);
        if (i == 0) return 1;
        else if (i > 0) curr = curr->right;
        else curr = curr->left;
    }
    return 0;
}


void ordered_insert(dict_ptr D, node_ptr x){
    node_ptr curr, prev;

    curr = D->head;
    prev = NULL;
    while(curr != NULL && strcmp(x->word, curr->word) > 0){
        prev = curr;
        curr = curr->next;
    }

    x->next = curr;
    if (D->head == NULL) D->head = x;
    else if (prev != NULL) prev->next = x;
    
    ++(D->len);
}

void print_list(dict_ptr T, FILE *output){
    node_ptr curr;

    for (curr = T->head; curr != NULL; curr = curr->next){
        fputs(curr->word, output);
        fputs("\n", output);
    }
}

// free pointers
static void free_tree(node_ptr curr, node_ptr NIL){ //postorder traversal
    if (curr->left != NIL) free_tree(curr->left, NIL);
    if (curr->right != NIL) free_tree(curr->right, NIL);
    free(curr);
}
static void empty_blocks(node_ptr curr){ //linear traversal, resets .next fields
    if (curr->next != NULL) {
        empty_blocks(curr->next);
        curr->next = NULL;
    }
}

void reset_list(dict_ptr T){
    if (T->head != NULL){
        empty_blocks(T->head);
        T->head = NULL;
        T->len = 0;
    }
}

void free_dict(dict_ptr D){
    if (D->root != NULL) free_tree(D->root, D->NIL);
    free(D->NIL);
    free(D);
}
