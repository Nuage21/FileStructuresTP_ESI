//
// Created by Admin on 12/21/2019.
//

#ifndef ESI_TP_UTIL_H
#define ESI_TP_UTIL_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define OPENFILE_OP   0
#define CLOSEFILE_OP  1
#define LOADFILE_OP   2
#define ADJUSTFILE_OP 3
#define QUITAPP_OP    4
#define DELETE_OP     5
#define UDELETE_OP    6 // unordered file value deletion by last replacing
#define SHOW_OP       7
#define SEARCH_OP     8 // binary search a value (par dichotomie)
#define HELP_OP       9 // show help file
#define CLEAR_OP      10 // clear screen

// word rand
#define RANDOM_WORD_MAX 15
#define RANDOM_WORD_MIN 3


// get operation code of string command
int get_opcode(const char *_task);

// system("clear/cls")
void clear_screen();

// show help file content
void show_help();

// return a random a-z word
char* rand_word();

#endif //ESI_TP_UTIL_H
