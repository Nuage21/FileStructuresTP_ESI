//
// Created by Admin on 12/15/2019.

#include "model.h"
#include <stdio.h>


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
void dir_read( FILE *_f_holder, long _i, fblock_t *_buf )
{
    fseek(_f_holder, sizeof(fheader_t) + sizeof(fblock_t), SEEK_SET);
    fread(_buf, sizeof(fblock_t), 1, _f_holder);
}


void dir_write( FILE *_f_holder, long _i, fblock_t *_buf )
{
    fseek(_f_holder, sizeof(fheader_t) + sizeof(fblock_t), SEEK_SET);
    fwrite(_buf, sizeof(fblock_t), 1, _f_holder);
}

//

