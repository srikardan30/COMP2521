// COMP2521 21T2 Assignment 1
// Dict.c ... implementation of the Dictionary ADT

// Written by Viraj Srikar Danthurty 
// z5359063@ad.unsw.edu.au
// Written in June/July 2021

// Program function:
// This is the .c file which creates and stores words into a dictionary 
// also can display the tree and return the top number of frequent 
// words in the given dictionary 

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "WFreq.h"

typedef struct DictNode *Node;

// This is the struct which will be known as 'Node', which will contain all
// the items needed 
struct DictNode {

    WFreq item;

    Node right;

    Node left;

    int height;   

};

// This is the struct which will be known as 'Dict' which is the wrapper for
// the dictionary 
struct DictRep {  

    Node tree;

    // Will store the number of unique words that are entered into the dictionary
    int nwords;
};
///////////////////////////////////////////////////////////////////////////////
//                           FUNCTION PROTOTYPES                             //
///////////////////////////////////////////////////////////////////////////////

int height (Node d);
Node new_word(char *word);
Node insert_helper(Node d, char *word, Dict rep);
static void view_tree(Node d, int depth);
int DictFind_helper(Node d, char *word);
Node DictFree_nodes(Node d);
Node right_rotate(Node d);
Node left_rotate(Node d);
int balance_factor(Node d);
WFreq TreeToArray(WFreq *wfs, Node d);
static int compareFreq(const void *pa, const void *pb);

///////////////////////////////////////////////////////////////////////////////
//                              FUNCTIONS                                    //
///////////////////////////////////////////////////////////////////////////////

//////////////////////////DictNew and helper functions/////////////////////////
// Creates a new empty Dictionary
Dict DictNew(void) {

    // Allocate memory for making a new dictionary and node 
    Dict dictionary = malloc(sizeof(struct DictRep));
    dictionary->nwords = 0;
    dictionary->tree = NULL;

    return dictionary;   
}

// Creates a new word node in the tree
Node new_word(char *word) {

    // Allocate memory for the node. Doing this also allocates memory for 
    // each data structure within it
    Node node = malloc(sizeof(struct DictNode));

    // Initialise the right, left and height variables 
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    // Copy in the word passed in 
    node->item.word = strndup(word, strlen(word));
    // Set the freq value to 1
    node->item.freq = 1;

    return node;
}

///////////////////////DictFree and helper functions///////////////////////////
// Frees the given Dictionary
void DictFree(Dict d) {
    
    // Pass the node of the tree in to be freed in a helper function
    DictFree_nodes(d->tree);
    // Then delete the dictionary that is passed in 
    free(d);

}

// This is a helper function which will delete all of the nodes contents
// then the node itself 
Node DictFree_nodes(Node d) {

    // Use recursion to delete the nodes and its contents 
    if (d != NULL) {
        if (d->left != NULL) {
            DictFree_nodes(d->left);
        }
        if (d->right != NULL) {
            DictFree_nodes(d->right);
        }
        if (d->item.word != NULL) {
            free(d->item.word);
        }
        // Free the node 
        free(d);
    }
    return NULL;
}

////////////////////////DictInsert and helper functions////////////////////////
// Gets the height of the tree and returns it
int height(Node d) {
    
    if (d == NULL) {
        return -1;
    } else {
        // Get the height of each subtree
        int left_height = height(d->left);
        int right_height = height(d->right);

        if (left_height > right_height) {
            return (left_height + 1);
        } else {
            return (right_height + 1);
        }
    }   
}

// Function to rotate the tree to the right 
Node right_rotate(Node d) {
    
    Node temp1 = d->left;
    Node temp2 = temp1->right;

    // Perform the rotation
    temp1->right = d;
    d->left = temp2;

    return temp1; 
}

// Function to rotate the tree to the left 
Node left_rotate(Node d) {

    Node temp1 = d->right;
    Node temp2 = temp1->left;

    // Perform the rotation
    temp1->left = d;
    d->right = temp2;

    return temp1;
}

// Returns the balance factor of the tree in an int 
int balance_factor(Node d) {

    if (d == NULL) {
        return -1;
    } else {
        return (height(d->left) - height(d->right));
    }
}


// Inserts an occurrence of the given word into the Dictionary
void DictInsert(Dict d, char *word) {

    // Pass the nodes into the helper function where it will insert 
    // into the tree
    d->tree = insert_helper(d->tree, word, d);
}

