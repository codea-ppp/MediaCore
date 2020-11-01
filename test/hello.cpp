#include <stdio.h>
#include <stdlib.h>

#include "Common/MessageFormat.h"

int main(int argc, char* argv[]) 
{
	using namespace MediaCoreMessageFormat;
	
	StreamMessage m;
	SetTransactionId(&m, 44);

	printf("hello, world\n");
	return 0;
}
