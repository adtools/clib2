/*
 * $Id: fgets_test.c,v 1.1.1.1 2004-07-26 16:36:07 obarthel Exp $
 *
 * :ts=4
 */

/****************************************************************************/

#include <string.h>
#include <stdio.h>

/****************************************************************************/

int
main(int argc,char ** argv)
{
	char line[256];
	size_t len;
	FILE * in;
	int i;

	for(i = 1 ; i < argc ; i++)
	{
		in = fopen(argv[i],"rb");
		if(in != NULL)
		{
			while(fgets(line,sizeof(line),in) != NULL)
			{
				len = strlen(line);
				while(len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
					line[--len] = '\0';

				printf("%s\n",line);
			}

			fclose(in);
		}
	}

	return(0);
}
