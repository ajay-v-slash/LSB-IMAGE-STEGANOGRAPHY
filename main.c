#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "types.h"
#include "decode.h"
#include "common.h"

OperationType check_operation_type(char *symbol);

void display_menu(void)
{
    printf("\n" CYAN "%s\n" RESET, LINE);
    printf(CYAN "              STEGANOGRAPHY TOOL\n" RESET);
    printf("%s\n", LINE);
    printf("\n" WHITE "Usage:\n" RESET);
    printf("  " GREEN "Encode" RESET " : ./stego -e source.bmp secret.txt [output.bmp]\n");
    printf("  " GREEN "Decode" RESET " : ./stego -d stego.bmp [output_name]\n");
    printf("\n" DIM "Example:\n" RESET);
    printf("  ./stego -e beautiful.bmp secret.txt stego.bmp\n");
    printf("  ./stego -d stego.bmp output\n");
    printf("\n" CYAN "%s\n\n" RESET, LINE);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        display_menu();
        return 0;
    }

    OperationType operation = check_operation_type(argv[1]);

    if (operation == e_encode)
    {
        EncodeInfo encInfo;

        printf("\n" BLUE "%s\n" RESET, LINE);
        printf(BLUE "                    ENCODING MODE\n" RESET);
        printf("%s\n", LINE);

        if (argc < 4)
        {
            fprintf(stderr, RED "ERROR: Insufficient arguments for encoding.\n" RESET);
            display_menu();
            return 1;
        }

        printf(" Validating input files...   ");
        if (read_and_validate_encode_args(argv, &encInfo) != e_success)
        {
            printf(RED "FAILED\n" RESET);
            return 1;
        }
        printf(GREEN "OK\n" RESET);

        printf("Encoding secret data...  \n ");
        if (do_encoding(&encInfo) != e_success)
        {
            printf(RED "FAILED\n" RESET);
            return 1;
        }
        printf(GREEN "DONE\n" RESET);

        printf("\n" GREEN "SUCCESS: Encoding completed successfully.\n" RESET);
        printf("%s\n\n", THIN);
    }
    else if (operation == e_decode)
    {
        DecodeInfo decInfo;

        printf("\n" BLUE "%s\n" RESET, LINE);
        printf(BLUE "                    DECODING MODE\n" RESET);
        printf("%s\n", LINE);

        if (read_and_validate_decode_args(argc, argv, &decInfo) != e_success)
        {
            fprintf(stderr, RED "ERROR: Invalid decode arguments.\n" RESET);
            return 1;
        }

        printf("1.Validating source image... ");
        printf(GREEN "OK\n" RESET);

        printf("2.Decoding secret data...   ");
        if (do_decoding(&decInfo) != e_success)
        {
            printf(RED "FAILED\n" RESET);
            return 1;
        }
        printf(GREEN "DONE\n" RESET);

        printf("\n" GREEN "SUCCESS: Decoding completed successfully.\n" RESET);
        printf("  Output file : " CYAN "%s\n" RESET, decInfo.secret_fname);
        printf("  File size   : " CYAN "%ld bytes\n" RESET, decInfo.size_secret_file);
        printf("%s\n\n", THIN);
    }
    else
    {
        fprintf(stderr, RED "ERROR: Unsupported operation '%s'\n" RESET, argv[1]);
        display_menu();
        return 1;
    }

    return 0;
}

OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }

    if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }

    return e_unsupported;
}
