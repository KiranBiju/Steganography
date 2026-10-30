
#include <stdio.h>
#include "encode.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    EncodeInfo encodeInfo;
    DecodeInfo decodeInfo;

    if (argc == 1)
    {
        printf("Error: Missing command-line arguments.\n");
        printf("Usage:\n");
        printf("Encoding: ./a.out -e <source.bmp> <secret_file> [output.bmp]\n");
        printf("Decoding: ./a.out -d <stego.bmp> [output_secret_file]\n");
        return 1;
    }

    Opr_type opr = check_operation(argv[1]);

    if (opr == encode)
    {
        if (argc < 4 || argc > 5)
        {
            printf("Error: Invalid encoding arguments.\n");
            printf("Usage: ./a.out -e <source.bmp> <secret_file> [output.bmp]\n");
            return 1;
        }

        if (do_encoding(argv, &encodeInfo) == success)
        {
            printf("Encoding completed successfully.\n");
            return 0;
        }
        else
        {
            printf("Error: Encoding failed.\n");
            return 1;
        }
    }
    else if (opr == decode)
    {
        if (argc < 3 || argc > 4)
        {
            printf("Error: Invalid decoding arguments.\n");
            printf("Usage: ./a.out -d <stego.bmp> [output_secret_file]\n");
            return 1;
        }

        if (do_decoding(argv, &decodeInfo) == success)
        {
            printf("Decoding completed successfully.\n");
            return 0;
        }
        else
        {
            printf("Error: Decoding failed.\n");
            return 1;
        }
    }
    else
    {
        printf("Error: Unsupported operation '%s'.\n", argv[1]);
        printf("Use -e for encoding or -d for decoding.\n");
        printf("Encoding: ./a.out -e <source.bmp> <secret_file> [output.bmp]\n");
        printf("Decoding: ./a.out -d <stego.bmp> [output_secret_file]\n");
        return 1;
    }
}