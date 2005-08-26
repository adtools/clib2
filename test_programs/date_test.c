#include <time.h>
#include <stdio.h>
#include <locale.h>

int
main(void)
{
	time_t tm;
	struct tm *ptr;
	char str[60];

	setlocale(LC_ALL, "C");

	tm = time(NULL);
	ptr = localtime(&tm);

	strftime(str, sizeof(str), "%x\n", ptr);

	printf(str);

	return 0;
}
