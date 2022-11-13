// COMP2521 21T2 Assignment 1
// tw.c ... compute top N most frequent words in file F
// Usage: ./tw [Nwords] File

// Written by Viraj Srikar Danthurty 
// z5359063@ad.unsw.edu.au
// Written in June/July 2021

// Program function:
// This is the main program which will take in the input of a file name
// and a number (which represents the number of words) to be displayed. 
// This program does pre processing of the words in the file (tokenise, 
// normalise, remove stopwords and stem) and then insert into a dictionary
// then will return with the top 'n' frequent words in that file and what
// what it was

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "stemmer.h"
#include "WFreq.h"

///////////////////////////////////////////////////////////////////////////////
//                                DEFINITIONS                                //
/////////////////////////////////////////////////////////////////////////////// 

#define MAXLINE 1000
#define MAXWORD 100
#define isWordChar(c) (isalnum(c) || (c) == '\'' || (c) == '-')

// Constants I created for better readability
#define TRUE 1
#define FALSE 0
#define BOOK_START "*** START OF"
#define BOOK_END "*** END OF"

///////////////////////////////////////////////////////////////////////////////
//                           FUNCTION PROTOTYPES                             //
///////////////////////////////////////////////////////////////////////////////

void file_pre_process (FILE *file, Dict d, Dict process, int user_number);
int valid_book_check(FILE *file);
Dict stopwords_dict (Dict d);

///////////////////////////////////////////////////////////////////////////////
//                             MAIN FUNCTION                                 //
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
	int   nWords;    // number of top frequency words to show
	char *fileName;  // name of file containing book text
    FILE *fp; // File pointer name 

	// Switch function to check the validity of the command line argument
	switch (argc) {
		case 2:
			nWords = 10;
			fileName = argv[1];
			break;
		case 3:
			nWords = atoi(argv[1]);
			if (nWords < 10) nWords = 10;
			fileName = argv[2];
			break;
		default:
			fprintf(stderr,"Usage: %s [Nwords] File\n", argv[0]);
			exit(EXIT_FAILURE);
	}
	

    // Create dictionary to store the words of the stopwords file
	Dict stopwords = DictNew();
    // Function adds the words in stopwords file into the dictionary 
    stopwords = stopwords_dict(stopwords);
    
    // Open the file up 
    // 'fp' is a file pointer. can pass to other functions, etc
    fp = fopen(fileName, "r");
    // Handle error if the program can't open the file passed in 
    if (fp == NULL) {
			fprintf(stderr, "Can't open %s\n", fileName);
			exit(EXIT_FAILURE);
	}
		
	// Handles error to check if file passed in is a valid book
	if (valid_book_check(fp) == FALSE) {
	    fprintf(stderr, "Not a Project Gutenberg book\n");
	    exit(EXIT_FAILURE);
	}

	// Create a new dictionary which will store the words after they
    // have pre processed
    Dict book_dictionary = DictNew();

    // Pass data into the function which does the pre processing of the 
    // text file passed in 
	file_pre_process(fp, stopwords, book_dictionary, nWords);
	
    // AFTER the top words are displayed, free the dictionary space created
    // for the stopwords dictionary
	DictFree(stopwords);
	// Close the file  		
    fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////
//                              FUNCTIONS                                    //
///////////////////////////////////////////////////////////////////////////////

// This function will get the file passed in and check if it a valid file 
// or not. Returns a value to the main function to handle the error if 
// it is valid
int valid_book_check(FILE *file) {
    int flag = FALSE;
    char line[MAXLINE];
    // Read through the lines of the file until the "*** START OF..."
    // line is encountered
    while (fgets(line, MAXLINE, file) != NULL) {
        if (strstr(line, BOOK_START)) {
            flag = TRUE;
        }
    }
    return flag;    
}

// This function will do the pre processing of the file passed in. It will
// tokenise, normalise, remove the stopwords, stem the word and then pass
// that word into the dictionary created in the main function. 
void file_pre_process (FILE *file, Dict d, Dict process, int user_number) {

    // Initialise arrays to store the words when searching     
    char line[MAXLINE];
    char word[MAXLINE];
    WFreq words[MAXWORD];

    // To reset the file pointer to the top again  
    rewind(file);

    char temp[MAXLINE];
    while (fgets(temp, MAXLINE, file) != NULL) { // Reads the file until the 
    // BOOK_START constant is reached
        if (strstr(temp, BOOK_START)) {
            break;
        }    
    }

    // Now will process the words after the "*** START OF..." line

    while (fgets(line, MAXLINE, file) != NULL) { // Checks the read in line 
    // isn't the "*** END OF..." line. If so, don't process lines after
        if (strstr(line, BOOK_END)) {
            break;         
        }     

        if (strcmp(line, "\n") != 0) { // Checks to see the character passed in
        // is not a new line
            int j = 0;
            int i = 0;
            while (line[i] != '\0') {	
                // Checks if each character of a word stored in the array line, 
                // is a valid character or not		
                if (isWordChar(line[i])) { // If valid, pass into the word array
                    word[j] = tolower(line[i]);
                    j++;                  
                } else if (!isWordChar(line[i]) || line[i] == ' ') { // Otherwise, break
                // off, make it into a string (by adding the null terminator) 
                    word[j] = '\0';
                    j = 0;
                    int check = DictFind(d, word);
                    if (strlen(word) != 1 && strcmp(word, "\0") != 0 && check == 0) { // Checking 
                    // if the word passed into array word[] is not a null terminator, the length isn't 1
                    // or that the word isn't present in the stopwords dictionary 
                        // Stem the word using the stemming program
                        stem(word, 0, strlen(word)- 1);
                        int len = strlen(word);
                        word[len + 1] = '\0';
                        // Insert the final word into the dictionary 
                        DictInsert(process, word);
                    }
                } 
            i++; 
            }           
        }             
    } 

    // Error statement to check if end of book was reached without reading "*** END OF..."
    if (strstr(line, BOOK_END) == NULL) {
        fprintf(stderr, "Not a Project Gutenberg book\n");
        exit(EXIT_FAILURE);

    }
    
    // This process will print out the most frequent 'n' number of words and 
    // it's respective freq value
    int number = DictFindTopN(process, words, user_number);
    int counter = 0;
    while (counter < number) {
        printf("%d %s\n", words[counter].freq, words[counter].word);
        counter++;
    }
    // Free the dictionary used to store the pre_processed words 
    DictFree(process);
}

// This function opens the stopwords dictionary, reads in each word and inputs
// it into the dictionary created to store all the words
Dict stopwords_dict(Dict d) {

    // Open the stopwords file
    FILE *fp;
    char word[MAXWORD];
    fp = fopen("stopwords", "r");

    // Error handling to check if the stopwords file opened up correctly 
    if (fp == NULL) {
            fprintf(stderr, "Can't open stopwords\n");
            exit(EXIT_FAILURE);
		}

    // Insert the words of the stopwords file into the dictionary 
    while(fscanf(fp, "%s\n", word) != EOF) {
        DictInsert(d, word);
    }

    // Close the file 
    fclose(fp);

    return d;
}
