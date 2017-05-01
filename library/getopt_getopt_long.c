/*
 * $Id: getopt_getopt_long.c,v 1.4 2006-01-08 12:04:22 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

#include <getopt.h>

extern int		opterr;
extern int		optind;
extern int		optopt;
extern char *	optarg;
int optreset = 0;

#define GETOPTF_LONG_ONLY		(1L << 0)	/* Operate in getopt_long_only more */
#define GETOPTF_GNUEXT			(1L << 1)	/* Allow -W extension */
#define GETOPTF_POSIX			(1L << 2)	/* Will disallow permutation */
#define GETOPTF_ONE_ARG			(1L << 3)	/* Non-option argv will be handled as -\1 */

static int getopt_long_internal(int argc, const char **argv, const char *optstring,
		const struct option *longopts, int *longindex, int flags);


int getopt_long(int argc, const char **argv, const char *optstring,
		const struct option *longopts, int *longindex)
{
	return getopt_long_internal(argc, argv, optstring, longopts, longindex, 0);
}

#if 0
int getopt_long_only(int argc, const char **argv, const char *optstring,
		const struct option *longopts, int *longindex)
{
	return getopt_long_internal(argc, argv, optstring, longopts, longindex,
		GETOPTF_LONG_ONLY);
}
#endif


static int parse_long_option(int argc, char **argv, const struct option *longopts,
	int *longindex, int flags)
{
	char *current = argv[optind] + 2;	/* Skip the '--' */
	char *argument;
	int keylen = 0;
	int i;
	int idx = -1;
	int result = '?';

	(void)flags;

	optind ++;

	/* If there's an equals sign in the option, the actual word to check is
	 * between the -- and = characters. Otherwise, it's the length of the
	 * remainder (without --)
	 */
	argument = strchr(current, '=');
	if (argument)
	{
		keylen = (int)(argument - current);
		/* Skip the '=' sign */
		argument++;
	}
	else
		keylen = strlen(current);

	for (i = 0; longopts[i].name; i++)
	{
		if (strncmp(longopts[i].name, current, keylen))
			continue;

		if (strlen(longopts[i].name) == (size_t)keylen)
		{
			/* Exact match */
			idx = i;
			break;
		}

		/* Partial match. Check if we already found another partial match, in
		 * which case the match is ambigous.
		 */
		if (idx == -1)
		{
			idx = i;
		}
		else
		{
			*longindex = -1;
			return '?';
		}
	}

	*longindex = idx;

	if (idx != -1)
	{
		/* Found a match. Check if it needs an argument */
		if (longopts[idx].has_arg == optional_argument)
		{
			/* Optional arguments must be given as --foo=bar, it will not
			 * advance to the next argv element
			 */
			if (argument)
				optarg = argument;
			else
				optarg = 0;
		}
		else if (longopts[idx].has_arg == required_argument)
		{
			/* Either argument was given as --foo=bar, or the next argv element
			 * (if present) will be returned
			 */
			if (argument)
				optarg = argument;
			else
			{
				if (optind < argc)
				{
					optarg = argv[optind];
					optind++;
				}
				else
				{
					if (opterr)
						fprintf(stderr, "%s: option requires an argument -- %s\n",
							argv[0], longopts[idx].name);

					optarg = 0;

					return '?';
				}
			}
		}
		else
		{
			/* No argument */
			optarg = 0;

			if (argument)
			{
				if (opterr)
					fprintf(stderr, "%s: option does not take an argument -- %s\n",
						argv[0], longopts[idx].name);

				return '?';
			}
		}

		if (longopts[idx].flag)
		{
			*longopts[idx].flag = longopts[idx].val;
			optopt = 0;
			result = 0;
		}
		else
		{
			result = optopt = longopts[idx].val;
		}

		return result;
	}

	optopt = 0;

	return '?';
}



/* Note: This implementation currently does not permute argv vectors, and will
 * stop parsing input on the first non-option argument
 */

