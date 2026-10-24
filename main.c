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

    int opr = check_operation(argv[1]);

    if( opr == encode )
    {
	if( argc < 4 || argc > 5 )
	{
	    // print -> Error + usage msg
	    return 0;
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
