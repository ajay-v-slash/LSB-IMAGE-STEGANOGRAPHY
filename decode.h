#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include "types.h"

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;

    /* Secret File info */
    char secret_fname[20];     /* holds base name, then decoded extn gets appended (step 7) */
    FILE *fptr_secret;
    char extn_secret_file[5];
    long size_secret_file;
} DecodeInfo;

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);
Status open_dec_files(DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);

#endif