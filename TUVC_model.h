//
// Created by Admin on 12/18/2019.
//

#ifndef ESI_TP_TUVC_MODEL_H
#define ESI_TP_TUVC_MODEL_H

#include <stdio.h>
#include <ctype.h>
#include "model.h"
#include "util.h"

#define TUVC_MAX 512

typedef unsigned char byte_t;

typedef struct  TUVC_file_block
{
    char arr[TUVC_MAX];

} TUVCblock_t;

typedef struct __overlap__{
    char *data;
    size_t len;
} __overlap__;

typedef struct infile_value_coord{
    unsigned long bck;
    unsigned long offset;
} f_coord;


// return an INVALID overlap
__overlap__ INVALID_OVERLAP();

// return an INVALID fcoord
f_coord INVALID_FCOORD();

// _i starts at 0 {ARRAY INDEXES BEGIN AT 0 - PLEASE DON'T GET US CONFUSED IN OUR NEXT JOB FIELD - SAHITSOU!}
void TUVC_blck_read( FILE *_f_holder, long _i, TUVCblock_t *_buf );

void TUVC_blck_write(FILE *_f_holder, long _i, TUVCblock_t *_buf);

// O(n) seek of val
f_coord TUVCf_search(FILE *file, fheader_t *header, TUVCblock_t  *buf, char *val);

// load file with total_data strings
void TUVCf_load(FILE *file, fheader_t *header, TUVCblock_t *buf, unsigned int total_data);

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

#endif
