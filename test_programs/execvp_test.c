/*
 * $Id: execvp_test.c,v 1.1 2006-08-02 09:22:38 obarthel Exp $
 *
 * :ts=4
 */

#include <unistd.h>

int
main(int argc,char ** argv)
{
	if(argc > 1)
		execvp(argv[1],&argv[2]);

	return(0);
}
