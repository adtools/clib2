/*
 * $Id: stdio_popen.c,v 1.1.1.1 2004-07-26 16:31:38 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

int
pclose(FILE *stream)
{
	int result = -1;

	ENTER();

	SHOWPOINTER(stream);

	assert(stream != NULL);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL)
		{
			SHOWMSG("invalid stream parameter");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

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

#define SET_TAG(t,v,d)					\
	do									\
	{									\
		(t)->ti_Tag		= (Tag)(v);		\
		(t)->ti_Data	= (ULONG)(d);	\
		(t)++;							\
	}									\
	while(0)

#define END_TAG(t) \
	(void)((t)->ti_Tag = TAG_END)

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
	struct TagItem tags[5];
	struct TagItem * tag;
	FILE * result = NULL;
	LONG status;
	unsigned long task_address;
	time_t now;
	size_t type_len;
	char actual_type[8];
	int i;

	ENTER();

	SHOWSTRING(command);
	SHOWSTRING(type);

	assert(command != NULL && type != NULL);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(command == NULL || type == NULL)
		{
			SHOWMSG("invalid parameters");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

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
				errno = ENAMETOOLONG;
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
				errno = ENOMEM;
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

	/* Skip any options following the comma. */
	type_len = strlen(type);
	for(i = 0 ; i < (int)type_len ; i++)
	{
		if(type[i] == ',')
		{
			type_len = i;
			break;
		}
	}

	/* Keep only the first few letters of the type string. */
	if(type_len > sizeof(actual_type)-1)
		type_len = sizeof(actual_type)-1;

	memmove(actual_type,type,type_len);
	actual_type[type_len] = '\0';

	/* The current PIPE: device only supports unidirectional connections. */
	if(strcmp(actual_type,"r+") == SAME)
	{
		SHOWMSG("unsupported access mode");

		errno = EINVAL;
		goto out;
	}

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

	if(strcmp(actual_type,"r") == SAME)
	{
		input = Open("NIL:",MODE_NEWFILE);
		if(input != ZERO)
			output = Open(pipe_file_name,MODE_NEWFILE);
	}
	else if (strcmp(actual_type,"w") == SAME)
	{
		input = Open(pipe_file_name,MODE_NEWFILE);
		if(input != ZERO)
			output = Open("NIL:",MODE_NEWFILE);
	}
	else
	{
		errno = EINVAL;
		goto out;
	}

	PROFILE_ON();

	if(input == ZERO || output == ZERO)
	{
		SHOWMSG("couldn't open the streams");

		__translate_io_error_to_errno(IoErr(),&errno);
		goto out;
	}

	tag = tags;

	SET_TAG(tag,SYS_Input,		input);
	SET_TAG(tag,SYS_Output,		output);
	SET_TAG(tag,SYS_Asynch,		TRUE);
	SET_TAG(tag,SYS_UserShell,	TRUE);
	END_TAG(tag);

	PROFILE_OFF();
	status = SystemTagList((STRPTR)command,tags);
	PROFILE_ON();

	if(status == -1)
	{
		SHOWMSG("SystemTagList() failed");

		__translate_io_error_to_errno(IoErr(),&errno);
		goto out;
	}

	input = output = ZERO;

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
