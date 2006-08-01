/*
 * $Id: unistd_execle.c,v 1.1 2006-08-01 14:27:52 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
execle(const char *path, const char *arg0, ...)
{
	char ** argv = NULL;
	int result = -1;
	size_t argc = 0;
	va_list args;
	char **envp;

	/* If there are any, count the number of arguments supplied */
	if(arg0 != NULL)
	{
		argc = 1;

		va_start(args,arg0);

		while(va_arg(args,const char *) != NULL)
			argc++;

		va_end(args);
	}

	/* Allocate enough memory for the entire argument list
	   plus the terminating NULL */
	argv = (char **)malloc(sizeof(*argv) * (argc + 1));
	if(argv == NULL)
	{
		__set_errno(ENOMEM);
		goto out;
	}

	if(argc > 0)
	{
		size_t i;

		argv[0] = arg0;

		va_start(args,arg0);

		for(i = 1 ; i < argc ; i++)
			argv[i] = va_arg(args,const char *);

		/* Skip the NULL terminator */
		(void)va_arg(args, char **);
		envp = va_arg(args, char **);

		va_end(args);
	}
	else
	{
		va_start(args,arg0);

		envp = va_arg(args, char **);

		va_end(args);
	}

	argv[argc] = NULL;

	result = execve(path,(char * const *)argv, envp );

 out:

	if(argv != NULL)
		free(argv);

	return(result);
}
