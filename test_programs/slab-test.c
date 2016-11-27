#include <stdlib.h>
#include <stdio.h>

unsigned long __slab_max_size = 4096;

static int print_json(void * ignore,const char * buffer,size_t len)
{
	fputs(buffer, stdout);

	return(0);
}

int
main(int argc,char ** argv)
{
	int i;

	srand(1);

	for(i = 0 ; i < 1000 ; i++)
		malloc(1 + (rand() %  8192));

	__get_slab_stats(NULL, print_json);

	return(0);
}
