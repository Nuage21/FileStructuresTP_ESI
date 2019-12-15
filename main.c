#include <stdio.h>
#include "model.h"

#define LEN(X) (sizeof(X) / sizeof(X[0])) // get array length

// functions

int main()
{
    FILE* f = NULL;
    fheader_t fheader;
    fblock_t buf;
    buf.total = 0;

    f_open(&f, "myfile.txt", &fheader, 'n');
    for(int i = 0; i < 15; ++i)
    {
        buf.arr[i] = i;
        buf.raz[i] = ' ';
        (buf.total)++;
    }

    blck_write(f, 0, &buf);
    buf.total = 0;

    for(int i = 16, j = 0; i < 90; i+=3, j++)
    {
        buf.arr[j] = i;
        buf.raz[j] = ' ';
        (buf.total)++;
    }

    blck_write(f, 1, &buf);

    fheader.bck = 2;
    fheader.ins = MAX_ARR;

    f_close(f, &fheader);

    fheader.bck = 0xffeeffee; // test read;

    f_open(&f, "myfile.txt", &fheader, 'r');

    f_show(f, &buf, 0, fheader.bck);

    // test deletion
    f_del(f, &fheader, &buf, 13);


    int found, off = -1;
    long  at_block = -1;
    int val = 10;

    f_binary_search(f, &fheader, &buf, val, &found, &at_block, &off);
    if(found)
        printf("value is at %d'th block with a %d's offset\n", at_block, off);
    else
        printf("not found!\n");

    f_close(f, &fheader);

    return 0;
}





