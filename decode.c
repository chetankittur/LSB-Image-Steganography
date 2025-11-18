#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"

// Function to read and validate decode arguments
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Validate the stego image file
    if (argv[2] && strstr(argv[2], ".bmp"))
        decInfo->stego_image_fname = argv[2];
    else
    {
        printf("ERROR: Stego image file must be a .bmp file\n");
        return e_failure;
    }

    // Store the output filename (or assign a default name)
    if (argv[3] == NULL)
        strcpy(decInfo->output_fname, "decoded_output");
    else
    {
        strncpy(decInfo->output_fname, argv[3], sizeof(decInfo->output_fname) - 1);
        decInfo->output_fname[sizeof(decInfo->output_fname) - 1] = '\0';
    }

    return e_success;
}

// Function to open the stego image file
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open stego image file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    return e_success;
}
// Function to extract a single byte of data from 8 image bytes
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    for (int i = 0; i < 8; i++)
    {
        data = data << 1;
        data |= (image_buffer[i] & 1);
    }
    return data;
}

// Function to decode an integer (32 bits) from image bytes
int decode_size_from_lsb(char *image_buffer)
{
    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        size = size << 1;
        size |= (image_buffer[i] & 1);
    }
    return size;
}

// Function to decode and verify the magic string
Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic_string[10];
    int i;

    for (i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        magic_string[i] = decode_byte_from_lsb(image_buffer);
    }
    magic_string[i] = '\0';

    if (strcmp(magic_string, MAGIC_STRING) == 0)
    {
        printf("Magic string matched successfully\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Magic string mismatch\n");
        return e_failure;
    }
}

// Function to decode size of the secret file extension
int decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);
    return decode_size_from_lsb(image_buffer);
}

// Function to decode the secret file extension (.txt, .c, .sh)
Status decode_secret_file_extn(DecodeInfo *decInfo, int extn_size)
{
    char image_buffer[8];
    for (int i = 0; i < extn_size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        decInfo->file_extn[i] = decode_byte_from_lsb(image_buffer);
    }
    decInfo->file_extn[extn_size] = '\0';
    printf("File extension decoded: %s\n", decInfo->file_extn);
    return e_success;
}

// Function to decode size of the secret file data
long decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32];
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);
    return decode_size_from_lsb(image_buffer);
}

// Function to decode the hidden secret file data
Status decode_secret_file_data(DecodeInfo *decInfo, long file_size)
{
    char image_buffer[8];
    char ch;

    for (long i = 0; i < file_size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        ch = decode_byte_from_lsb(image_buffer);
        fputc(ch, decInfo->fptr_output);
    }

    printf("Secret data decoded successfully\n");
    return e_success;
}

// Function to perform all decoding operations
Status do_decoding(DecodeInfo *decInfo)
{
    printf("Decoding started\n");

    // Open stego image in binary mode
    if (open_decode_files(decInfo) == e_failure)
        return e_failure;

    // Skip BMP header
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    // Decode and verify magic string
    if (decode_magic_string(decInfo) == e_failure)
        return e_failure;

    // Decode extension size and extension
    int extn_size = decode_secret_file_extn_size(decInfo);
    if (decode_secret_file_extn(decInfo, extn_size) == e_failure)
        return e_failure;

    // Adjust output filename by removing incorrect extension and adding decoded one
    char *dot = strrchr(decInfo->output_fname, '.');
    if (dot != NULL)
        *dot = '\0';
    strcat(decInfo->output_fname, decInfo->file_extn);

    printf("Corrected output filename -> %s\n", decInfo->output_fname);

    // Open output file in binary write mode
    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open output file %s\n", decInfo->output_fname);
        return e_failure;
    }

    // Decode file size and actual data
    long file_size = decode_secret_file_size(decInfo);
    printf("File size decoded: %ld bytes\n", file_size);

    if (decode_secret_file_data(decInfo, file_size) == e_failure)
        return e_failure;

    printf("Decoding completed successfully.\n");

    // Close open files
    fclose(decInfo->fptr_output);
    fclose(decInfo->fptr_stego_image);

    return e_success;
}
