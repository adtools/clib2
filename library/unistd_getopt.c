/*
 * $Id: unistd_getopt.c,v 1.3 2005-01-02 09:07:19 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int		opterr = 1;
int		optind = 1;
int		optopt;
char *	optarg;

/****************************************************************************/

int
getopt(int argc, char * argv[], char *opts)
{
	static int sp = 1;
	int result = EOF;
	char *cp;
	int c;

	ENTER();

	SHOWVALUE(argc);
	SHOWPOINTER(argv);
	SHOWSTRING(opts);

	assert( argc > 0 && argv != NULL && opts != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(argv == NULL || opts == NULL)
		{
			SHOWMSG("invalid parameters");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

	SHOWVALUE(optind);

	if(sp == 1)
	{
		if(optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
		{
			goto out;
		}
		else if (strcmp(argv[optind], "--") == SAME)
		{
			optind++;

			goto out;
		}
	}

	optopt = c = argv[optind][sp];

	if(c == ':' || (cp = strchr(opts, c)) == NULL)
	{
		if(opterr != 0)
			fprintf(stderr, "%s%s%c\n", argv[0], ": illegal option -- ", c);

		if(argv[optind][++sp] == '\0')
		{
			optind++;

			sp = 1;
		}

		result = '?';
		goto out;
	}

	if(*++cp == ':')
	{
		if(argv[optind][sp+1] != '\0')
		{
			optarg = &argv[optind++][sp+1];
		}
		else if (++optind >= argc)
		{
			if(opterr != 0)
				fprintf(stderr, "%s%s%c\n", argv[0], ": option requires an argument -- ", c);

			sp = 1;

			result = '?';
			goto out;
		}
		else
		{
			optarg = argv[optind++];
		}

		sp = 1;
	}
	else
	{
		if(argv[optind][++sp] == '\0')
		{
			sp = 1;

			optind++;
		}

		optarg = NULL;
	}

	result = c;

 out:

	RETURN(result);
	return(result);
}
