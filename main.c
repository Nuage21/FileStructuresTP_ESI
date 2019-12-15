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
    fheader.bck = 1;
    fheader.ins = MAX_ARR;

    blck_write(f, 0, &buf);
    f_close(f, &fheader);

    fheader.bck = 0xffeeffee;

    f_open(&f, "myfile.txt", &fheader, 'r');

    printf("total block = %d\n", fheader.bck);

    blck_read(f, 0, &buf);

    // test deletion
    f_del(f, &fheader, &buf, 13);

    f_show(f, &buf, 0, 0);

    int found, off = -1;
    long  at_block = -1;
    int val = 12;

    f_binary_search(f, &fheader, &buf, val, &found, &at_block, &off);
    if(found)
        printf("value is at %d'th block with a %d's offset\n", at_block, off);
    f_close(f, &fheader);
    return 0;
}





