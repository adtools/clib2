#include <stdio.h>

int __stack_size = 60000;

int
main(void)
{
	int first, second, num;

	num = sscanf("6", "%d %d", &first, &second);

	printf("%d %d\n", num, first);

	return(0);
}
