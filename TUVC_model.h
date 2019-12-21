//
// Created by Admin on 12/18/2019.
//

#ifndef ESI_TP_TUVC_MODEL_H
#define ESI_TP_TUVC_MODEL_H

#include <stdio.h>
#include <ctype.h>
#include "model.h"

#define TUVC_MAX 512
#define INVALID_OVERLAP {.data = NULL, .len = 0};

typedef unsigned char byte_t;

typedef struct  TUVC_file_block
{
    char arr[TUVC_MAX];

} TUVCblock_t;

typedef struct  TUVC_block_element
{
    char razed;
    int size;
    char *data;

} TUVCelt_t;

typedef struct __overlap__{
    char *data;
    size_t len;
} __overlap__;


// _i starts at 0 {ARRAY INDEXES BEGIN AT 0 - PLEASE DON'T GET US CONFUSED IN OUR NEXT JOB FIELD - SAHITSOU!}
void TUVC_blck_read( FILE *_f_holder, long _i, TUVCblock_t *_buf );

void TUVC_blck_write(FILE *_f_holder, long _i, TUVCblock_t *_buf);

// to use the same header structure for both file types, this fnct get the first
// free position of the last block that would normally be stored inside the strct.pad
size_t fh_get_freepos(fheader_t *_header);

// set in header the first free position of the last header
void fh_set_freepos(fheader_t *_header, size_t _pos);

// insert _val in buf
// return size of inserted data 1byte(erase-index) + 4by(data-size-holder) + data-size
unsigned int TUVCblck_insert(TUVCblock_t *buf, char *_val, int _pos);

// insert _val to file
void TUVCf_insert(FILE *file, fheader_t *header, TUVCblock_t *buf, char *_val);

// show _buf's data
// return the data and length of the last element if overlapped
// if not then return INVALID_OVERLAPPED
__overlap__ TUVCblck_show(TUVCblock_t *_buf, int i);

// display blocks @[min, max]
void TUVCf_show(FILE *file, TUVCblock_t *buf, int min, int max);

// truncate _n blocks from _file (change its size & its bck-num prop)
void f_truncate_blocks(FILE *_file, fheader_t *_fheader, long _n);

#endif
