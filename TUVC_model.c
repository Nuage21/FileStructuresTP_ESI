//
// Created by Admin on 12/15/2019.

#include "TUVC_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <io.h>


// to use the same header structure for both file types, this fnct get the first
// free position of the last block that would normally be stored inside the strct.pad
size_t fh_get_freepos(fheader_t *_header)
{
    size_t first_free_position = -1;
    memmove(&first_free_position, _header->pad, sizeof(size_t));
    return first_free_position;
}

void fh_set_freepos(fheader_t *_header, size_t _pos)
{
    memmove(_header->pad, &_pos, sizeof(size_t));
}

// _i starts at 0 {ARRAY INDEXES BEGIN AT 0 - PLEASE DON'T GET US CONFUSED IN OUR NEXT JOB FIELD - SAHITSOU!}
void TUVC_blck_read( FILE *_f_holder, long _i, TUVCblock_t *_buf )
{
    fseek(_f_holder, sizeof(fheader_t) + _i * sizeof(TUVCblock_t), SEEK_SET);
    fread(_buf, sizeof(TUVCblock_t), 1, _f_holder);
}


void TUVC_blck_write( FILE *_f_holder, long _i, TUVCblock_t *_buf )
{
    fseek(_f_holder, sizeof(fheader_t) + _i * sizeof(TUVCblock_t), SEEK_SET);
    fwrite(_buf, sizeof(TUVCblock_t), 1, _f_holder);
}


// show _buf's data
__overlap__ TUVCblck_show(TUVCblock_t *_buf, int i)
{
    printf("|block|");
    printf("\n________________________________________________\n");
    char raz = 0xff;
    unsigned int len = 0;
    __overlap__ overlapped_data = INVALID_OVERLAP;

    while(i < TUVC_MAX)
    {
        raz = *(_buf->arr + i); // logic erase byte index read
        i++; // next pos

        // enough left space ?
        if(i + sizeof(int) > TUVC_MAX)
        {
            size_t leftlen = TUVC_MAX - i;
            char *store = (char*) malloc(leftlen + 1);

            *store = raz; // move erase index
            memmove(store+1, _buf->arr + i, leftlen); // move length

            overlapped_data.data = store;
            overlapped_data.len = leftlen + 1;

            return overlapped_data;
        }

        memmove(&len, _buf->arr + i, sizeof(int)); // read length

        if(len == 0) // no empty data please!
            break;

        i += sizeof(unsigned int); // next param pos

        if(i + len > TUVC_MAX)
        {
            size_t leftlen = TUVC_MAX - i;
            char *store = (char*) malloc(leftlen + 1 + sizeof(int));
            *store = raz; // move erase index
            memmove(store+1, &len, sizeof(int)); // move length
            memmove(store+1 + sizeof(int), _buf->arr + i, leftlen);
            overlapped_data.data = store;
            overlapped_data.len = 1 + sizeof(int) + leftlen;

            break;
        }

        char *data = (char*) malloc(len + 1);
        memset(data, 0, len + 1);

        memmove(data, _buf->arr + i, len); // read data

        if(raz == '*')
            printf('%c', raz);

        printf(data); // display data

        if(raz == '*')
            printf('%c', raz);

        printf(" "); // some space

        i += len;
        free(data);
    }
    printf("\n________________________________________________\n");
    return overlapped_data;
}

