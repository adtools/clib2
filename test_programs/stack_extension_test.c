/* gcc -mstackextend -o stack_extension_test stack_extension_test.c */

#include <stdio.h>
#include <ctype.h>

void
recursive_function(char *data,int data_size,int level)
{
	char local_data[10000];
	char line[10];
	int c;

	data_size += sizeof(local_data);
	level++;

	printf("recursion level=%d, size=%d; continue? ",level,data_size);

	fgets(line,sizeof(line),stdin);
	c = toupper(line[0]);

	if(c == 'Y')
		recursive_function(local_data,data_size,level);
}

int
main(int argc,char ** argv)
{
	recursive_function(NULL,0,0);

	return(0);
}
