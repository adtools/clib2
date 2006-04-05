#include <stdio.h>
#include <locale.h>

/* uncomment to fix corruption
#include <dos.h>
BOOL __open_locale = FALSE;
*/

int
main(void)
{
	char *loc;
	time_t t;
	struct tm *m;
	char buf[200];
	int len;

	loc = setlocale(LC_ALL, "");
	printf("loc=%p (%s)\n", loc, loc);

	time(&t);
	m = gmtime(&t);

	len = strftime(buf, sizeof(buf), "%a %b", m);

	printf("len=%d buf=%s\n", len, buf);

	return 0;
}
