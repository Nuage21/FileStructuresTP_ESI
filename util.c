//
// Created by Admin on 12/21/2019.
//

#include "util.h"

char* ops[] = {"open", "close", "load", "adjust", "quit", "del", "udel", "show", "search", "help", "clear"};

#define INVALID_OP   -1

int get_opcode(const char *_task)
{
    for(int i = 0; i < 11; i++)
        if(strcmp(_task, ops[i]) == 0)
            return i;
    return INVALID_OP;
}

void clear_screen()
{
#ifdef __WIN32__ // compile for windows
    system("cls");
#endif
#ifdef __linux__ // compile for linux
    system("clear");
#endif
}

void show_help() {
    FILE *file = fopen("help.txt", "r");
    if (file == NULL) {
        printf("error: can't find help file!\n");
        return;
    }
    char c = '\0';
    while ((c = getc(file)) != EOF)
        putchar(c);

    fclose(file);
}

// return a random a-z word
char* rand_word()
{
    static int s_randed = 0; // gotta call srand one time only
    if(s_randed == 0)
    {
        srand(time(0));
        s_randed = 0xff;
    }

    int word_len = (rand() % (RANDOM_WORD_MAX - RANDOM_WORD_MIN)) + RANDOM_WORD_MIN;
    char *word = (char*) malloc(word_len + 1);
    *(word + word_len) = '\0';
    for(int i = 0; i < word_len; i++)
    {
        char c = (char) ((rand() % 26) + 'a');
        *(word + i) = c;
    }
    return word;
}