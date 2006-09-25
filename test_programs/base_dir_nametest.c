/*
 * $Id: base_dir_nametest.c,v 1.1 2006-09-25 18:20:09 obarthel Exp $
 *
 * :ts=4
 */

#include <unistd.h>
#include <libgen.h>
#include <stdio.h>

int
main(int argc,char ** argv)
{
	int i;

	for(i = 1 ; i < argc ; i++)
		printf("%s\n\tbasename=%s\n\t dirname=%s\n",argv[i],basename(argv[i]),dirname(argv[i]));

	return(0);
}
