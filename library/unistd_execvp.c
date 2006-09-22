/*
 * $Id: unistd_execvp.c,v 1.6 2006-09-22 09:02:51 obarthel Exp $
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

/* A local version of the BSD strsep() function. */
static char *
local_strsep(char ** string_ptr, const char * delimiter)
{
	char * result = NULL;
	char * string;

	assert( string_ptr != NULL && delimiter != NULL );

	string = (*string_ptr);
	if(string != NULL)
	{
		char * token = string;
		char c;

		while(TRUE)
		{
			c = (*string++);

			/* Reached the end of the string? */
			if(c == '\0')
			{
				/* No further string data available. */
				(*string_ptr) = NULL;

				result = token;
				break;
			}

			/* Is that character a delimiter? */
			if(strchr(delimiter,c) != NULL)
			{
				/* NUL-terminate the string, overwriting
				   the delimiter character */
				string[-1] = '\0';

				/* Scanning can resume with the next following
				   character. */
				(*string_ptr) = string;

				result = token;
				break;
			}
		}
	}

	return(result);
}

/****************************************************************************/

int
execvp(const char *command,char * const argv[])
{
	char * command_buffer = NULL;
	size_t command_name_len,i;
	char * path_copy = NULL;
	int result = -1;
	BOOL found_path_separators;

	/* Do not allow null command */
	if(command == NULL || (*command) == '\0')
	{
		__set_errno(ENOENT);
		goto out;
	}

	command_name_len = strlen(command);

	/* Check if there are any path separator characters in the
	   command name. */
	found_path_separators = FALSE;

	for(i = 0 ; i < command_name_len ; i++)
	{
		if(command[i] == '/' || command[i] == ':')
		{
			found_path_separators = TRUE;
			break;
		}
	}

	/* If it's an absolute or relative path name, it's easy. */
	if(found_path_separators)
	{
		result = execve(command, argv, environ);
	}
	else
	{
		size_t command_buffer_size = 0;
		char * path_delimiter;
		char * path;
		char * search_prefix;
		size_t search_prefix_len;
		size_t complete_path_len;
		int error;

		/* We first look up the PATH environment variable because
		   we will be making a copy of it. This avoids trouble
		   lateron when we will be calling getenv() again. */
		path = getenv("PATH");
		if(path == NULL)
			path = __default_path;

		path_copy = strdup(path);
		if(path_copy == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}

		path = path_copy;

		path_delimiter = getenv("PATH_SEPARATOR");
		if(path_delimiter == NULL)
			path_delimiter = __default_path_delimiter;

		while((search_prefix = local_strsep(&path,path_delimiter)) != NULL)
		{
			if((*search_prefix) == '\0')
				search_prefix = (char *)".";

			search_prefix_len = strlen(search_prefix);

			complete_path_len = search_prefix_len + 1 + command_name_len;
			if(complete_path_len + 1 > command_buffer_size)
			{
				char * new_command_buffer;

				/* Allocate a little more memory than we
				   really need. */
				new_command_buffer = malloc(complete_path_len + 10);
				if(new_command_buffer == NULL)
				{
					__set_errno(ENOMEM);
					goto out;
				}

				if(command_buffer != NULL)
					free(command_buffer);

				command_buffer		= new_command_buffer;
				command_buffer_size	= complete_path_len + 10;
			}

			/* Combine the search prefix with the command name. */
			memcpy(command_buffer,search_prefix,search_prefix_len);
			command_buffer[search_prefix_len] = '/';
			memcpy(&command_buffer[search_prefix_len+1],command,command_name_len);
			command_buffer[complete_path_len] = '\0';

			/* Now try to run that command. */
			result = execve(command_buffer, argv, environ);

			/* Did it work? And if it didn't work, did it fail because
			   the command to be run could not be executed? */
			error = __get_errno();

			if(result == 0 ||
			   (error != EACCES &&
			    error != EISDIR &&
			    error != ENOENT &&
			    error != ENOEXEC &&
			    error != EPERM))
			{
				break;
			}

			/* Just in case somebody wants to quit... */
			__check_abort();
		}
	}

 out:

	if(path_copy != NULL)
		free(path_copy);

	if(command_buffer != NULL)
		free(command_buffer);

	return(result);
}
