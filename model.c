//
// Created by Admin on 12/15/2019.

#include "model.h"
#include <stdio.h>
#include <io.h>


void f_open(FILE** _f_holder, char *_filename, fheader_t *_h_holder, char _mode)
{
    if ( _mode == 'R' || _mode == 'r' ) {
        *_f_holder = fopen(_filename, "rb+" );
        if ( *_f_holder == NULL ) perror("file not found! (@open_file)");
        fread(_h_holder, sizeof(fheader_t), 1, *_f_holder);
    }
    else { // mode == 'N' || mode == 'n'
        *_f_holder = fopen(_filename, "wb+" );
        if ( *_f_holder == NULL ) perror("file not found! (@open_file)");
        _h_holder->bck = 0;
        _h_holder->ins = 0;
        _h_holder->sup = 0;
        fwrite(_h_holder, sizeof(fheader_t), 1, *_f_holder);
    }
}

// updates header
void f_close(FILE *_f_holder, fheader_t *_h_holder )
{
    fseek(_f_holder, 0L, SEEK_SET);
    fwrite(_h_holder, sizeof(fheader_t), 1, _f_holder);
    fclose(_f_holder);
}


// _i starts at 0 {ARRAY INDEXES BEGIN AT 0 - PLEASE DON'T GET US CONFUSED IN OUR NEXT JOB FIELD - SAHITSOU!}
void blck_read( FILE *_f_holder, long _i, fblock_t *_buf )
{
    fseek(_f_holder, sizeof(fheader_t) + _i * sizeof(fblock_t), SEEK_SET);
    fread(_buf, sizeof(fblock_t), 1, _f_holder);
}


void blck_write( FILE *_f_holder, long _i, fblock_t *_buf )
{
    fseek(_f_holder, sizeof(fheader_t) + _i * sizeof(fblock_t), SEEK_SET);
    fwrite(_buf, sizeof(fblock_t), 1, _f_holder);
}

// block deletion
long blck_del(FILE *_file, fheader_t *_fheader, fblock_t *_buf, long _i)
{
    if(_i >= _fheader->bck)
        return 0;
    // else => @_i = last block
    if(_i < _fheader->bck - 1)
        for(int i = _i + 1; i < _fheader->bck; i++)
        {
            blck_read(_file, i, _buf);
            blck_write(_file, i - 1, _buf);
        }
    _fheader->bck--;
    _chsize(_fileno(_file), sizeof(fheader_t) + sizeof(fblock_t) * _fheader->bck);
    return 2 * (_fheader->bck - _i - 1);
}

// BinarySearch
long f_binary_search( FILE* _f, fheader_t *_fheader, fblock_t *_buf, long val, int *found, long *i, int *j)
{
    long bi, bs, cpt;
    int stop, inf, sup;

    bi = 0;
    bs = _fheader->bck;
    *found = 0;
    stop = 0;
    cpt = 0; // counter of physical inputs

    while ( bi <= bs && !*found && !stop ) {
        *i = (bi + bs) / 2;		// just a BS classic
        blck_read(_f, *i, _buf ); cpt++; 	// read middle block
        if ( val < _buf->arr[0] )
            bs = *i - 1;		// go left
        else
        if (val > _buf->arr[_buf->total-1])
            bi = *i + 1;	// go right
        else {
            stop = 1;	// block found! now let's seek the value inside...
            inf = 0;
            sup = _buf->total-1;
            // binary search inside the containing? block
            while (inf <= sup && !*found) {
                *j = (inf + sup) / 2;
                if ( val == _buf->arr[*j])
                {
                    if( _buf->raz[*j] == ' ')
                        *found= 1;
                    return cpt;
                }
                else
                if ( val < _buf->arr[*j] )
                    sup = *j - 1;
                else
                    inf = *j + 1;
            }
            if ( inf > sup ) *j = inf;
        }
    }
    if ( bi > bs ) {
        *i = bi;
        *j = 0;
    }
    return cpt;
}


// logic deletion of a file's in-value
long f_del(FILE* _f, fheader_t *_fheader, fblock_t *_buf, int _val)
{
    long i, cptR;
    int found, j;

    cptR = f_binary_search(_f, _fheader, _buf, _val, &found, &i, &j );  // return total physical inputs

    if (!found || _buf->raz[j] == '*') {
        printf("error: can't delete value! not found!\n");
        return cptR;
    }

    _buf->raz[j] = '*';   // logical kick out
    blck_write(_f, i, _buf );

    _fheader->sup++;

    return cptR + 1;

}

void blck_show(fblock_t *_buf)
{
    printf("|block \t total-stored-structs = %2d \t max-capacity = %2d]\n",  _buf->total, MAX_ARR);

    for(int j=0; j<MAX_ARR; j++)
        if (_buf->raz[j] == ' ')
            printf("%ld ", _buf->arr[j]);
        else if(_buf->raz[j] == '*')
            printf("*%ld* ", _buf->arr[j]);
        else
            printf("~ "); // unset
    printf("\n________________________________________________\n");
}
// display blocks @[min, max]
void f_show(FILE* _f, fblock_t *_buf, int min, int max)
{
    int j;

    for (;min <= max; ++min) {
        blck_read(_f, min, _buf);
        printf("\n[%3ld'th ", min);
        blck_show(_buf);
    }

}

//

