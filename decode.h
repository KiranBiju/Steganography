#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "common.h"

/* Structure to store decoding information */
typedef struct _DecodeInfo
{
    /* Encoded image details */
    char *stego_image_fname;
    FILE *stego_image_fptr;

    /* Recovered secret file details */
    char output_secret_fname[256];
    FILE *output_secret_fptr;

    /* Metadata extracted from the image */
    char secret_extn[7];
    int secret_extn_size;
    unsigned int secret_file_size;

} DecodeInfo;


/* Validate command-line arguments */
Status validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open the encoded image */
Status open_files(DecodeInfo *decInfo);

/* Skip the BMP header */
Status skip_bmp_header(DecodeInfo *decInfo);

/* Extract and verify the magic string */
Status decode_magic_string(DecodeInfo *decInfo);

/* Extract the secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Extract the secret file size and contents */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Extract one byte from the LSBs of 8 image bytes */
char decode_1byte_from_lsb(char *image_buffer);

/* Extract a four-byte integer from the LSBs */
int decode_4byte_from_lsb(char *image_buffer);

/* Coordinate the complete decoding process */
Status do_decoding(char *argv[], DecodeInfo *decInfo);

#endif