// This function will do most of the insert tasks. Will also perform rotations
// in the tree to help keep it balanced (since it is an AVL tree) 
Node insert_helper(Node d, char *word, Dict rep) {

    if (d == NULL) {
        d = new_word(word);
        // Dereference the nwords variable and add to it
        // whenever there is a unique word that is added
        (*rep).nwords++;
        return d;
	} else if (strcmp(word, d->item.word) < 0) { // If the word is lower than
    // the nodes word alphabetically
		d->left = insert_helper(d->left, word, rep);
	} else if (strcmp(word, d->item.word) > 0) { // If the word is higher than
    // the nodes word alphabetically
		d->right = insert_helper(d->right, word, rep);
	} else if (strcmp(word, d->item.word) == 0) { // If it's the same word,
    // increase the freq counter of it
        d->item.freq++;
        return d;
    } 
    
    // Find the balance of the tree after inserting the node 
    int balance = balance_factor(d);

    // Left left rotation case 
    if (balance > 1 && strcmp(word, d->left->item.word) < 0) {
        return right_rotate(d);
    }

    // Right right rotation case
    if (balance < -1 && strcmp(word, d->right->item.word) > 0) {
        return left_rotate(d);
    }

    // Left right rotation case 
    if (balance > 1 && strcmp(word, d->left->item.word) > 0) {
        d->left = left_rotate(d->left);
        return right_rotate(d);
    }

    // Right left rotation case
    if (balance < -1 && strcmp(word, d->right->item.word) < 0) {
        d->right = right_rotate(d->right);
        return left_rotate(d);
    }

    // Finally return the unchanged node pointer
    return d;
}

///////////////////////DictFind and helper functions///////////////////////////
// Returns the occurrence count of the given word. Returns 0 if the word
// is not in the Dictionary.
int DictFind(Dict d, char *word) {

    // Checking the passed in word against those in the tree
    // based on the return from strcmp() then we use recursion to 
    // traverse the tree until the word is either found and the 
    // frequency is returned or 0 as it doesn't exist 
    int occurrence = DictFind_helper(d->tree, word);  

    return occurrence;  
}

// This helper function compares the word and the nodes of the dictionary 
// and returns the frequency count to the DictFind function
int DictFind_helper(Node d, char *word) {
    
    if (d == NULL) {
        return 0;
    } else if (strcmp(word, d->item.word) < 0) { // If the word is lower than
    // the nodes word alphabetically
        return DictFind_helper(d->left, word);
    } else if (strcmp(word, d->item.word) > 0) { // If the word is higher than
    // the nodes word alphabetically
        return DictFind_helper(d->right, word);
    } else {
        return d->item.freq;
    }
}

//////////////////////DictFindTopN and helper functions////////////////////////
// Finds  the top `n` frequently occurring words in the given Dictionary
// and stores them in the given  `wfs`  array  in  decreasing  order  of
// frequency,  and then in increasing lexicographic order for words with
// the same frequency. Returns the number of WFreq's stored in the given
// array (this will be min(`n`, #words in the Dictionary)) in  case  the
// Dictionary  does  not  contain enough words to fill the entire array.
// Assumes that the `wfs` array has size `n`.

// This is the comparison function for qsort() in DictFindTopN function 
static int compareFreq(const void *pa, const void *pb) {
    const WFreq *p1 = pa;
    const WFreq *p2 = pb;
    return p2->freq - p1->freq;
}

// This function will calculate the number of words that need to be displayed 
// along with storing the words and freq of the nodes from the tree
int DictFindTopN(Dict d, WFreq *wfs, int n) {

    if (d->nwords == 0) {
        return 0;
    }

    // This array will store the words from the tree when converted
    // to an array
    WFreq arr[d->nwords];
    // Function to place the values of the nodes in the AVL tree into
    // an array 
    TreeToArray(arr, d->tree);
    // qsort() function will sort the words in the array into decreasing order 
    // in terms of frequency value
    qsort(arr, d->nwords, sizeof(WFreq), compareFreq);

    // Checking how many words to display and return to the main function
    if (d->nwords < n) {
        for (int i = 0; i < d->nwords; i++) { // If the number of top words
        // wanted by the user is larger than the number of unique words
        // return the number of unique values and store words in array
            wfs[i] = arr[i];
        }
        return d->nwords;
    } else {
            for (int i = 0; i < n; i++) { // If the number of top words
        // wanted by the user is smaller than the number of unique words
        // return the users desired number and store words in array
                wfs[i] = arr[i];
            }
            return n;
    }
}

// This function adds the WFreq structs of the dictionary into an array 
WFreq TreeToArray(WFreq *wfs, Node d) {

    static int i = 0;
    // Searching the tree in IN ORDER traveral 
    // and adding to the tree
    if(d->left != NULL) {
          TreeToArray(wfs, d->left);
    }
    if (d->item.word != NULL && d->item.freq > 0) { // Add to the array 
        wfs[i].freq = d->item.freq;
        wfs[i].word = d->item.word;
        i++;
    }

    if(d->right != NULL) {
          TreeToArray(wfs, d->right);
    }
    // Return the array storing all the words back 
    return *wfs;
}

///////////////////////DictShow and helper functions///////////////////////////
// Displays the given Dictionary. This is purely for debugging purposes,
// so  you  may  display the Dictionary in any format you want.  You may
// choose not to implement this.
void DictShow(Dict d) {

    view_tree(d->tree, 0);
    return;
}

// Function to view the tree sideways 
static void view_tree(Node d, int depth) {

    if (d != NULL) {
        view_tree(d->right, depth + 1);
        for (int i = 0; i < depth; i++) {
            printf("    ");
        }
        printf("%s %d\n", d->item.word, d->item.freq);
        view_tree(d->left, depth + 1);
    }  
}
