/*
 * $Id: stdlib_startup.c,v 1.8 2005-03-03 09:32:09 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#include "stdlib_headers.h"
#include "unistd_headers.h"

/****************************************************************************/

#include <exec/execbase.h>

/****************************************************************************/

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/****************************************************************************/

#include "macros.h"

/****************************************************************************/

static struct MsgPort *	old_console_task;
static BOOL				restore_console_task;

/****************************************************************************/

static BOOL restore_streams;

/****************************************************************************/

static BPTR old_output;
static BPTR old_input;

/****************************************************************************/

static BPTR output;
static BPTR input;

/****************************************************************************/

char **	__argv;
int		__argc;

/****************************************************************************/

STATIC BOOL
is_space(unsigned char c)
{
	BOOL result;

	result = (BOOL)(c == '\t' ||	/* tab */
	                c == '\r' ||	/* carriage return */
	                c == '\n' ||	/* line feed */
	                c == '\v' ||	/* vertical tab */
	                c == '\f' ||	/* form feed */
	                c == ' '  ||	/* blank space */
	                c == '\240');	/* non-breaking space */

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

int
__startup_init(void)
{
	int result = -1;

	/* Shell startup? */
	if(__WBenchMsg == NULL)
	{
		size_t			number_of_arguments;
		unsigned char *	command_name;
		size_t			command_name_len;
		unsigned char *	arg_str;
		size_t			arg_len;
		unsigned char *	command_line;
		unsigned char *	str;

		/* Make a copy of the current command name string. */
		command_name_len = (size_t)((UBYTE *)BADDR(Cli()->cli_CommandName))[0];

		__free_program_name = TRUE;

		command_name = AllocVec((ULONG)(command_name_len + 1),MEMF_ANY);
		if(command_name == NULL)
			goto out;

		if(CANNOT GetProgramName(command_name,command_name_len + 1))
			goto out;

		__program_name = (char *)command_name;

		/* Get the shell parameter string and find out
		   how long it is, stripping a trailing line
		   feed and blank spaces if necessary. */
		arg_str = GetArgStr();

		while((*arg_str) == ' ' || (*arg_str) == '\t')
			arg_str++;

		arg_len = strlen(arg_str);

		while(arg_len > 0 && (arg_str[arg_len - 1] == '\n' || arg_str[arg_len - 1] == ' ' || arg_str[arg_len - 1] == '\t'))
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
		__argv[0] = command_name;

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

				/* If necessary, indicate that this parameter was quoted. */
				if(__wildcard_quote_parameter(__argc) < 0)
					goto out;

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
	}
	else
	{
		PROFILE_OFF();

		__program_name = (char *)__WBenchMsg->sm_ArgList[0].wa_Name;

		__original_current_directory = CurrentDir(__WBenchMsg->sm_ArgList[0].wa_Lock);
		__current_directory_changed = TRUE;

		if (__stdio_window_specification != NULL)
		{
			input = Open(__stdio_window_specification,MODE_NEWFILE);	
		}
		else if (__WBenchMsg->sm_ToolWindow != NULL)
		{
			input = Open(__WBenchMsg->sm_ToolWindow,MODE_NEWFILE);	
		}
		else
		{
			static const char console_prefix[] = "CON:20/20/600/150/";
			static const char console_suffix[] = " Output/AUTO/CLOSE/WAIT";
			STRPTR window_specifier;
			STRPTR tool_name;
			size_t len;

			tool_name = FilePart(__WBenchMsg->sm_ArgList[0].wa_Name);

			len = strlen(console_prefix) + strlen(tool_name) + strlen(console_suffix);

			window_specifier = malloc(len+1);
			if(window_specifier == NULL)
				goto out;

			strcpy(window_specifier,console_prefix);
			strcat(window_specifier,tool_name);
			strcat(window_specifier,console_suffix);

			input = Open(window_specifier,MODE_NEWFILE);

			free(window_specifier);
		}

		if(input == ZERO)
			input = Open("NIL:",MODE_NEWFILE);

		if(input != ZERO)
		{
			struct FileHandle * fh = BADDR(input);

			old_console_task = SetConsoleTask(fh->fh_Type);

			output = Open("CONSOLE:",MODE_NEWFILE);
			if(output != ZERO)
				restore_console_task = TRUE;
			else
				SetConsoleTask((struct MsgPort *)old_console_task);
		}

		if(output == ZERO)
			output = Open("NIL:",MODE_NEWFILE);

		if(input == ZERO || output == ZERO)
			goto out;

		old_input	= SelectInput(input);
		old_output	= SelectOutput(output);

		restore_streams = TRUE;

		__argv = (char **)__WBenchMsg;

		PROFILE_ON();
	}

	result = 0;

 out:

	return(result);
}

/****************************************************************************/

CLIB_DESTRUCTOR(__startup_exit)
{
	ENTER();

	PROFILE_OFF();

	/* Now clean up after the streams set up for the Workbench
	   startup... */
	if(restore_console_task)
	{
		SetConsoleTask((struct MsgPort *)old_console_task);
		old_console_task = NULL;

		restore_console_task = FALSE;
	}

	if(restore_streams)
	{
		SelectInput(old_input);
		old_input = ZERO;

		SelectOutput(old_output);
		old_output = ZERO;

		restore_streams = FALSE;
	}

	if(input != ZERO)
	{
		Close(input);
		input = ZERO;
	}

	if(output != ZERO)
	{
		Close(output);
		output = ZERO;
	}

	PROFILE_ON();

	LEAVE();
}
