#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "decode.h"

/* Validate the input BMP and optional output filename */
Status validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ext;

    if (argv[2] == NULL)
    {
        printf("Error: Encoded image filename is missing.\n");
        return failure;
    }

    ext = strrchr(argv[2], '.');

    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        printf("Error: Encoded image must have .bmp extension.\n");
        return failure;
    }

    decInfo->stego_image_fname = argv[2];

    /* Empty means the output filename will be generated later */
    decInfo->output_secret_fname[0] = '\0';

    if (argv[3] != NULL)
    {
        if (strlen(argv[3]) >= sizeof(decInfo->output_secret_fname))
        {
            printf("Error: Output filename is too long.\n");
            return failure;
        }

        strcpy(decInfo->output_secret_fname, argv[3]);
    }

    decInfo->stego_image_fptr = NULL;
    decInfo->output_secret_fptr = NULL;
    decInfo->secret_extn_size = 0;
    decInfo->secret_file_size = 0;

    return success;
}


/* Open the encoded BMP file */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->stego_image_fptr =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->stego_image_fptr == NULL)
    {
        perror("Encoded image");
        return failure;
    }

    return success;
}


/* Skip the conventional 54-byte BMP header */
Status skip_bmp_header(DecodeInfo *decInfo)
{
    if (fseek(decInfo->stego_image_fptr, 54, SEEK_SET) != 0)
    {
        printf("Error: Unable to skip BMP header.\n");
        return failure;
    }

    return success;
}


/* Extract one byte from the LSBs of 8 image bytes */
char decode_1byte_from_lsb(char *image_buffer)
{
    unsigned char data = 0;
    int i;

    for (i = 0; i < 8; i++)
    {
        data = (unsigned char)
               ((data << 1) |
                ((unsigned char)image_buffer[i] & 1U));
    }

    return (char)data;
}


/* Extract a four-byte integer from 32 image bytes */
int decode_4byte_from_lsb(char *image_buffer)
{
    unsigned int data = 0;
    int i;

    for (i = 0; i < 4; i++)
    {
        unsigned char byte =
            (unsigned char)decode_1byte_from_lsb(
                &image_buffer[i * 8]
            );

        data = (data << 8) | byte;
    }

    return (int)data;
}


/* Extract and verify the magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic_string[sizeof(MAGIC_STRING)];
    int i;

    for (i = 0; i < (int)strlen(MAGIC_STRING); i++)
    {
        if (fread(image_buffer, 1, 8,
                  decInfo->stego_image_fptr) != 8)
        {
            printf("Error: Unable to read magic string.\n");
            return failure;
        }

        magic_string[i] = decode_1byte_from_lsb(image_buffer);
    }

    magic_string[i] = '\0';

    if (strcmp(magic_string, MAGIC_STRING) != 0)
    {
        printf("Error: Magic string does not match.\n");
        printf("The image may not contain encoded data.\n");
        return failure;
    }

    printf("Magic string verified successfully.\n");

    return success;
}


/* Extract the extension length and extension */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[32];
    char buffer_8[8];
    unsigned int extn_size;
    unsigned int i;

    /* Read 32 image bytes containing the extension length */
    if (fread(image_buffer, 1, 32,
              decInfo->stego_image_fptr) != 32)
    {
        printf("Error: Unable to read extension length.\n");
        return failure;
    }

    extn_size = (unsigned int)
                decode_4byte_from_lsb(image_buffer);

    /* secret_extn has room for 6 characters plus '\0' */
    if (extn_size == 0 ||
        extn_size >= sizeof(decInfo->secret_extn))
    {
        printf("Error: Invalid secret file extension length.\n");
        return failure;
    }

    decInfo->secret_extn_size = (int)extn_size;

    /* Decode each extension character */
    for (i = 0; i < extn_size; i++)
    {
        if (fread(buffer_8, 1, 8,
                  decInfo->stego_image_fptr) != 8)
        {
            printf("Error: Unable to read file extension.\n");
            return failure;
        }

        decInfo->secret_extn[i] =
            decode_1byte_from_lsb(buffer_8);
    }

    decInfo->secret_extn[extn_size] = '\0';

    printf("Recovered extension: %s\n",
           decInfo->secret_extn);

    /*
     * If the user did not supply an output filename,
     * generate one using the recovered extension.
     */
    if (decInfo->output_secret_fname[0] == '\0')
    {
        int written = snprintf(
            decInfo->output_secret_fname,
            sizeof(decInfo->output_secret_fname),
            "decoded%s",
            decInfo->secret_extn
        );

        if (written < 0 ||
            (size_t)written >=
            sizeof(decInfo->output_secret_fname))
        {
            printf("Error: Generated output filename is too long.\n");
            return failure;
        }
    }

    /* Open the recovered secret file in binary write mode */
    decInfo->output_secret_fptr =
        fopen(decInfo->output_secret_fname, "wb");

    if (decInfo->output_secret_fptr == NULL)
    {
        perror("Output secret file");
        return failure;
    }

    return success;
}


