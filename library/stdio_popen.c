/*
 * $Id: stdio_popen.c,v 1.7 2005-03-18 12:38:23 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
pclose(FILE *stream)
{
	int result = -1;

	ENTER();

	SHOWPOINTER(stream);

	assert(stream != NULL);

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL)
		{
			SHOWMSG("invalid stream parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	fclose(stream);

	/* ZZZ we actually could catch the program's termination code
	 * by passing an exit function address to SystemTags() below.
	 */
	result = 0;

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

FILE *
popen(const char *command, const char *type)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info command_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	char * command_copy = NULL;
	BPTR input = ZERO;
	BPTR output = ZERO;
	char pipe_file_name[40];
	FILE * result = NULL;
	LONG status;
	unsigned long task_address;
	time_t now;
	int i;

	ENTER();

	SHOWSTRING(command);
	SHOWSTRING(type);

	assert(command != NULL && type != NULL);

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(command == NULL || type == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	/* The first character selects the access mode: read or write. We don't
	   support anything else. */
	switch(type[0])
	{
		case 'r':

			SHOWMSG("read mode");
			break;

		case 'w':

			SHOWMSG("write mode");
			break;

		default:

			D(("unsupported access mode '%lc'",type[0]));

			__set_errno(EINVAL);
			goto out;
	}

	/* The current PIPE: device only supports unidirectional connections. */
	if((type[1] == '+') || (type[1] != '\0' && type[2] == '+'))
	{
		D(("unsupported access mode '%s'",type));

		__set_errno(EINVAL);
		goto out;
	}

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			char just_the_command_name[MAXPATHLEN+1];
			BOOL quotes_needed = FALSE;
			char * command_name;
			size_t command_len;
			BOOL have_quote;
			size_t len;

			/* We may need to replace the path specified for the command,
			   so let's figure out first how long the command name,
			   including everything, really is. */
			len = strlen(command);
			command_len = len;

			have_quote = FALSE;
			for(i = 0 ; i < (int)len ; i++)
			{
				if(command[i] == '\"')
				{
					quotes_needed = TRUE;

					have_quote ^= TRUE;
				}

				if((command[i] == ' ' || command[i] == '\t') && NOT have_quote)
				{
					command_len = i;
					break;
				}
			}

			/* This may be too long for proper translation... */
			if(command_len > MAXPATHLEN)
			{
				__set_errno(ENAMETOOLONG);

				result = NULL;
				goto out;
			}

			/* Grab the command name itself, then have it translated. */
			command_name = just_the_command_name;
			for(i = 0; (size_t)i < command_len; i++)
			{
				if(command[i] != '\"')
					(*command_name++) = command[i];
			}

			(*command_name) = '\0';

			command_name = just_the_command_name;

			if(__translate_unix_to_amiga_path_name((const char **)&command_name,&command_nti) != 0)
			{
				result = NULL;
				goto out;
			}

			/* Now put it all together again */
			command_copy = malloc(1 + strlen(command_name) + 1 + strlen(&command[command_len]) + 1);
			if(command_copy == NULL)
			{
				__set_errno(ENOMEM);

				result = NULL;
				goto out;
			}

			if(quotes_needed)
			{
				command_copy[0] = '\"';
				strcpy(&command_copy[1],command_name);
				strcat(command_copy,"\"");
			}
			else
			{
				strcpy(command_copy,command_name);
			}

			strcat(command_copy,&command[command_len]);

			command = command_copy;
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	/* Build a (hopefully) unique name for the pipe stream to open. We
	   construct it from the current process address, converted into
	   an octal number, followed by the current time (in seconds),
	   converted into another octal number. */
	strcpy(pipe_file_name,"PIPE:");

	task_address = (unsigned long)FindTask(NULL);

	for(i = strlen(pipe_file_name) ; task_address != 0 && i < (int)sizeof(pipe_file_name)-1 ; i++)
	{
		pipe_file_name[i] = '0' + (task_address % 8);
		task_address = task_address / 8;
	}

	pipe_file_name[i++] = '.';

	time(&now);

	for( ; now != 0 && i < (int)sizeof(pipe_file_name)-1 ; i++)
	{
		pipe_file_name[i] = '0' + (now % 8);
		now = now / 8;
	}

	pipe_file_name[i] = '\0';

	SHOWSTRING(pipe_file_name);

	PROFILE_OFF();

	/* Now open the input and output streams for the program to launch. */
	if(type[0] == 'r')
	{
		/* Read mode: we want to read the output of the program; the program
		   should read from "NIL:". */
		input = Open("NIL:",MODE_NEWFILE);
		if(input != ZERO)
			output = Open(pipe_file_name,MODE_NEWFILE);
	}
	else
	{
		/* Write mode: we want to send data to the program; the program
		   should write to "NIL:". */
		input = Open(pipe_file_name,MODE_NEWFILE);
		if(input != ZERO)
			output = Open("NIL:",MODE_NEWFILE);
	}

	PROFILE_ON();

	/* Check if both I/O streams could be opened. */
	if(input == ZERO || output == ZERO)
	{
		SHOWMSG("couldn't open the streams");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	PROFILE_OFF();

	/* Now try to launch the program. */
	status = SystemTags((STRPTR)command,
		SYS_Input,		input,
		SYS_Output,		output,
		SYS_Asynch,		TRUE,
		SYS_UserShell,	TRUE,
	TAG_END);

	PROFILE_ON();

	/* If launching the program returned -1 then it could not be started.
	   We'll need to close the I/O streams we opened above. */
	if(status == -1)
	{
		SHOWMSG("SystemTagList() failed");

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	/* OK, the program is running. Once it terminates, it will automatically
	   shut down the streams we opened for it. */
	input = output = ZERO;

	/* Now try to open the pipe we will use to exchange data with the program. */
	result = fopen(pipe_file_name,type);

 out:

	if(command_copy != NULL)
		free(command_copy);

	PROFILE_OFF();

	if(input != ZERO)
		Close(input);

	if(output != ZERO)
		Close(output);

	PROFILE_ON();

	RETURN(result);
	return(result);
}
