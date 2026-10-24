#include <stdio.h>
#include "encode.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    EncodeInfo encodeInfo;

    if( argc == 1 )
    {
	// print -> Error + usage msg
	return 0;
    }

    Opr_type opr = check_operation(argv[1]);

    if (opr == encode)
    {
        if (argc < 4 || argc > 5)
        {
            printf("Error: Invalid arguments for encoding.\n");
            printf("Usage: ./steganography -e <source.bmp> <secret_file.txt> [output.bmp]\n");
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
    
    else if( opr == decode )
    {
	if( argc < 3 || argc > 4 )	
	{
	    // print -> Error + usage msg
	    return 0;
	}
    }
    else
    {
	// Print -> Error + usage msg
    }
    return 0;
}
