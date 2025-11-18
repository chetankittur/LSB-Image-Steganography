#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

// Magic string used to identify valid stego data
#define MAGIC_STRING "#*"

// Structure to store all decoding-related information
typedef struct _DecodeInfo
{
    char *stego_image_fname;   // Name of the input stego image
    char output_fname[260];    // Name of the output file to store decoded data
    char file_extn[10];        // File extension of the hidden secret file
    FILE *fptr_stego_image;    // File pointer to the stego image
    FILE *fptr_output;         // File pointer to the decoded output file
} DecodeInfo;

// Function to read and validate command-line arguments for decoding
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

// Function to open required files for decoding
Status open_decode_files(DecodeInfo *decInfo);

// Function to extract a single byte of data from 8 image bytes
char decode_byte_from_lsb(char *image_buffer);

// Function to decode a 32-bit integer value from image bytes
int decode_size_from_lsb(char *image_buffer);

// Function to decode and verify the magic string
Status decode_magic_string(DecodeInfo *decInfo);

// Function to decode size of the secret file extension
int decode_secret_file_extn_size(DecodeInfo *decInfo);

// Function to decode the actual file extension
Status decode_secret_file_extn(DecodeInfo *decInfo, int extn_size);

// Function to decode the total size of the secret file
long decode_secret_file_size(DecodeInfo *decInfo);

// Function to decode and write secret data to the output file
Status decode_secret_file_data(DecodeInfo *decInfo, long file_size);

// Function to perform the entire decoding process
Status do_decoding(DecodeInfo *decInfo);

#endif
