#include <stdio.h>
#include <string.h>

#include "types.h"
#include "common.h"
#include "decode.h"

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc < 3)
    {
        fprintf(stderr, RED "ERROR: Insufficient arguments\n" RESET);
        return e_failure;
    }

    /* Step 1: Validate source BMP file */
    char *c1 = strrchr(argv[2], '.');

    if (c1 != NULL && strcmp(c1, ".bmp") == 0)
    {
        decInfo->src_image_fname = argv[2];
    }
    else
    {
        fprintf(stderr, RED "ERROR: Source file must be a .bmp file\n" RESET);
        return e_failure;
    }

    /* Step 2: Get output file name */
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
    /* Step 3: Open source BMP file */
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname, "rb");

    if (decInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR: Unable to open file %s\n" RESET,
                decInfo->src_image_fname);
        return e_failure;
    }

    return e_success;
}

void decode_byte_from_lsb(char *buffer, char *c)
{
    /* Extract 1 byte from 8 image bytes */
    for (int i = 0; i < 8; i++)
    {
        *c |= (buffer[i] & 1) << (7 - i);
    }
}

void decode_size_from_lsb(char *buffer, int *size)
{
    /* Extract 32-bit size from 32 image bytes */
    for (int i = 0; i < 32; i++)
    {
        *size |= (buffer[i] & 1) << (31 - i);
    }
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    /* Step 4: Decode and verify magic string */
    char buffer[8];
    int size = strlen(magic_string);

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
    /* Step 5: Decode secret file extension size */
    char buffer[32];

    fread(buffer, 1, 32, decInfo->fptr_src_image);
    *extn_size = 0;
    decode_size_from_lsb(buffer, extn_size);

    return e_success;
}

Status decode_secret_file_extn(int extn_size, DecodeInfo *decInfo)
{
    /* Step 6: Decode secret file extension */
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
    /* Step 7: Create output file using decoded extension */
    strcat(decInfo->secret_fname, decInfo->extn_secret_file);

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");

    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR: Unable to create file %s\n" RESET,
                decInfo->secret_fname);
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    /* Step 8: Decode secret file size */
    char buffer[32];
    int size = 0;

    fread(buffer, 1, 32, decInfo->fptr_src_image);
    decode_size_from_lsb(buffer, &size);
    decInfo->size_secret_file = size;

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    /* Step 9: Decode secret file data */
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
    /* Step 3: Open source image */
    if (open_dec_files(decInfo) != e_success)
    {
        return e_failure;
    }

    /* Step 4: Skip BMP header */
    if (fseek(decInfo->fptr_src_image, 54, SEEK_SET) != 0)
    {
        fprintf(stderr, RED "ERROR: Failed to skip BMP header\n" RESET);
        fclose(decInfo->fptr_src_image);
        return e_failure;
    }

    /* Step 5: Decode and verify magic string */
    if (decode_magic_string(MAGIC_STRING, decInfo) != e_success)
    {
        fprintf(stderr, RED "ERROR: Magic string not found\n" RESET);
        fclose(decInfo->fptr_src_image);
        return e_failure;
    }

    /* Step 6: Decode extension size */
    int extn_size = 0;
    if (decode_secret_file_extn_size(decInfo, &extn_size) != e_success)
    {
        fprintf(stderr, RED "ERROR: Failed to decode extension size\n" RESET);
        fclose(decInfo->fptr_src_image);
        return e_failure;
    }

    /* Step 7: Decode extension */
    if (decode_secret_file_extn(extn_size, decInfo) != e_success)
    {
        fprintf(stderr, RED "ERROR: Failed to decode extension\n" RESET);
        fclose(decInfo->fptr_src_image);
        return e_failure;
    }

    /* Step 8: Create output file */
    if (open_dec_secret_file(decInfo) != e_success)
    {
        fclose(decInfo->fptr_src_image);
        return e_failure;
    }

    /* Step 9: Decode secret file size */
    if (decode_secret_file_size(decInfo) != e_success)
    {
        fprintf(stderr, RED "ERROR: Failed to decode file size\n" RESET);
        fclose(decInfo->fptr_src_image);
        fclose(decInfo->fptr_secret);
        return e_failure;
    }

    /* Step 10: Decode secret data */
    if (decode_secret_file_data(decInfo) != e_success)
    {
        fprintf(stderr, RED "ERROR: Failed to decode secret data\n" RESET);
        fclose(decInfo->fptr_src_image);
        fclose(decInfo->fptr_secret);
        return e_failure;
    }

    fclose(decInfo->fptr_src_image);
    fclose(decInfo->fptr_secret);

    return e_success;
}
