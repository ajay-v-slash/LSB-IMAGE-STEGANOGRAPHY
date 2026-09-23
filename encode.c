#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */
/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);   // must reset position - caller still needs to fread from the start later
    return size;
}
/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{ 
    // step 1 -> check argv[2] is having .bmp or not
        //yes -> store the file name into encInfo -> src_image_fname = argv[2]
        //no - > return e_failure

    // step 2 -> check argv[3] is having extn there or not
        // yes - > store the file name into encInfo -> secret_fname = argv[3]
        // no -> return e_failure
    
    // step 3 -> check argv[4] is NULL or not
        //yes -> check argv[4] is having .bmp or not
                //yes -> store the file name into encInfo -> stego_image_fname = argv[4]
                //no - > return e_failure
        //no -> store default name the encInfo -> stego_image_fname = "stego.bmp"
    // return e_success;
    char * c1= strrchr(argv[2],'.');
    if(c1 != NULL && strcmp(c1,".bmp")==0)
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    c1= strrchr(argv[3],'.');
    if(c1 != NULL && strcmp(c1,".txt")==0)
    {
        encInfo -> secret_fname = argv[3];
        strcpy(encInfo -> extn_secret_file, c1);
    }
    else
    {
        return e_failure;
    }
    if( argv[4]!=NULL)
    {
        c1 = strrchr(argv[4],'.');
        if(c1 != NULL && strcmp(c1,".bmp")==0)
        {
          encInfo -> stego_image_fname = argv[4];
        }
        else
        {
          return e_failure;
        }
    }
    else
    {
        encInfo -> stego_image_fname = "stego.bmp";
    }
    return e_success;
    
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo ->fptr_src_image);
    printf("Image Capacity : %u\n", encInfo -> image_capacity);
    encInfo -> size_secret_file =  get_file_size(encInfo -> fptr_secret);
    printf("Secret File Size : %lu\n",encInfo -> size_secret_file);
    if(encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }

    //step 1 -> encInfo -> image_capacity = call get_image_size_for_bmp(encInfo -> fptr_src_image)
    //step 2 -> encInfo -> size_secret_file = call get_file_size(encInfo -> fptr_secret)
    //step 3 -> check encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)
                //t -> return e_success;
                //f -> return e_failure
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
     // step 1 -> rewind source file pointer
    rewind(fptr_src_image);
    char buffer[54];
    // step 2 -> read 54 bytes from source file
    fread(buffer,1,54,fptr_src_image);
    // step 3 -> write 54 bytes to dest file
    fwrite(buffer,1,54,fptr_dest_image);
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    int size = strlen(magic_string);
    printf("Started Encoding Magic String !\n");
    for(int i=0;i<size;i++)
    {
        //step 1 -> read 8 bytes from source file
        fread(buffer,1,8,encInfo->fptr_src_image);
        //step 2 -> call encode_byte_to_lsb(magic_string[i], buffer)
        encode_byte_to_lsb(magic_string[i],buffer);
        //step 3 -> write the buffer into dest file
        fwrite(buffer,1,8,encInfo->fptr_stego_image);
    }
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
    {
        return e_success;
    }
    return e_failure;
    // repeat this for strlen(magic_string) times from step 1
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    // A 32-bit value needs exactly one 32-byte chunk (1 image byte per bit) -
    // no outer loop needed, that was re-encoding the same value repeatedly
    fread(buffer,1,32,encInfo->fptr_src_image);
    encode_size_to_lsb(size,buffer);
    fwrite(buffer,1,32,encInfo->fptr_stego_image);
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
    {
        return e_success;
    }
    return e_failure;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    int len = strlen(file_extn);
    for(int i=0;i<len;i++)
    {
    //step 1 -> read 8 bytes from source file
       fread(buffer,1,8,encInfo->fptr_src_image);
    //step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)
      encode_byte_to_lsb(file_extn[i], buffer);
    //step 3 -> write the buffer into dest file
      fwrite(buffer,1,8,encInfo->fptr_stego_image);
    }
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
    {
        return e_success;
    }
    return e_failure;
    
    // repeat this for strlen(file_extn) times from step 1   
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    // Same fix as encode_secret_file_extn_size: one 32-byte chunk, once.
    fread(buffer,1,32,encInfo->fptr_src_image);
    encode_size_to_lsb((int)file_size, buffer);
    fwrite(buffer,1,32,encInfo->fptr_stego_image);
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
    {
        return e_success;
    }
    return e_failure;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // Stream one byte at a time instead of fread-ing the whole secret file into
    // secret_data[100] up front - that buffer overflows for any secret file > 100 bytes.
    char ch;
    char buffer[8];
    for(long i=0;i<encInfo->size_secret_file;i++)
    {
      //step 1 -> read one byte from secret file
      fread(&ch, 1, 1, encInfo->fptr_secret);
      //step 2 -> read 8 bytes from source file
      fread(buffer,1,8,encInfo->fptr_src_image);
      //step 3 -> call encode_byte_to_lsb(ch, buffer)
      encode_byte_to_lsb(ch, buffer);
      //step 4 -> write the buffer into dest file
      fwrite(buffer,1,8,encInfo->fptr_stego_image);
    }
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
    {
        return e_success;
    }
    return e_failure;
    // repeat this for size_secret_file times from step 1
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // write logic for copy remaining data
    char ch;
    while(fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // write logic to encode the char
    for(int i=0;i<8;i++)
    {
        image_buffer[i]= ((image_buffer[i] &(~(1))) | (((unsigned char)data>>(7-i)) & 1));
    }
    return e_success;

}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // write logic to encode the int
    for(int i=0;i<32;i++)
    {
        imageBuffer[i]= ((imageBuffer[i] &(~(1))) | (((unsigned int)size>>(31-i)) & 1));
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
        //step 1 -> check open_files(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step

    //step 2 -> check check_capacity(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step

    //step 3 -> call copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step

    //step 4 -> call encode_magic_string(MAGIC_STRING, encInfo)
    //step 5 -> call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)

    //step 6 -> call encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)

    //step 7 -> call encode_secret_file_size(encInfo ->  size_secret_file, encInfo)

    //step 8 -> call encode_secret_file_data(encInfo)

    //step 9 -> copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success
    // return e_success
    if(open_files(encInfo)==e_success)
    {
        printf("File Opened successfully!\n");
    }
    else
    {
        printf("Error : Failed to open file !\n");
        return e_failure;
    }
    if(check_capacity(encInfo)==e_success)
    {
        printf("Capacity is Checked\n");
    }
    else
    {
        printf("Capacity is not enough!\n");
        return e_failure;
    }
    if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)==e_success)
    {
        printf("Copying of bmp Header is success\n");
    }
    else
    {
        printf("ERROR : Failed to Copy bmp header\n");
        return e_failure;
    }
    if(encode_magic_string(MAGIC_STRING, encInfo)==e_success)
    {
        printf("Magic String is Encoded\n");
    }
    else
    {
        printf("ERROR : Encoding Magic String is Failed!\n");
        return e_failure;
    }
    if(encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)==e_success)
    {
        printf("Secret File Extesnsion Size is Encoded\n");  
    }
    else
    {
        printf("ERROR : Encoding Secret File Extesnsion Size is Failed!\n");
        return e_failure;
    }
    if(encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)==e_success)
    {
        printf("Secret File Extension is Encoded\n");
    }
    else
    {
        printf("ERROR : Encoding Secret File Extension is Failed!\n");
        return e_failure;
    }
    if(encode_secret_file_size(encInfo ->  size_secret_file, encInfo)==e_success)
    {
        printf("Secret File Size is Encoded\n");  
    }
    else
    {
        printf("ERROR : Encoding Secret File Size is Failed!\n");
        return e_failure;
    }
    if(encode_secret_file_data(encInfo)==e_success)
    {
        printf("Secret File Data is Encoded\n");  
    }
    else
    {
        printf("ERROR : Encoding Secret Data is Failed!\n");
        return e_failure;
    }
    if(copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)==e_success)
    {
        printf("Remaining image data is copied successfully \n");  
    }
    else
    {
        printf("ERROR : Encoding Remaining image data is Failed!\n");
        return e_failure;
    }
    

    return e_success;
}