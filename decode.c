#include <stdio.h>
#include <string.h>
#include "types.h"
#include "common.h"
#include "decode.h"

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc < 3)
    {
        fprintf(stderr, "ERROR: Insufficient arguments\n");
        return e_failure;
    }

    // step 1 -> Check whether source image is .bmp
    char *c1 = strrchr(argv[2], '.');
    if (c1 != NULL && strcmp(c1, ".bmp") == 0)
    {
        printf("The valid bmp image file \n");
        // Store source image filename
        decInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // Get output filename If output name is not given use "output"
    if (argc == 4)
    {
        strcpy(decInfo->secret_fname, argv[3]);
    }
    else
    {
        strcpy(decInfo->secret_fname, "output");
    }

    return e_success;
}

Status open_dec_files(DecodeInfo *decInfo)
{
    // step 2 -> Open the source BMP file in read mode
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname, "rb");
    if (decInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_image_fname);
        return e_failure;
    }
    printf("Source File is opened Successfully !\n");
    return e_success;
}

void decode_byte_from_lsb(char *buffer, char *c)
{
    //extract 1 byte from lsb of a 8 byte
    for (int i = 0; i < 8; i++)
    {
        *c = (*c | ((buffer[i] & 1) << (7 - i)));
    }
}

void decode_size_from_lsb(char *buffer, int *size)
{
    //extract 4 bytes from 32 bytes lsb
    for (int i = 0; i < 32; i++)
    {
        *size = (*size | ((buffer[i] & 1) << (31 - i)));
    }
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    // step 4 -> decode magic string and check it is matching with MAGIC_STRING
    char buffer[8];
    int size = strlen(magic_string);
    printf("Started Decoding Magic String !\n");
    for (int i = 0; i < size; i++)
    {
        fread(buffer, 1, 8, decInfo->fptr_src_image);
        char c = '\0';
        decode_byte_from_lsb(buffer, &c);
        if (c != magic_string[i])
        {
            return e_failure;
        }
    }
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo, int *extn_size)
{
    // step 5 -> Decode secret file extension size
    char buffer[32];
    fread(buffer, 1, 32, decInfo->fptr_src_image);
    *extn_size = 0;
    decode_size_from_lsb(buffer, extn_size);
    return e_success;
}

Status decode_secret_file_extn(int extn_size, DecodeInfo *decInfo)
{
    // step 6 -> Decode secret file extension
    char buffer[8];
    if (extn_size < 0 || extn_size >= (int)sizeof(decInfo->extn_secret_file))
    {
        return e_failure;
    }
    for (int i = 0; i < extn_size; i++)
    {
        fread(buffer, 1, 8, decInfo->fptr_src_image);
        char c = '\0';
        decode_byte_from_lsb(buffer, &c);
        decInfo->extn_secret_file[i] = c;
    }
    decInfo->extn_secret_file[extn_size] = '\0';
    return e_success;
}

Status open_dec_secret_file(DecodeInfo *decInfo)
{
    // step 7 -> Create the output filename
    // Take the output name given by user (already in secret_fname), add decoded extn
    strcat(decInfo->secret_fname, decInfo->extn_secret_file);

    // Final filename e.g. "output.txt" - open this file for writing mode
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);
        return e_failure;
    }
    printf("Output File is opened Successfully : %s\n", decInfo->secret_fname);
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 1, 32, decInfo->fptr_src_image);
    int size = 0;
    decode_size_from_lsb(buffer, &size);
    decInfo->size_secret_file = size;
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 1, 8, decInfo->fptr_src_image);
        char c = '\0';
        decode_byte_from_lsb(buffer, &c);
        fputc(c, decInfo->fptr_secret);
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    // step 2 -> open source bmp file
    if (open_dec_files(decInfo) == e_success)
    {
        printf("File Opened successfully!\n");
    }
    else
    {
        printf("Error : Failed to open file !\n");
        return e_failure;
    }

    // step 3 -> Move file pointer to 54th byte (skip BMP header)
    if (fseek(decInfo->fptr_src_image, 54, SEEK_SET) == 0)
    {
        printf("The 54 bytes of bmp is skipped\n");
    }
    else
    {
        printf("The 54 bytes of bmp is not skipped\n");
        return e_failure;
    }

    // step 4 -> decode + verify magic string
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_success)
    {
        printf("The magic string is Found!\n");
    }
    else
    {
        printf("The magic string is not found!\n");
        return e_failure;
    }

    // step 5 -> decode secret file extension size
    int extn_size = 0;
    if (decode_secret_file_extn_size(decInfo, &extn_size) == e_success)
    {
        printf("Secret File Extension Size is Decoded : %d\n", extn_size);
    }
    else
    {
        printf("ERROR : Decoding Secret File Extension Size Failed!\n");
        return e_failure;
    }

    // step 6 -> decode secret file extension
    if (decode_secret_file_extn(extn_size, decInfo) == e_success)
    {
        printf("Secret File Extension is Decoded : %s\n", decInfo->extn_secret_file);
    }
    else
    {
        printf("ERROR : Decoding Secret File Extension Failed!\n");
        return e_failure;
    }

    // step 7 -> build final output filename (base + decoded extn) and open it
    if (open_dec_secret_file(decInfo) == e_success)
    {
        printf("Output filename created successfully\n");
    }
    else
    {
        printf("ERROR : Failed to open output secret file\n");
        return e_failure;
    }

    if (decode_secret_file_size(decInfo) == e_success)
    {
        printf("Secret File Size is Decoded : %ld\n", decInfo->size_secret_file);
    }
    else
    {
        printf("ERROR : Decoding Secret File Size Failed!\n");
        return e_failure;
    }

    if (decode_secret_file_data(decInfo) == e_success)
    {
        printf("Secret File Data is Decoded\n");
    }
    else
    {
        printf("ERROR : Decoding Secret File Data Failed!\n");
        return e_failure;
    }
    fclose(decInfo->fptr_src_image);
    fclose(decInfo->fptr_secret);
    return e_success;
}