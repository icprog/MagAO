
#include <stdio.h>

int main() {
	
	char* uniqueName = tmpnam("aoapp_");
	printf("%s\n", uniqueName);
	
	return 0;
}