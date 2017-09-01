#include <stdio.h>

void main()
{
	char buffer[1024];
	float value;

	while( fgets( buffer, 1024, stdin))
		{
		sscanf(buffer, "%f", &value);

		printf("0x%08X\n", *( (int *)&value));
		}


}