/* Extract the secret-file size and recover its contents */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[32];
    char buffer_8[8];
    unsigned int file_size;
    unsigned int i;

    /* Read 32 image bytes containing the secret-file size */
    if (fread(image_buffer, 1, 32,
              decInfo->stego_image_fptr) != 32)
    {
        printf("Error: Unable to read secret file size.\n");
        return failure;
    }

    {
        int decoded_size = decode_4byte_from_lsb(image_buffer);

        if (decoded_size < 0)
        {
            printf("Error: Invalid secret file size.\n");
            return failure;
        }

        file_size = (unsigned int)decoded_size;
    }

    decInfo->secret_file_size = file_size;

    printf("Recovered secret file size: %u bytes\n",
           decInfo->secret_file_size);

    /* Recover one secret byte for every 8 image bytes */
    for (i = 0; i < file_size; i++)
    {
        unsigned char secret_byte;

        if (fread(buffer_8, 1, 8,
                  decInfo->stego_image_fptr) != 8)
        {
            printf("Error: Image ended before secret data was recovered.\n");
            return failure;
        }

        secret_byte =
            (unsigned char)decode_1byte_from_lsb(buffer_8);

        if (fputc(secret_byte, decInfo->output_secret_fptr) == EOF)
        {
            printf("Error: Unable to write recovered secret data.\n");
            return failure;
        }
    }

    if (ferror(decInfo->stego_image_fptr))
    {
        printf("Error: Failed while reading encoded image.\n");
        return failure;
    }

    if (ferror(decInfo->output_secret_fptr))
    {
        printf("Error: Failed while writing secret file.\n");
        return failure;
    }

    return success;
}


/* Coordinate the complete decoding process */
Status do_decoding(char *argv[], DecodeInfo *decInfo)
{
    Status status = failure;

    decInfo->stego_image_fptr = NULL;
    decInfo->output_secret_fptr = NULL;

    if (validate_decode_args(argv, decInfo) == failure)
        goto cleanup;

    if (open_decode_files(decInfo) == failure)
        goto cleanup;

    if (skip_bmp_header(decInfo) == failure)
        goto cleanup;

    if (decode_magic_string(decInfo) == failure)
        goto cleanup;

    if (decode_secret_file_extn(decInfo) == failure)
        goto cleanup;

    if (decode_secret_file_data(decInfo) == failure)
        goto cleanup;

    status = success;

cleanup:

    if (decInfo->stego_image_fptr != NULL)
    {
        fclose(decInfo->stego_image_fptr);
        decInfo->stego_image_fptr = NULL;
    }

    if (decInfo->output_secret_fptr != NULL)
    {
        if (fclose(decInfo->output_secret_fptr) != 0)
            status = failure;

        decInfo->output_secret_fptr = NULL;
    }

    if (status == success)
    {
        printf("Decoding completed successfully!\n");
        printf("Recovered file: %s\n",
               decInfo->output_secret_fname);
    }
    else
    {
        printf("Decoding failed!\n");
    }

    return status;
}
