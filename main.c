#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

OperationType check_operation_type(char *);
void display_menu()
{
    printf("Encode Mode -e source.bmp secret.txt <Optional output filename >\n");
    printf("Decode Mode -d  source.bmp <Optional output filename>\n");

}
int main(int argc, char *argv[])
{
    //step 1 -> check check_operation_type(argv[1]) is returning e_encode or not
        // yes -> EncodeInfo encInfo;
            // check read_and_validate_encode_args(argv, &encInfo) is returning e_success or e_failure
                // failure -> print error msg and stop
                // success -> print success msg 
                            // check do_encoding(&encInfo) is returning e_success or e_failure
                                    // failure -> print error msg and stop
                                    // success -> print success msg and stop
    if(argc==1)
    {
        display_menu();
        return 0;
    }
    if(argc == 2)
    {
        display_menu();
        return 0;
    }
    if(check_operation_type(argv[1])==e_encode)
    {
         
         if(argc==3)
         {
            display_menu();
            return 0;
        }
        printf("Operation : ENCODING  \n");
        printf("Validating to source file format...\n");
        EncodeInfo encInfo;
        if(read_and_validate_encode_args(argv, &encInfo)==e_success)
        {
            printf("Source file format found\n");
            printf("Start Encoding..\n");
            if(do_encoding(&encInfo)==e_success)
            {
                printf("Encoding done successfully ");
                
            }
            else 
            {
                printf("Encoding is not done");
                return 0;
            }
        }
        else
        {
            printf("Encode file format is not found\n");
            return 0;
        }
    }
    else if(check_operation_type(argv[1])==e_decode)
    {
        printf("Operation : DECODING  \n");
        printf("Validating to source file format...\n");
        DecodeInfo decInfo;

        if(read_and_validate_decode_args(argc,argv, &decInfo)==e_success)
        {
            printf("Source file format found\n");
            printf("Start Decoding..\n");
            if(do_decoding(&decInfo)==e_success)
            {
                printf("Decoding done successfully ");
            }
            else
            {
                printf("Decoding is not done");
                return 0;
            }
        }
        else
        {
            printf("Decode file format is not found\n");
            return 0;
        }
    }

   
}


OperationType check_operation_type(char *symbol)
{
    //step 1 -> check symbol is -e or not
        // yes -> return e_encode
    
    // step 2 -> check symbol is -d or not
        // yes -> return e_decode
        
    // return e_unsupported
    if(strcmp(symbol,"-e")==0)
    {
        return e_encode;
    }
    if(strcmp(symbol,"-d")==0)
    {
        return e_decode;
    }
    return e_unsupported;
}