#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "model.h"

/******************************************************************
 * Author: Hakim Beldjoudi
 * code can be found at https://github.com/hbFree/FileStructuresTP_ESI
 *
 */
// functions
void blck_adjust(fblock_t *buf);
long f_adjust(FILE *_f, fheader_t *_fheader, fblock_t *_buf1, fblock_t *_buf2);
void f_load(FILE *_f, fheader_t *_fheader, fblock_t *buf);
long f_del_unordered(FILE *_f, fheader_t *_fheader, fblock_t *buf, long _blck, int offset);

char* ops[] = {"open", "close", "load", "adjust", "quit", "del", "udel", "show", "search"};

#define OPENFILE_OP   0
#define CLOSEFILE_OP  1
#define LOADFILE_OP   2
#define ADJUSTFILE_OP 3
#define QUITAPP_OP    4
#define DELETE_OP     5
#define UDELETE_OP    6 // unordered file value deletion by last replacing
#define SHOW_OP       7
#define SEARCH_OP     8 // binary search a value (par dichotomie)
#define INVALID_OP   -1

int get_opcode(const char *_task)
{
    for(int i = 0; i < 9; i++)
        if(strcmp(_task, ops[i]) == 0)
            return i;
    return INVALID_OP;
}
int main()
{
    FILE* f = NULL;
    fheader_t fheader;
    fblock_t buf1, buf2;

    char taskbuf[20];
    char filename[FILENAME_MAX];
    char answer = '\0';
    long from = 0, to = 0, tmp = 0, tmp1 = 0;
    while(true)
    {
        memset(filename, 0, FILENAME_MAX);
        memset(&buf1, 0, sizeof(fblock_t));
        memset(&buf2, 0, sizeof(fblock_t));
        memset(taskbuf, 0, 20);

        printf(">> ");
        scanf("%s", taskbuf);
        int opcode = get_opcode(taskbuf);

        if(opcode == QUITAPP_OP)
            break;

        switch(opcode)
        {
            case OPENFILE_OP:
                if(f != NULL)
                    printf("error: please close the current file and retry!\n");
                else
                {
                    printf("opening mode (new/read): ");
                    scanf("%s", &answer);
                    printf("filename = ");
                    scanf("%s", filename);
                    f_open(&f, filename, &fheader, answer);
                    printf("success: file %s is open!\n", filename);
                }
                break;

            case CLOSEFILE_OP:
                if(f == NULL)
                    printf("error: no file to close - nothing opened!\n");
                else
                {
                    f_close(f, &fheader);
                    f = NULL;
                }
                break;

            case LOADFILE_OP:
                if(f == NULL)
                    printf("error: no file to load elements to - nothing opened!\n");
                else
                    f_load(f, &fheader, &buf1);
                break;

            case ADJUSTFILE_OP:
                if(f == NULL)
                    printf("error: no file to adjust - nothing opened!\n");
                else
                {
                    tmp = f_adjust(f, &fheader, &buf1, &buf2);
                    printf("task has taken %d i/o ops\n", tmp);
                }
                break;

            case QUITAPP_OP:
                if(f != NULL)
                {
                    f_close(f, &fheader);
                    printf("file closed - bye!\n");
                }
                break;

            case SHOW_OP:
                if(f == NULL)
                    printf("error: no file is opened yet!\n");
                else
                {
                    printf("show from block: ");
                    scanf("%d", &from);
                    printf("to(type -1 for last block): ");
                    scanf("%d", &to);
                    if(to == -1)
                        to = fheader.bck - 1;
                    f_show(f, &buf1, 0, fheader.bck - 1);
                }
                break;

            case DELETE_OP:
                if(f == NULL)
                    printf("error: no file is opened yet!\n");
                else
                {
                    printf("type value to delete: ");
                    scanf("%d", &tmp);
                    tmp = f_del(f, &fheader, &buf1, tmp);
                    if(tmp > 0)
                        printf("value has been succesfully deleted!\n");
                    tmp = (tmp>=0)?tmp:-tmp;
                    printf("task has taken %d i/o ops\n", tmp);
                }
                break;

            case UDELETE_OP:
                if(f == NULL)
                    printf("error: no file is opened yet!\n");
                else
                {
                    printf("block(type -1 for last block): ");
                    scanf("%d", &from);
                    printf("position (type -1 for last el): ");
                    scanf("%d", &to);
                    if(to == -1)
                        to = MAX_ARR - 1;
                    if(from == -1)
                        to = fheader.bck - 1;
                    tmp = f_del_unordered(f, &fheader, &buf1, from, to);
                    if(tmp > 0)
                        printf("sucess: value at block %d offset %d is deleted, taken %d i/o ops!\n", from ,to, tmp);
                    else
                        printf("error: coords out of range!\n");

                }
                break;

            case SEARCH_OP:
                if(f == NULL)
                    printf("error: no file is opened yet!\n");
                else {
                    printf("value to seek is: ");
                    scanf("%d", &tmp);

                    tmp1 = f_binary_search(f, &fheader, &buf1, tmp, &tmp, &from, &to);
                    if (tmp > 0) // if found
                        printf("value found in block %d at position %d\n", from, to);
                    else
                        printf("value not found\n");
                    printf("task has taken %d i/o ops\n", tmp1);
                }
                break;
            default:
                printf("error: can't handle this operation (not found)\n");
        }

    }

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
                    blck_write(_f, i, buf); // kill the last one
                    memset(buf, 0, sizeof(fblock_t));
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



