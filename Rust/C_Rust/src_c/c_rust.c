#include <stdio.h>

extern void rust_func();

int main()
{
	int i;

	for(i=0;i<10;i++)
	{
		printf("Hello from C !\n");

		rust_func();
	}

    return 0;
}