static int getopt_long_internal(int argc, const char **argv, const char *optstring,
		const struct option *longopts, int *longindex, int flags)
{
	static char *nextchar = 0;
	int result = EOF;
	int c;
	char *optp;
	int longidx_subst;

	ENTER();
	SHOWVALUE(argc);
	SHOWPOINTER(argv);
	SHOWSTRING(optstring);
	SHOWPOINTER(longopts);
	SHOWPOINTER(longindex);
	SHOWVALUE(flags);

	if(__check_abort_enabled)
		__check_abort();

	if (NULL == optstring)
		goto out;

	if (!longindex)
		longindex = &longidx_subst;

	/* Check options */
	/* FIXME: Should probably also check environment variable */
	if (*optstring == '+')
	{
		flags |= GETOPTF_POSIX;
		optstring++;
	}

	if (*optstring == '-')
	{
		flags |= GETOPTF_ONE_ARG;
		optstring++;
	}

	/* Check for reset */
	if (optreset || optind == 0)
	{
		optind = 1;
		optreset = 0;
		nextchar = 0;
	}

	if (optind >= argc)
		goto out;

	/* Begin or continue scanning
	 * First off, check for a short option and set nextchar, if present.
	 * if nextchar is not 0, there's a compound option waiting or just begun.
	 */
	if (!nextchar
		&& argv[optind][0] == '-'
		&& argv[optind][1] != '\0'
		&& argv[optind][1] != '-')
	{
		/* It's a short option. */
		nextchar = (char *)&argv[optind][1];
	}

	if (nextchar)
	{
		c = *nextchar ++;

		/* Advancing nextchar. If it points to a 0 character now, it's reached
		 * the end of the current argv string, so set it to 0 and advance optind
		 * to the next argv element.
		 */
		if (0 == *nextchar)
		{
			nextchar = 0;
			optind ++;
		}

		optp = strchr(optstring, c);

		/* We never find a long option in a compound option */
		*longindex = -1;

		/* Check if it's legal */
		if (c == ':' || (optp == NULL))
		{
			/* Illegal option */
			if (opterr)
				fprintf(stderr, "%s: illegal option -- %c\n", argv[0], c);

			result = '?';

			goto out;
		}
		else
		{
			/* It is legal. Three possibilities here:
			 * 1. It doesn't have arguments. Return the character.
			 * 2. It has an argument. Return the character, and point optarg
			 *    to the rest of the argv entry.
			 * 3. It has an optional argument. If there are non-0 characters
			 *    following, point optarg there, otherwise, clear optarg.
			 */
			if (optp[1] != ':')
			{
				/* Case 1 (no argument). */
				result = c;
				goto out;
			}
			else
			{
				/* Argument may follow */
				if (optp[2] == ':')
				{
					/* Optional argument. nextchar is 0 if there's no further
					 * stuff following, which means the option is without
					 * argument
					 */
					if (nextchar)
					{
						optarg = nextchar;
						nextchar = 0;
						optind++;
					}
					else
						optarg = 0;
				}
				else
				{
					/* Mandatory argument. Either nextchar points to the rest
					 * of this argv element, or we use the next argv element
					 */
					if (nextchar)
					{
						optarg = nextchar;
						nextchar = 0;
						optind++;
					}
					else
					{
						if (optind >= argc)
						{
							if (opterr)
								fprintf(stderr, "%s: option requires an argument -- %c\n",
									argv[0], c);

							result = '?';

							goto out;
						}

						optarg = (char *)argv[optind++];
					}
				}

				result = c;
				goto out;
			}
		}
	} /* if (nextchar) */

	/* Start checking an argv element for an option.
	 * There are different possibilities here:
	 * 1. We ran out of argv elements
	 * 2. We have an argv element that's just "-" or not an option
	 * 3. We have an argv element that's just "--"
	 * 4. we habe a long option (--xxx pr -W xxx)
	 */

	/* Check case 3 and 1*/
	if ((strcmp(argv[optind], "--") == 0) || optind >= argc)
	{
		result = EOF;
		goto out;
	}

	/* Check case 4 */
	if (argv[optind][0] == '-' && argv[optind][1] == '-' && longopts)
	{
		/* It is a long option */
		result = parse_long_option(argc, (char **)argv, longopts, longindex, flags);
		goto out;
	}

	/* Case 2 */


 out:
 	RETURN(result);
 	return result;
}
