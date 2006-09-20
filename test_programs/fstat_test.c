/*
 * $Id: fstat_test.c,v 1.1 2006-09-20 19:46:37 obarthel Exp $
 *
 * :ts=4
 */

#include <sys/stat.h>

#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

/****************************************************************************/

int
main(int argc,char ** argv)
{
	struct stat st;
	int fd;

	if(argc <= 1)
		return(EXIT_FAILURE);

	fd = open(argv[1],O_RDONLY);
	if(fd == -1)
	{
		perror("open");
		return(EXIT_FAILURE);
	}

	if(fstat(fd,&st) == -1)
	{
		perror("fstat");
		return(EXIT_FAILURE);
	}

	return(EXIT_SUCCESS);
}