void TUVCf_show(FILE *file, TUVCblock_t *buf, int min, int max)
{
    __overlap__ ov_data = INVALID_OVERLAP;
    while(min <= max)
    {
        int st = 0;
        TUVC_blck_read(file, min, buf);
        if(ov_data.len > 0)
        {
            char raz = *ov_data.data; // erase index
            unsigned int data_size = -1;
            char *data = NULL;
            // get data_size
            if(ov_data.len > 1)
            {
                // if data-size is overlapped over two buffers
                if(ov_data.len - 1 < sizeof(int))
                {
                    char sz[4] = {'\0', '\0', '\0', '\0'}; // size holder
                    memmove(sz, ov_data.data + 1, ov_data.len - 1); // first part
                    st = sizeof(int) - ov_data.len + 1;
                    memmove(sz + ov_data.len - 1 , buf->arr, st); //2nd part in the buffer
                    memmove(&data_size, sz, sizeof(int)); //2nd part in the buffer
                    data = (char *) malloc(data_size + 1);
                    *(data+data_size) = '\0'; // taken as a string
                    memmove(data, buf->arr + st, data_size); //2nd part in the buffer
                    st += data_size;
                }
                else // data-size is contained not overlapped!
                {
                    memmove(&data_size, ov_data.data + 1, sizeof(int));

                    data = (char *) malloc(data_size + 1);
                    *(data+data_size) = '\0'; // taken as a string
                    int data_still = ov_data.len - 1 - sizeof(int);

                    if(data_still > 0) // still some data in the last buffer
                    {
                        memmove(data, ov_data.data + 1 + sizeof(int), data_still); // init with the part from previous buf
                        data_size -= data_still;
                    }
                    memmove(data + data_still, buf->arr, data_size); //2nd part in the buffer
                    st = data_size;

                }

            }
            else if(ov_data.len == 1)
            {
                memmove(&data_size, buf->arr, sizeof(int));
                data = (char *) malloc(data_size + 1);
                *(data+data_size) = '\0'; // taken as a string
                memmove(data, buf->arr + sizeof(int), data_size); //2nd part in the buffer
                st = sizeof(int) + data_size;
            }
            printf("\n|Overlapped data: ~%c%s%c~|\n\n", raz, data, raz);
            free(data);
        }
        printf("%d'th ", min);

        if(ov_data.len > 0)
            free(ov_data.data);

        ov_data = TUVCblck_show(buf, st);
        min++;
    }
}


// insert _val in buf
// return size of all-inserts
unsigned int TUVCblck_insert(TUVCblock_t *buf, char *_val, int _pos)
{
    unsigned int len = strlen(_val);
    *(buf->arr + _pos) = ' '; // not erased

    _pos++;

    if(_pos >= TUVC_MAX)
        return 1; // only erase-index inserted

    int left_space = TUVC_MAX - _pos;

    left_space = (left_space >= sizeof(int))? sizeof(int): left_space; // correction

    memmove(buf->arr + _pos, &len, left_space); // write length

    if(left_space < sizeof(int)) // if not all length inserted then return
        return 1 + left_space; // only erase-index inserted

    _pos += sizeof(int);

    len = (_pos + len < TUVC_MAX)?len:TUVC_MAX-_pos;

    memmove(buf->arr + _pos, _val, len); // write data

    return 1 + sizeof(int) + len;
}

void TUVCf_insert(FILE *file, fheader_t *header, TUVCblock_t *buf, char *_val)
{
    memset(buf, 0, sizeof(TUVCblock_t));

    size_t last_freepos = fh_get_freepos(header);
    int val_len = strlen(_val);

    if(header->bck != 0 && last_freepos != 0) // if bck=0 or freepos=0, useless to read, just edit a newone
        TUVC_blck_read(file, header->bck - 1, buf);

    if(header->bck == 0)
        header->bck++;

    unsigned int len = TUVCblck_insert(buf, _val, last_freepos);

    TUVC_blck_write(file, header->bck - 1, buf);

    int written_data_len = val_len + 1 + sizeof(int); // includes erase-byte & prefixed length

    if(len < written_data_len) // is there any overlapping ?
    {
        memset(buf, 0, sizeof(TUVCblock_t));

        // treat overlapped length & data
        char *tmp = (char*) malloc(val_len + sizeof(int));
        memmove(tmp, &val_len, sizeof(int));
        memmove(tmp + sizeof(int), _val, val_len);

        memmove(buf, tmp + len - 1, written_data_len - len);
        header->bck++; // new block created
        fh_set_freepos(header, written_data_len - len); // new last free pos
        TUVC_blck_write(file, header->bck - 1, buf);
    }
    else
    {
        if(last_freepos + written_data_len >= TUVC_MAX)
        {
            header->bck++;
            fh_set_freepos(header, 0); // new last free pos on the new block
        }
        else
            fh_set_freepos(header, last_freepos + len); // new last free pos
    }

    header->ins++;
}

// load file with total_data strings
void TUVCf_load(FILE *file, fheader_t *header, TUVCblock_t *buf, unsigned int total_data)
{
    // YES! to make quick I've could just insert them inside buf and do so for the buf when is full
    // but really I've got no time! just consider that I got well the lesson xD
    for(int i = 0; i < total_data; i++)
    {
        char *randw = rand_word();
        TUVCf_insert(file, header, buf, randw);
        free(randw);
    }
}

//

