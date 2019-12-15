//
// Created by Admin on 12/15/2019.
//

#ifndef ESI_TP_MODEL_H
#define ESI_TP_MODEL_H

#include<stdio.h>

// max structures per block
#define MAX_ARR 60

typedef unsigned char byte_t;

typedef struct file_block{
    int arr[MAX_ARR];
    char raz[MAX_ARR];
    size_t total; // how many structures are in
    byte_t pad[218]; // as suggested, to get a 512 bytes block
} fblock_t;

typedef struct file_header{
    size_t bck; // total blocks of this file
    size_t ins; // total insertions
    size_t sup; // total deletions
    byte_t pad[488];
} fheader_t;

// open @_filename file (if-exist): reference @f_holder & h_holder stores its header
void f_open(FILE** _f_holder, char *_filename, fheader_t *_h_holder, char mode);

// close _file and get him a hew _header (stored at pos 0)
void f_close(FILE *_file, fheader_t *_header);

// read the _i'th block from _file & stores it @buf
void dir_read( FILE *_file, long _i, fblock_t *_buf);

// write down in@buf at _i'th pos of _file
void dir_write( FILE *_file, long _i, fblock_t *_buf );

#endif //ESI_TP_MODEL_H
