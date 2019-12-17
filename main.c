#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "model.h"

#define LEN(X) (sizeof(X) / sizeof(X[0])) // get array length

// functions
void blck_adjust(fblock_t *buf);
long f_adjust(FILE *_f, fheader_t *_fheader, fblock_t *_buf1, fblock_t *_buf2);
void f_load(FILE *_f, fheader_t *_fheader, fblock_t *buf);
long f_del_unordered(FILE *_f, fheader_t *_fheader, fblock_t *buf, long _blck, int offset);

int main()
{
    FILE* f = NULL;
    fheader_t fheader;
    fblock_t buf;
    buf.total = 0;

    f_open(&f, "myfile.txt", &fheader, 'n');
    memset(&buf, 0, sizeof(fblock_t));

    f_load(f, &fheader, &buf);

    f_close(f, &fheader);

    f_open(&f, "myfile.txt", &fheader, 'r');

    f_show(f, &buf, 0, fheader.bck - 1);

    printf("____________________________AFTER__ADJUST_____________________________\n");

    f_del_unordered(f, &fheader, &buf, 0, 0);

    f_show(f, &buf, 0, fheader.bck - 1);

    printf("Adjusting op has taken: ops\n");
    f_close(f, &fheader);

    return 0;
}

// delete offset'th structure from the _blck'th block replacing it with the last element of the file
// if element doesn't existe then it'll just be replaced with the last one;
long f_del_unordered(FILE *_f, fheader_t *_fheader, fblock_t *buf, long _blck, int offset)
{
    if(_blck >= _fheader->bck || offset >= MAX_ARR)
        return 0;

    bool found = false;
    long last_strct = 0;
    long io_counter = 0;
    int i = _fheader->bck - 1; // last block index & iterate back till find a non-empty one

    while(!found)
    {
        blck_read(_f, i, buf);
        io_counter++; // read

        if(buf->total > 0) // has at least one element ?
        {
            // get last element
            for(int j = MAX_ARR; j >= 0; j--)
            {
                if(buf->raz[j] == ' ')
                {
                    last_strct = buf->arr[j]; // gotcha
                    buf->raz[j] = 'x';
                    blck_write(_f, _blck, buf); // kill the last one
                    found = true;
                    io_counter++; // write
                    break;
                }
            }
        }
        i--;
    }
    if(found)
    {
        blck_read(_f, _blck, buf);
        buf->arr[offset] = last_strct;
        blck_write(_f, _blck, buf);
        io_counter += 2;
    }
    return io_counter;
}

// move deleted structures to the extreme right;
void blck_adjust(fblock_t *buf)
{
    for(int i = 0; i < MAX_ARR; ++i)
        if(buf->raz[i] != ' ')
            for(int j = i + 1; j < MAX_ARR; ++j)
                if(buf->raz[j] == ' ')
                {
                    buf->arr[i] = buf->arr[j];
                    buf->raz[j] = buf->raz[i];
                    buf->raz[i] = ' ';
                    break;
                }
}


// adjust a sorted file to get a 100% loading rate (except for last block in most cases)
// file is kept sorted
// return total input/output to the hard drive

long f_adjust(FILE *_f, fheader_t *_fheader, fblock_t *_buf1, fblock_t *_buf2)
{
    long io_counter = 0;
    int z = 0;
    for(int i = 0;i < _fheader->bck;i++)
    {
        blck_read(_f, i, _buf1);
        io_counter++; // block read!

        if(_buf1->total == MAX_ARR)
            continue; // skip full blocks

        if(_buf1->total == 0)
        {
            io_counter += blck_del(_f, _fheader, _buf1, i);
            i--; // next block is at current position
            continue;
        }

        blck_adjust(_buf1);

        int p = _buf1->total; // ex: starts from here 1 2 3 |4| * * . . . and fill with next block els
        int j = i + 1; // starts from block at i+1 and find a !empty one

        // while array not full
        while(p < MAX_ARR && j < _fheader->bck)
        {
            // iterate until find a not-empty block
            do
            {
                blck_read(_f, j++, _buf2);
                io_counter++; // block read!
            }
            while(_buf2->total <= 0 && j < _fheader->bck);

            // if all the remaining blocks are empty (didn't found any) or arrived at last one
            if(_buf2->total <= 0 || j > _fheader->bck)
            {
                // go out { left blocks are empty - kick'em out }
                _fheader->bck = i + 1; // last block is i'th one
                break;
            }

            blck_adjust(_buf2); // adjust next block

            int q = 0;
            for(;(q < _buf2->total) && (p < MAX_ARR); ++p, ++q)
            {
                _buf1->arr[p] = _buf2->arr[q];
                _buf2->raz[q] = _buf1->raz[p]; // moved => erased or not set
                _buf1->raz[p] = ' ';
                _buf1->total++;
            }

            _buf2->total -= q; // sub moved elements from total

            // write down modified blocks
            blck_write(_f, i, _buf1);
            blck_write(_f, j - 1, _buf2);

            io_counter += 2; // block written twice!

        }
    }
    if(_buf1->total > 0) // only if has elements
        blck_write(_f, _fheader->bck - 1, _buf1); // write last block
    return ++io_counter;
}



