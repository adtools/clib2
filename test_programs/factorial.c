#include <stdio.h>

/*
gcc -std=c99 -mcrt=clib2 -o factorial factorial.c -lm
output is:
16! = 20922789888 <- Wrong

gcc -std=c99 -mcrt=newlib -o factorial factorial.c
output is:
16! = 20922789888000 <- Correct
*/

int
main(void)
{
	double result = 1.0;
	int i;

	for ( i = 2; i < 17; ++i )
		result = result * i;

	printf("16! = %.14g\n", result);

	return 0;
}
