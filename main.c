#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "model.h"
#include "TUVC_model.h"
#include "util.h"

/******************************************************************
 * Author: Hakim Beldjoudi @esi-2cp-g3
 * code can be found at https://github.com/hbFree/FileStructuresTP_ESI
 * help.txt must be at the same directory as the executable program
 * Vocabulary: TUVC -> file of type: TABLE(ARRAY)-UNORDERED-VARIABLE_DATA_LENGTH-OVERLAPPING(CHEVAUCHEMENT)
 *
 * fr/
 * Pour ne pas trop repeter du code j'ai utilise la meme entete pour les deux types de fichiers et inclue
 * la position de la premiere position libre dans le pad de l'entete!
 * Le TP traite est le Niveau 1 dont exo-3, exo-6 & exo-13
 * Bonne correction!
 */


// functions
void blck_adjust(fblock_t *buf);
long f_adjust(FILE *_f, fheader_t *_fheader, fblock_t *_buf1, fblock_t *_buf2);
void f_load(FILE *_f, fheader_t *_fheader, fblock_t *buf);
long f_del_unordered(FILE *_f, fheader_t *_fheader, fblock_t *buf, long _blck, int offset);

#define TOF_FILE  0
#define TUVC_FILE 1

int main()
{
    FILE* f = NULL;
    fheader_t fheader;
    fblock_t buf1, buf2;
    TUVCblock_t tbuf1, tbuf2; // T~OVC buffers

    char taskbuf[20];
    char filename[FILENAME_MAX];
    char answer = '\0';
    long from = 0, to = 0, tmp = 0, tmp1 = 0;
    f_coord cd = INVALID_FCOORD();

    int file_type = TOF_FILE; // current file type

    show_help();

    while(true)
    {
        memset(filename, 0, FILENAME_MAX);
        memset(&buf1, 0, sizeof(fblock_t));
        memset(&buf2, 0, sizeof(fblock_t));
        memset(taskbuf, 0, 20);

        printf(">> ");
        scanf("%s", taskbuf);
        int opcode = get_opcode(taskbuf);
        fflush(stdin);

        if(opcode == QUITAPP_OP)
        {
            if(f != NULL)
            {
                f_close(f, &fheader);
                printf("file closed - bye!\n");
            }
            break;
        }

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
                    printf("file type (0 for TOF/ 1 for T~OVC) = ");
                    scanf("%d", &file_type);
                    if(file_type != 0 && file_type != 1)
                    {
                        printf("file type taken as TOF!\n");
                        file_type = 0; // some correction
                    }
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
                {
                    if(file_type == TOF_FILE)
                        f_load(f, &fheader, &buf1);
                    else
                    {
                        printf("total elements to insert in the file= ");
                        scanf("%d", &tmp);
                        TUVCf_load(f, &fheader, &tbuf1, tmp);
                    }
                }
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
                    if(file_type == TOF_FILE)
                        f_show(f, &buf1, from, to);
                    else
                        TUVCf_show(f, &tbuf1, from, to);
                }
                break;

            case DELETE_OP:
                if(f == NULL)
                    printf("error: no file is opened yet!\n");
                else
                {
                    printf("type value to delete: ");
                    if(file_type == TOF_FILE)
                    {
                        scanf("%d", &tmp);
                        tmp = f_del(f, &fheader, &buf1, tmp);
                        if(tmp > 0)
                            printf("value has been succesfully deleted!\n");
                        tmp = (tmp>=0)?tmp:-tmp;
                        printf("task has taken %d i/o ops\n", tmp);
                    }
                    else
                    {
                        scanf("%s", filename);
                        tmp = TUVCf_del(f, &fheader, &tbuf1, filename);
                        if(tmp == 0)
                            printf("value has been succesfully deleted!\n");
                        else
                            printf("error: value not found!\n");
                    }
                }
                break;

            case UDELETE_OP:
                if(f == NULL)
                    printf("error: no file is opened yet!\n");
                else
                {
                    if(file_type == TUVC_FILE)
                    {
                        printf("can't run this op! for T~OVC files, deletion is logically performed\n");
                        break;
                    }

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
                        printf("success: value at block %d offset %d is deleted, taken %d i/o ops!\n", from ,to, tmp);
                    else
                        printf("error: coords out of range!\n");

                }
                break;

            case SEARCH_OP:
                if(f == NULL)
                    printf("error: no file is opened yet!\n");
                else {
                    if(file_type == TOF_FILE)
                    {
                        printf("value to seek is: ");
                        scanf("%d", &tmp);

                        tmp1 = f_binary_search(f, &fheader, &buf1, tmp, &tmp, &from, &to);
                        if (tmp > 0) // if found
                            printf("value found in block %d at position %d\n", from, to);
                        else
                            printf("value not found\n");
                        printf("task has taken %d i/o ops\n", tmp1);
                    }
                    else // T~OVC
                    {
                        printf("value to seek is: ");
                        scanf("%s", filename);
                        cd = TUVCf_search(f, &fheader, &tbuf1, filename);
                        if(cd.offset < 0 || cd.bck < 0)
                            printf("value not found\n");
                        else
                            printf("value found in block %d at position %d\n", cd.bck, cd.offset);

                    }
                }
                break;
            case CLEAR_OP:
                clear_screen();
                break;

            case HELP_OP:
                show_help();
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



