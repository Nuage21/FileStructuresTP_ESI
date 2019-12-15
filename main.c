#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "model.h"

#define LEN(X) (sizeof(X) / sizeof(X[0])) // get array length

int main() {
    FILE *file = NULL;
    fheader_t fheader;
    fblock_t fblock;

    fblock.total = 0;
    f_open(&file, "myfile.txt", &fheader, 'n');
    for(int i = 0; i < MAX_ARR; ++i)
    {
        fblock.arr[i] = 42818;
        fblock.raz[i] = ' ';
        (fblock.total)++;
    }
    fheader.bck = 1;
    fheader.ins = MAX_ARR;

    dir_write(file, 0, &fblock);
    f_close(file, &fheader);

    fheader.bck = 0xffeeffee;

    f_open(&file, "myfile.txt", &fheader, 'r');

    printf("total block = %d\n", fheader.bck);

    fblock_t buf;
    dir_read(file, 0, &buf);

    printf("block_0[0] = %d\n", buf.arr[0]);

    f_close(file, &fheader);
    return 0;
}
