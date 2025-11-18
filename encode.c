#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"

// Function to get image size (width * height * 3 bytes per pixel)
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    // Move file pointer to 18th byte where width is stored
    fseek(fptr_image, 18, SEEK_SET);

    // Read width and height values from the BMP header
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);

    printf("width = %u\n", width);
    printf("height = %u\n", height);

    // Return total number of bytes available for pixel data
    return width * height * 3;
}

// Function to find file size in bytes
uint get_file_size(FILE *fptr)
{
    long size;
    fseek(fptr, 0, SEEK_END);    // Move pointer to end
    size = ftell(fptr);          // Get current position (size)
    rewind(fptr);                // Move pointer back to start
    return size;
}

// Function to validate and store filenames from command-line arguments
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Validate source image name (must be .bmp)
    if (strstr(argv[2], ".bmp") != NULL)
        encInfo->src_image_fname = argv[2];
    else
    {
        printf("ERROR: Source image file must be a .bmp file\n");
        return e_failure;
    }

    // Validate secret file type (.txt, .c, or .sh)
    if (strstr(argv[3], ".txt") || strstr(argv[3], ".c") || strstr(argv[3], ".sh"))
        encInfo->secret_fname = argv[3];
    else
    {
        printf("ERROR: Secret file must be a .txt, .c, or .sh file\n");
        return e_failure;
    }

    // Validate or assign default stego image filename
    if (argv[4] == NULL)
        encInfo->stego_image_fname = "default.bmp";
    else if (strstr(argv[4], ".bmp"))
        encInfo->stego_image_fname = argv[4];
    else
    {
        printf("ERROR: Output file must be a .bmp file\n");
        return e_failure;
    }

    return e_success;
}

// Function to open source, secret, and stego files
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");  // Open source image in binary read mode
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");        // Open secret file in binary read mode
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");  // Open output stego image in binary write mode
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

// Function to verify if image has enough capacity to hide data
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Ensure image can hold all required data (header + secret + metadata)
    if (encInfo->image_capacity > (16 + 32 + 32 + 32 + (encInfo->size_secret_file * 8)))
        return e_success;
    else
        return e_failure;
}

// Function to copy the BMP header (first 54 bytes)
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);  // Reset pointer to start of BMP

    char imageBuffer[54];    // Buffer for header data
    fread(imageBuffer, 54, 1, fptr_src_image);
    fwrite(imageBuffer, 54, 1, fptr_dest_image);

    if (ftell(fptr_src_image) == ftell(fptr_dest_image))
        return e_success;
    else
        return e_failure;
}

// Function to encode a predefined magic string into image
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char imageBuffer[8];

    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);          // Read 8 bytes from image
        encode_byte_to_lsb(magic_string[i], imageBuffer);           // Encode one character
        fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image);       // Write modified bytes
    }

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

// Function to encode size of secret file extension
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char imageBuffer[32];
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, imageBuffer);
    fwrite(imageBuffer, 32, 1, encInfo->fptr_stego_image);

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

// Function to encode secret file extension (.txt, .c, .sh)
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char imageBuffer[8];
    for (int i = 0; i < strlen(file_extn); i++)
    {
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], imageBuffer);
        fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image);
    }

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

// Function to encode size of the secret file (in bytes)
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char imageBuffer[32];
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, imageBuffer);
    fwrite(imageBuffer, 32, 1, encInfo->fptr_stego_image);

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

// Function to encode secret file data byte-by-byte
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char secret_data[encInfo->size_secret_file];   // Buffer to hold entire secret file
    rewind(encInfo->fptr_secret);                  // Reset secret file pointer
    fread(secret_data, encInfo->size_secret_file, 1, encInfo->fptr_secret);

    char imageBuffer[8];
    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);          // Read 8 image bytes
        encode_byte_to_lsb(secret_data[i], imageBuffer);            // Encode 1 byte of secret data
        fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image);       // Write modified image bytes
    }

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

// Function to copy remaining image data after encoding
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
        fwrite(&ch, 1, 1, fptr_dest);

    if (ftell(fptr_src) == ftell(fptr_dest))
        return e_success;
    else
        return e_failure;
}

// Function to encode 1 byte of data into 8 LSBs of image bytes
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] &= 0xFE;                      // Clear LSB
        char bit = (data >> (7 - i)) & 1;             // Extract corresponding bit
        image_buffer[i] |= bit;                       // Set LSB to data bit
    }
    return e_success;
}

// Function to encode 32-bit integer (size) into image bytes
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] &= 0xFE;
        char bit = (size >> (31 - i)) & 1;
        imageBuffer[i] |= bit;
    }
    return e_success;
}

// Function to perform full encoding operation sequence
Status do_encoding(EncodeInfo *encInfo)
{
    printf("Opening files\n");
    if (open_files(encInfo) == e_failure)
        return e_failure;

    printf("Checking capacity\n");
    if (check_capacity(encInfo) == e_failure)
        return e_failure;

    printf("Copying BMP header\n");
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        return e_failure;

    printf("Encoding magic string\n");
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
        return e_failure;

    // Get the secret file extension (e.g., .txt, .c, .sh)
    char *extn = strstr(encInfo->secret_fname, ".");
    int extn_size = strlen(extn);

    printf("Encoding file extension size\n");
    encode_secret_file_extn_size(extn_size, encInfo);

    printf("Encoding file extension\n");
    encode_secret_file_extn(extn, encInfo);

    printf("Encoding secret file size\n");
    encode_secret_file_size(encInfo->size_secret_file, encInfo);

    printf("Encoding secret file data\n");
    encode_secret_file_data(encInfo);

    printf("Copying remaining image data\n");
    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    printf("Encoding complete! Stego image saved as %s\n", encInfo->stego_image_fname);
    return e_success;
}
