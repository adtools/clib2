#include <stdlib.h>
#include <stdio.h>

int
main(int argc,char **argv)
{
	int n = 10,i,x;

	if(argc > 1)
		n = atoi(argv[1]);

	for(i = 0 ; i < n ; i++)
	{
		x = rand();

		printf("0x%08lx %d\n",x,x);
	}

	return(0);
}
