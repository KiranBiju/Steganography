#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "common.h"

/* Check operation type */
Opr_type check_operation(char *option)
{
    if (option == NULL)
        return unsupported;

    if (strcmp(option, "-e") == 0)
        return encode;

    if (strcmp(option, "-d") == 0)
        return decode;

    return unsupported;
}

/* Validate command-line arguments for encoding */
Status validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ext;
    int valid_ext = 0;

    if (argv[2] == NULL || argv[3] == NULL)
        return failure;

    /* Validate source image extension */
    ext = strrchr(argv[2], '.');

    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        printf("Error: Source image must be a .bmp file\n");
        return failure;
    }

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];

    /* Validate secret file extension */
    ext = strrchr(argv[3], '.');

    if (ext == NULL)
    {
        printf("Error: Secret file must have a valid extension\n");
        return failure;
    }

    if (strcmp(ext, ".txt") == 0 ||
        strcmp(ext, ".c") == 0 ||
        strcmp(ext, ".sh") == 0 ||
        strcmp(ext, ".cpp") == 0 ||
        strcmp(ext, ".py") == 0)
    {
        valid_ext = 1;
    }

    if (!valid_ext || strlen(ext) >= sizeof(encInfo->secret_extn))
    {
        printf("Error: Unsupported secret file extension\n");
        return failure;
    }

    strcpy(encInfo->secret_extn, ext);
    encInfo->secret_extn_size = strlen(encInfo->secret_extn);

    /* Validate output image filename */
    if (argv[4] == NULL)
    {
        encInfo->output_image_fname = "stego.bmp";
    }
    else
    {
        ext = strrchr(argv[4], '.');

        if (ext == NULL || strcmp(ext, ".bmp") != 0)
        {
            printf("Error: Output image must have .bmp extension\n");
            return failure;
        }

        encInfo->output_image_fname = argv[4];
    }

    return success;
}

/* Get image size: width * height * 3 for 24-bit BMP */
unsigned int get_image_size_for_bmp(FILE *fptr_image)
{
    unsigned int width, height;

    fseek(fptr_image, 18, SEEK_SET);

    if (fread(&width, sizeof(width), 1, fptr_image) != 1)
        return 0;

    if (fread(&height, sizeof(height), 1, fptr_image) != 1)
        return 0;

    return width * height * 3;
}

/* Get secret file size */
unsigned int get_file_size(FILE *fptr)
{
    long size;

    if (fseek(fptr, 0, SEEK_END) != 0)
        return 0;

    size = ftell(fptr);

    if (size < 0)
        return 0;

    rewind(fptr);

    return (unsigned int)size;
}

/* Open source, secret and output files */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->src_image_fptr =
        fopen(encInfo->src_image_fname, "rb");

    if (encInfo->src_image_fptr == NULL)
    {
        perror("Source image");
        return failure;
    }

    encInfo->secret_fptr =
        fopen(encInfo->secret_fname, "rb");

    if (encInfo->secret_fptr == NULL)
    {
        perror("Secret file");
        return failure;
    }

    encInfo->output_image_fptr =
        fopen(encInfo->output_image_fname, "wb");

    if (encInfo->output_image_fptr == NULL)
    {
        perror("Output image");
        return failure;
    }

    return success;
}

/* Check whether the image has enough capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    unsigned int img_capacity;
    unsigned long long encoding_bits;

    img_capacity =
        get_image_size_for_bmp(encInfo->src_image_fptr);

    if (img_capacity == 0)
        return failure;

    encInfo->secret_file_size =
        get_file_size(encInfo->secret_fptr);

    /*
     * Required bits:
     * Magic string + extension size + extension
     * + secret file size + secret file data.
     */
    encoding_bits =
        (strlen(MAGIC_STRING) +
         sizeof(int) +
         encInfo->secret_extn_size +
         sizeof(int) +
         encInfo->secret_file_size) * 8ULL;

    printf("Image capacity: %u bytes\n", img_capacity);
    printf("Required encoding bits: %llu\n", encoding_bits);

    /*
     * Each image byte stores one secret bit.
     * Therefore, compare image bytes with required bits.
     */
    if (img_capacity >= encoding_bits)
        return success;

    printf("Error: Insufficient image capacity\n");
    return failure;
}

/* Copy the 54-byte BMP header */
Status copy_bmp_header(EncodeInfo *encInfo)
{
    unsigned char header[54];

    rewind(encInfo->src_image_fptr);

    if (fread(header, 1, sizeof(header), encInfo->src_image_fptr) != sizeof(header))
    {
        return failure;
    }

    if (fwrite(header, 1, sizeof(header), encInfo->output_image_fptr) != sizeof(header))
    {
        return failure;
    }

    return success;
}

/* Encode one byte into the LSBs of eight image bytes */
Status encode_1byte_to_lsb(char data, char *buffer_8)
{
    int i;
    unsigned char value = (unsigned char)data;

    for (i = 0; i < 8; i++)
    {
        buffer_8[i] = buffer_8[i] & ~(1);

        int bit = (value >> (7 - i)) & 1;

        buffer_8[i] = buffer_8[i] | bit;
    }

    return success;
}

