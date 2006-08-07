/*
 * $Id: execvp_test.c,v 1.2 2006-08-07 08:39:56 obarthel Exp $
 *
 * :ts=4
 */

#include <unistd.h>

int
main(int argc,char ** argv)
{
	if(argc > 1)
		execvp(argv[1],&argv[1]);

	return(0);
}
