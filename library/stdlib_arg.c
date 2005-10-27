/*
 * $Id: stdlib_arg.c,v 1.9 2005-10-27 08:58:41 obarthel Exp $
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

/*#define DEBUG*/

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/****************************************************************************/

char **	NOCOMMON __argv;
int		NOCOMMON __argc;

/****************************************************************************/

STATIC BOOL
is_space(unsigned char c)
{
	BOOL result;

	result = (BOOL)(c == '\t' 					||	/* horizontal tab */
	                c == '\r' 					||	/* carriage return */
	                c == '\n' 					||	/* line feed */
	                c == '\v' 					||	/* vertical tab */
	                c == '\f' 					||	/* form feed */
	                c == ' '  					||	/* blank space */
	                c == (unsigned char)'\240');	/* non-breaking space */

	return(result);
}

/****************************************************************************/

STATIC BOOL
is_escape_character(unsigned char c)
{
	BOOL result;

	result = (BOOL)(c == '*' || c == __shell_escape_character);

	return(result);
}

/****************************************************************************/

STATIC BOOL
is_final_quote_character(const unsigned char * str)
{
	BOOL result;

	result = (BOOL)(str[0] == '\"' && (str[1] == '\0' || is_space(str[1])));

	return(result);
}

/****************************************************************************/

ARG_CONSTRUCTOR(arg_init)
{
	BOOL success = FALSE;

	ENTER();

	/* Shell startup? */
	if(__WBenchMsg == NULL)
	{
		size_t			number_of_arguments;
		unsigned char *	arg_str;
		size_t			arg_len;
		unsigned char *	command_line;
		unsigned char *	str;

		/* Get the shell parameter string and find out
		   how long it is, stripping a trailing line
		   feed and blank spaces if necessary. */
		arg_str = GetArgStr();

		while(is_space(*arg_str))
			arg_str++;

		arg_len = strlen((char *)arg_str);

		while(arg_len > 0 && is_space(arg_str[arg_len - 1]))
			arg_len--;

		/* Make a copy of the shell parameter string. */
		command_line = malloc(arg_len + 1);
		if(command_line == NULL)
			goto out;

		memmove(command_line,arg_str,arg_len);
		command_line[arg_len] = '\0';

		/* If we have a valid command line string and a command
		   name, proceed to set up the argument vector. */
		str = command_line;

		/* Count the number of arguments. */
		number_of_arguments = 1;

		while(TRUE)
		{
			/* Skip leading blank space. */
			while((*str) != '\0' && is_space(*str))
				str++;

			if((*str) == '\0')
				break;

			number_of_arguments++;

			/* Quoted parameter starts here? */
			if((*str) == '\"')
			{
				str++;

				/* Skip the quoted string. */
				while((*str) != '\0' && NOT is_final_quote_character(str))
				{
					/* Escape character? */
					if(is_escape_character(*str))
					{
						str++;

						if((*str) != '\0')
							str++;
					}
					else
					{
						str++;
					}
				}

				/* Skip the closing quote. */
				if((*str) != '\0')
					str++;
			}
			else
			{
				/* Skip the parameter. */
				while((*str) != '\0' && NOT is_space(*str))
					str++;

				if((*str) == '\0')
					break;
			}
		}

		/* Put all this together into an argument vector.
		   We allocate one extra slot to put a NULL pointer
		   into. */
		__argv = (char **)malloc((number_of_arguments + 1) * sizeof(*__argv));
		if(__argv == NULL)
			goto out;

		/* The first parameter is the program name. */
		__argv[0] = __program_name;

		str = command_line;

		__argc = 1;

		while(TRUE)
		{
			/* Skip leading blank space. */
			while((*str) != '\0' && is_space(*str))
				str++;

			if((*str) == '\0')
				break;

			/* Quoted parameter starts here? */
			if((*str) == '\"')
			{
				char * arg;

				#if defined(UNIX_PATH_SEMANTICS)
				{
					/* If necessary, indicate that this parameter was quoted. */
					if(__wildcard_quote_parameter(__argc) < 0)
						goto out;
				}
				#endif /* UNIX_PATH_SEMANTICS */

				str++;

				__argv[__argc++] = str;

				arg = str;

				/* Process the quoted string. */
				while((*str) != '\0' && NOT is_final_quote_character(str))
				{
					if(is_escape_character(*str))
					{
						str++;

						switch(*str)
						{
							/* "*e" == "\033" */
							case 'E':
							case 'e':

								(*arg++) = '\033';
								break;

							/* "*n" == "\n" */
							case 'N':
							case 'n':

								(*arg++) = '\n';
								break;

							case '\0':

								/* NOTE: the termination is handled down below. */
								break;

							default:

								(*arg++) = (*str);
								break;
						}

						if((*str) != '\0')
							str++;
					}
					else
					{
						(*arg++) = (*str++);
					}
				}

				/* Skip the terminating quote. */
				if((*str) != '\0')
					str++;

				/* Make sure that the quoted string is properly terminated. This
				   actually overwrites the final quote character. */
				(*arg) = '\0';
			}
			else
			{
				__argv[__argc++] = str;

				while((*str) != '\0' && NOT is_space(*str))
					str++;

				if((*str) == '\0')
					break;

				(*str++) = '\0';
			}
		}

		assert( __argc == (int)number_of_arguments );
		assert( str <= &command_line[arg_len] );

		__argv[__argc] = NULL;

		#if defined(UNIX_PATH_SEMANTICS)
		{
			/* If necessary, expand wildcard patterns found in the command
			   line string into file and directory names. */
			if(__wildcard_expand_init() < 0)
				goto out;
		}
		#endif /* UNIX_PATH_SEMANTICS */
	}
	else
	{
		/* Return a pointer to the startup message in place of the
		   the argument vector. The argument counter (what will come
		   out as 'argc' for the main() function) will remain 0. */
		__argv = (char **)__WBenchMsg;
	}

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}
