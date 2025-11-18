/*Name: Chetan Kittur
  Date:10/11/2025
  Project name: LSB Image Steganography
  Description: The project titled “LSB Image Steganography” focuses on securely hiding secret data within an image file using 
               the Least Significant Bit (LSB) technique. In this project, a secret file such as a text file (.txt), .c file (.c),
               or shell script (.sh) is embedded inside a BMP image without noticeably changing its visual appearance. The encoding 
               process reads the image pixel data and replaces the least significant bits with bits from the secret file to produce
               a stego image. The decoding process then extracts these hidden bits to reconstruct the original secret file with 
               complete accuracy. To ensure data authenticity, a unique magic string (“#*”) is used to identify the presence of 
               hidden information. This project demonstrates the practical application of data security and information hiding 
               techniques using C programming, emphasizing file handling, bitwise operations, and efficient use of image data for
               secure communication.
*/
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
// Function prototype to identify the operation type (-e or -d)
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    // Check that the program has enough arguments for encoding or decoding
    if (argc >= 3)
    {
        // If the operation selected is encoding
        if (check_operation_type(argv[1]) == e_encode)
        {
            // Verify minimum argument count for encoding
            if (argc < 4)
            {
                printf("Usage:\n");
                printf("For Encoding: ./a.out -e <input.bmp> <secret.txt/.c/.sh> [output.bmp]\n");
                printf("For Decoding: ./a.out -d <stego.bmp> [output.txt]\n");
                return 1;
            }

            // Create structure to hold encoding-related information
            EncodeInfo encInfo;

            // Validate encoding input arguments
            if (read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                // Perform the encoding process
                if (do_encoding(&encInfo) == e_success)
                    printf("Encoding Successful.\n");
                else
                    printf("Encoding Failed.\n");
            }
            else
            {
                printf("Validation Failed.\n");
                return e_failure;
            }
        }
        // If the operation selected is decoding
        else if (check_operation_type(argv[1]) == e_decode)
        {
            // Create structure to hold decoding-related information
            DecodeInfo decInfo;

            // Validate and assign decoding arguments
            if (argv[2] != NULL)
            {
                decInfo.stego_image_fname = argv[2];  // Input stego image filename

                // Copy user-provided output filename or use a default name
                if (argv[3] != NULL)
                    strcpy(decInfo.output_fname, argv[3]);
                else
                    strcpy(decInfo.output_fname, "decoded_output");

                // Perform the decoding process
                if (do_decoding(&decInfo) == e_success)
                    printf("Decoding Successful.\n");
                else
                    printf("Decoding Failed.\n");
            }
            else
            {
                // Handle missing arguments for decoding
                printf("ERROR: Missing arguments for decoding.\n");
                printf("Usage: ./a.out -d <stego.bmp> [output.txt]\n");
                return e_failure;
            }
        }
        // Handle unsupported or invalid operation type
        else
        {
            printf("Unsupported operation type.\n");
        }
    }
    else
    {
        // Display correct usage instructions when insufficient arguments are given
        printf("Usage:\n");
        printf("For Encoding: ./a.out -e <input.bmp> <secret.txt/.c/.sh> [output.bmp]\n");
        printf("For Decoding: ./a.out -d <stego.bmp> [output.txt]\n");
    }
    return 0;
}
// Function to identify the operation type based on user input (-e or -d)
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
        return e_encode;      // Encoding mode
    else if (strcmp(symbol, "-d") == 0)
        return e_decode;      // Decoding mode
    else
        return e_unsupported; // Invalid operation
}