/* Encode a 4-byte integer into 32 image bytes */
Status encode_4byte_to_lsb(int data, char *buffer_32)
{
    int i;
    unsigned int value = (unsigned int)data;

    for (i = 0; i < 4; i++)
    {
        unsigned char byte = (value >> (24 - (i * 8))) & 0xFF;

        if (encode_1byte_to_lsb(byte, &buffer_32[i * 8]) == failure)
        {
            return failure;
        }
    }

    return success;
}

/* Encode the magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];
    int i;

    for(i = 0; magic_string[i] != '\0'; i++)
    {
        if (fread(image_buffer, 1, 8, encInfo->src_image_fptr) != 8)
        {
            return failure;
        }

        if (encode_1byte_to_lsb(magic_string[i], image_buffer) == failure)
        {
            return failure;
        }

        if (fwrite(image_buffer, 1, 8, encInfo->output_image_fptr) != 8)
        {
            return failure;
        }
    }

    return success;
}

/* Encode the secret file extension size and extension */
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    char image_buffer[32];
    char buffer_8[8];
    int i;

    /* Encode extension size */
    if (fread(image_buffer, 1, 32, encInfo->src_image_fptr) != 32)
    {
        return failure;
    }

    if (encode_4byte_to_lsb(encInfo->secret_extn_size, image_buffer) == failure)
    {
        return failure;
    }

    if (fwrite(image_buffer, 1, 32, encInfo->output_image_fptr) != 32)
    {
        return failure;
    }

    /* Encode extension characters */
    for (i = 0; i < encInfo->secret_extn_size; i++)
    {
        if (fread(buffer_8, 1, 8, encInfo->src_image_fptr) != 8)
        {
            return failure;
        }

        if (encode_1byte_to_lsb(encInfo->secret_extn[i], buffer_8) == failure)
        {
            return failure;
        }

        if (fwrite(buffer_8, 1, 8, encInfo->output_image_fptr) != 8)
        {
            return failure;
        }
    }

    return success;
}

/* Encode secret file size and secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char image_buffer[32];
    char buffer_8[8];
    int i;
    int ch;

    /* Encode secret file size */
    if (fread(image_buffer, 1, 32, encInfo->src_image_fptr) != 32)
    {
        return failure;
    }

    if (encode_4byte_to_lsb( (int)encInfo->secret_file_size, image_buffer) == failure)
    {
        return failure;
    }

    if (fwrite(image_buffer, 1, 32, encInfo->output_image_fptr) != 32)
    {
        return failure;
    }

    /* Encode secret file contents */
    for (i = 0; i < encInfo->secret_file_size; i++)
    {
        ch = fgetc(encInfo->secret_fptr);

        if (ch == EOF)
            return failure;

        if (fread(buffer_8, 1, 8,
                  encInfo->src_image_fptr) != 8)
        {
            return failure;
        }

        if (encode_1byte_to_lsb((char)ch, buffer_8) == failure)
        {
            return failure;
        }

        if (fwrite(buffer_8, 1, 8, encInfo->output_image_fptr) != 8)
        {
            return failure;
        }
    }

    return success;
}

/* Copy all remaining image bytes */
Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    int ch;

    while ((ch = fgetc(encInfo->src_image_fptr)) != EOF)
    {
        if (fputc(ch, encInfo->output_image_fptr) == EOF)
            return failure;
    }

    if (ferror(encInfo->src_image_fptr))
        return failure;

    return success;
}

/* Perform the complete encoding process */
Status do_encoding(char *argv[], EncodeInfo *encInfo)
{
    Status status = failure;

    /* Initialize file pointers */
    encInfo->src_image_fptr = NULL;
    encInfo->secret_fptr = NULL;
    encInfo->output_image_fptr = NULL;

    if (validate_encode_args(argv, encInfo) == failure)
        goto reset;

    if (open_files(encInfo) == failure)
        goto reset;

    if (check_capacity(encInfo) == failure)
        goto reset;

    if (copy_bmp_header(encInfo) == failure)
        goto reset;

    if (encode_magic_string(MAGIC_STRING, encInfo) == failure)
        goto reset;

    if (encode_secret_file_extn(encInfo) == failure)
        goto reset;

    if (encode_secret_file_data(encInfo) == failure)
        goto reset;

    if (copy_remaining_img_data(encInfo) == failure)
        goto reset;

    status = success;

reset:

    if (encInfo->src_image_fptr != NULL)
        fclose(encInfo->src_image_fptr);

    if (encInfo->secret_fptr != NULL)
        fclose(encInfo->secret_fptr);

    if (encInfo->output_image_fptr != NULL)
        fclose(encInfo->output_image_fptr);

    if (status == success)
        printf("Encoding completed successfully!\n");
    else
        printf("Encoding failed!\n");

    return status;
}