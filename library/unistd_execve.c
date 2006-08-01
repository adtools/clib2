/*
 * $Id: unistd_execve.c,v 1.1 2006-08-01 14:27:52 obarthel Exp $
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

/* Try to find a file, given its full path name. Since it's possible that
   the path name may reference volumes, devices or assignments which are
   not currently valid we'll turn off DOS requesters while looking. */
STATIC int
find_file_and_parent(
	char *					path,
	BPTR *					parent_lock_ptr,
	struct FileInfoBlock *	fib)
{
	struct name_translation_info nti;
	APTR old_window_ptr;
	BPTR file_lock = ZERO;
	int result = -1;
	int error;

	(*parent_lock_ptr) = ZERO;

	old_window_ptr = __set_process_window((APTR)-1);

	error = __translate_unix_to_amiga_path_name(&path,&nti);
	if(error != 0)
	{
		__set_errno(error);
		goto out;
	}

	/* ZZZ we ought to walk down the assignment list all on our
	   own rather than trusting the Lock() to find the right
	   kind of file */
	file_lock = Lock(path,SHARED_LOCK);
	if(file_lock == ZERO)
	{
		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	if(CANNOT Examine(file_lock,fib))
	{
		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	/* This must be a file. */
	if(fib->fib_DirEntryType >= 0)
	{
		__set_errno(EISDIR);
		goto out;
	}

	/* And it should be executable. */
	if(fib->fib_Protection & FIBF_EXECUTE)
	{
		__set_errno(ENOEXEC);
		goto out;
	}

	(*parent_lock_ptr) = ParentDir(file_lock);
	if((*parent_lock_ptr) == ZERO)
	{
		LONG io_error;

		io_error = IoErr();
		if(io_error == 0)
			io_error = ERROR_OBJECT_WRONG_TYPE;

		__set_errno(__translate_io_error_to_errno(io_err));
		goto out;
	}

	result = 0;

 out:

	if(file_lock != ZERO)
		UnLock(file_lock);

	if(result != 0)
	{
		if((*parent_lock_ptr) != ZERO)
		{
			UnLock(*parent_lock_ptr);
			(*parent_lock_ptr) = ZERO;
		}
	}

	__set_process_window(old_window_ptr);

	return(result);
}

/****************************************************************************/

static size_t
count_extra_escape_chars(const char * string,size_t len)
{
	size_t count = 0;
	size_t i;
	char c;

	for(i = 0 ; i < len ; i++)
	{
		c = (*s++);
		if(c == '\"' || c == '*' || c == '\n')
			count++;
	}

	return(count);
}

/****************************************************************************/

STATIC BOOL
string_needs_escaping(const char * string,size_t len)
{
	BOOL result = FALSE;
	size_t i;
	char c;

	for(i = 0 ; i < len ; i++)
	{
		c = (*s++);
		if(c == ' ' || ((unsigned char)c) == 0xA0 || c == '\t' || c == '\n' || c == '\"')
		{
			result = TRUE;
			break;
		}
	}

	return(result);
}

/****************************************************************************/

static size_t
get_arg_string_length(char *const argv[])
{
	size_t result = 0;
	size_t i,len;
	char * s;

	for(i = 0 ; argv[i] != NULL ; i++)
	{
		s = (char *)argv[i];

		len = strlen(s);
		if(len > 0)
		{
			if((*s) != '\"')
			{
				if(string_needs_escaping(s,len))
					len += 1 + count_extra_escape_chars(s,len) + 1;
			}

			if(result == 0)
				result = len;
			else
				result = result + 1 + len;
		}
	}

	return(result);
}

/****************************************************************************/

static void
build_arg_string(char *const argv[],char * arg_string)
{
	BOOL first_char = FALSE;
	size_t i,j,len;
	char * s;

	for(i = 0 ; argv[i] != NULL ; i++)
	{
		s = (char *)argv[i];

		len = strlen(s);
		if(len > 0)
		{
			if(first_char)
				first_char = FALSE;
			else
				(*arg_string++) = ' ';

			if((*s) != '\"' && string_needs_escaping(s,len))
			{
				(*arg_string++) = '\"';

				for(j = 0 ; j < len ; j++)
				{
					if(s[j] == '\"' || s[j] == '*' || s[j] == '\n')
						(*arg_string++) = '*';

					(*arg_string++) = s[j];
				}

				(*arg_string++) = '\"';
			}
			else
			{
				memcpy(arg_string,s,len);
				arg_string += len;
			}
		}
	}
}

/****************************************************************************/

int
execve(const char *path, char *const argv[], char *const envp[])
{
	struct Process * this_process = (struct Process *)FindTask(NULL);
	D_S(struct FileInfoBlock,fib);
	char old_program_name[256]
	BPTR parent_dir = ZERO;
	BPTR old_dir;
	BPTR script_file = ZERO;
	int result = -1;
	char * interpreter_line = NULL;
	size_t interpreter_line_size = 0;
	size_t interpreter_line_length = 0;
	char * interpreter_name = NULL;
	char * interpreter_args = NULL;
	char * arg_string = NULL;
	size_t arg_string_len = 0;
	size_t parameter_string_len;
	BPTR segment_list = ZERO;
	BOOL success = FALSE;
	int error;
	LONG rc;
	LONG c;

	/* We begin by looking at the file or command to be run. */
	if(find_file_and_parent(path,&parent_dir,fib) != 0)
		goto out;

	/* Now open that file again so that we can check if it's
	   a script file. */
	old_dir = CurrentDir(parent_dir);
	script_file = Open(fib->fib_FileName,MODE_OLDFILE);
	CurrentDir(old_dir);

	if(script_file == ZERO)
	{
		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	SetVBuf(script_file,NULL,BUF_LINE,1024);

	/* Check if the first line begins with "#!" and if so,
	   read what else can be found in that line. */
	c = FGetC(script_file);
	if(c == '#')
	{
		c = FGetC(script_file);
		if(c == '!')
		{
			/* Skip leading blank spaces. */
			do
			{
				c = FGetC(script_file);
				if(c == -1)
					break;
			}
			while(isspace(c));

			if(c != -1)
			{
				/* Read everything that follows. */
				while((c = FGetC(script_file)) != -1)
				{
					/* Still enough room in the buffer? We always reserve
					   enough memory for the next character and a NUL
					   to terminate the string with. */
					if(interpreter_line_length + 2 > interpreter_line_size)
					{
						char * new_interpreter_line;

						/* Always reserve a little more memory than needed,
						   and one extra byte to allow us to to NUL-terminate
						   the string. */
						new_interpreter_line = realloc(interpreter_line,interpreter_line_length + 10);
						if(new_interpreter_line == NULL)
						{
							__set_error(ENOMEM);
							goto out;
						}

						interpreter_line		= new_interpreter_line;
						interpreter_line_size	= interpreter_line_length + 10;
					}

					interpreter_line[interpreter_line_length++] = c;
					if(c == '\n')
						break;
				}

				/* Provide for NUL-termination. */
				if(interpreter_line_size > 0)
				{
					/* Also strip all trailing blank spaces; that includes
					   line feed and carriage return characters. */
					while(interpreter_line_length > 0 && isspace(interpreter_line[interpreter_line_length-1]))
						interpreter_line_length--;

					interpreter_line[interpreter_line_length] = '\0';
				}
			}
		}
	}

	if(c == -1)
	{
		LONG io_error;

		/* Check if we just hit the end of the file or whethere
		   there was a genuine problem. */
		io_error = IoErr();
		if(io_error != 0)
		{
			__set_errno(__translate_io_error_to_errno(io_error));
			goto out;
		}
	}

	Close(script_file);
	script_file = ZERO;

	if(interpreter_line_size > 0)
	{
		interpreter_name = interpreter_line;

		for(i = 0 ; i < interpreter_line_size ; i++)
		{
			if(isspace(interpreter_name[i]))
			{
				interpreter_name[i] = '\0';

				interpreter_args = &interpreter_name[i+1];

				while((*interpreter_args) != '\0' && isspace(*interpreter_args))
					interpreter_args++;

				if((*interpreter_args) == '\0')
					interpreter_args = NULL;

				break;
			}
		}
	}

	parameter_string_len = get_arg_string_length(argv);

	if(interpreter_name != NULL)
	{
		struct name_translation_info nti;
		size_t interpreter_args_len;
		size_t len;

		UnLock(parent_dir);
		parent_dir = ZERO;

		if(find_file_and_parent(interpreter_name,&parent_dir,fib) != 0)
			goto out;

		error = __translate_unix_to_amiga_path_name(&path,&nti);
		if(error != 0)
		{
			__set_errno(error);
			goto out;
		}

		if(interpreter_args != NULL)
			interpreter_args_len = strlen(interpreter_args);
		else
			interpreter_args_len = 0;

		len = strlen(path);

		arg_string = malloc(interpreter_args_len + 1 + len + parameter_string_len + 1 + 1);
		if(arg_string == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}

		if(interpreter_args_len > 0)
		{
			memcpy(arg_string,interpreter_args,interpreter_args_len);
			arg_string_len += interpreter_args_len;
			
			arg_string[arg_string_len++] = ' ';
		}

		memcpy(&arg_string[arg_string_len],path,len);
		arg_string_len += len;

		if(parameter_string_len > 0)
			arg_string[arg_string_len++] = ' ';
	}
	else
	{
		arg_string = malloc(parameter_string_len + 1 + 1);
		if(arg_string == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}
	}

	if(parameter_string_len > 0)
	{
		build_arg_string(argv,&arg_string[arg_string_len]);
		arg_string_len += parameter_string_len;
	}

	arg_string[arg_string_len++]	= '\n';
	arg_string[arg_string_len]		= '\0';

	old_dir = CurrentDir(parent_dir);
	segment_list = LoadSeg(fib->fib_FileName);
	CurrentDir(old_dir);

	if(segment_list == ZERO)
	{
		__set_errno(ENOENT);
		goto out;
	}

	GetProgramName(old_program_name,sizeof(old_program_name));
	SetProgramName(fib->fib_FileName);

	old_dir = ThisProcess->pr_HomeDir;
	ThisProcess->pr_HomeDir = parent_dir;

	SetSignal(0,SIGBREAKF_CTRL_C);

	rc = RunCommand(segment_list,Cli()->cli_DefaultStack * sizeof(LONG),arg_string,arg_string_len);

	ThisProcess->pr_HomeDir = old_dir;

	if(rc == -1)
	{
		SetProgramName(old_program_name);

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	success = TRUE;

 out:

	if(arg_string != NULL)
		free(arg_string);

	if(parent_dir != ZERO)
		UnLock(parent_dir);

	if(script_file != ZERO)
		Close(script_file);

	if(interpreter_line != NULL)
		free(interpreter_line);

	if(segment_list != ZERO)
		UnLoadSeg(segment_list);

	if(success)
		exit(result);

	return(result);
}
