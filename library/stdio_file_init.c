/*
 * $Id: stdio_file_init.c,v 1.13 2008-09-04 12:07:58 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

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

struct WBStartup * NOCOMMON __WBenchMsg;

/****************************************************************************/

/* CPU cache line size; used to align I/O buffers for best performance. */
ULONG __cache_line_size = 32;

/****************************************************************************/

FILE_DESTRUCTOR(workbench_exit)
{
	ENTER();

	PROFILE_OFF();

	/* Now clean up after the streams set up for Workbench startup... */
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

/****************************************************************************/

STATIC int
wb_file_init(void)
{
	int result = ERROR;

	PROFILE_OFF();

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

	result = OK;

 out:

	PROFILE_ON();

	return(result);
}

/****************************************************************************/

FILE_CONSTRUCTOR(stdio_file_init)
{
	struct SignalSemaphore * stdio_lock;
	struct SignalSemaphore * fd_lock;
	BPTR default_file;
	ULONG fd_flags,iob_flags;
	BOOL success = FALSE;
	char * buffer;
	char * aligned_buffer;
	int i;

	ENTER();

	/* Figure out the proper address alignment for the memory we are
	   going to use for disk I/O. The default is 32 bytes, which should
	   be OK for most cases. If possible, ask the operating system for
	   its preferred alignment size. */
	#if defined(__amigaos4__)
	{
		if(SysBase->lib_Version >= 50)
		{
			uint32 physical_alignment = 0;

			GetCPUInfoTags(GCIT_CacheLineSize,&physical_alignment,TAG_DONE);

			if(__cache_line_size < physical_alignment)
				__cache_line_size = physical_alignment;
		}
	}
	#endif /* __amigaos4__ */

	/* If we were invoked from Workbench, set up the standard I/O streams. */
	if(__WBenchMsg != NULL)
	{
		if(wb_file_init() < 0)
			goto out;
	}

	/* Now initialize the standard I/O streams (input, output, error). */
	for(i = STDIN_FILENO ; i <= STDERR_FILENO ; i++)
	{
		PROFILE_OFF();

		switch(i)
		{
			case STDIN_FILENO:

				iob_flags		= IOBF_IN_USE | IOBF_READ | IOBF_NO_NUL | IOBF_BUFFER_MODE_LINE;
				fd_flags		= FDF_IN_USE | FDF_READ | FDF_NO_CLOSE;
				default_file	= Input();
				break;

			case STDOUT_FILENO:

				iob_flags		= IOBF_IN_USE | IOBF_WRITE | IOBF_NO_NUL | IOBF_BUFFER_MODE_LINE;
				fd_flags		= FDF_IN_USE | FDF_WRITE | FDF_NO_CLOSE;
				default_file	= Output();
				break;

			case STDERR_FILENO:
			default:

				iob_flags		= IOBF_IN_USE | IOBF_WRITE | IOBF_NO_NUL | IOBF_BUFFER_MODE_NONE;
				fd_flags		= FDF_IN_USE | FDF_WRITE;
				default_file	= ZERO; /* NOTE: this is really initialized later; see below... */
				break;
		}

		PROFILE_ON();

		/* Allocate a little more memory than necessary. */
		buffer = malloc(BUFSIZ + (__cache_line_size - 1));
		if(buffer == NULL)
			goto out;

		#if defined(__THREAD_SAFE)
		{
			/* Allocate memory for an arbitration mechanism, then
			   initialize it. */
			stdio_lock	= __create_semaphore();
			fd_lock		= __create_semaphore();

			if(stdio_lock == NULL || fd_lock == NULL)
			{
				__delete_semaphore(stdio_lock);
				__delete_semaphore(fd_lock);

				goto out;
			}

			/* We ignore the file handle and let the file I/O
			   code in the fd hook pick up the appropriate
			   Input/Output/ErrorOutput handle. */
			default_file = i;

			fd_flags |= FDF_NO_CLOSE | FDF_STDIO;
		}
		#else
		{
			stdio_lock	= NULL;
			fd_lock		= NULL;

			/* Check if this stream is attached to a console window. */
			if(default_file != ZERO)
			{
				PROFILE_OFF();

				if(IsInteractive(default_file))
					SET_FLAG(fd_flags,FDF_IS_INTERACTIVE);

				PROFILE_ON();
			}
		}
		#endif /* __THREAD_SAFE */

		/* Align the buffer start address to a cache line boundary. */
		aligned_buffer = (char *)((ULONG)(buffer + (__cache_line_size-1)) & ~(__cache_line_size-1));

		__initialize_fd(__fd[i],__fd_hook_entry,default_file,fd_flags,fd_lock);

		__initialize_iob(__iob[i],__iob_hook_entry,
			buffer,
			aligned_buffer,BUFSIZ,
			i,
			i,
			iob_flags,
			stdio_lock);
	}

	#if NOT defined(__THREAD_SAFE)
	{
		/* If the program was launched from Workbench, we continue by
		   duplicating the default output stream for use as the
		   standard error stream. */
		if(__WBenchMsg != NULL)
		{
			PROFILE_OFF();
			__fd[STDERR_FILENO]->fd_File = Output();
			PROFILE_ON();

			SET_FLAG(__fd[STDERR_FILENO]->fd_Flags,FDF_NO_CLOSE);
		}
		else
		{
			BPTR ces;

			PROFILE_OFF();

			/* Figure out what the default error output stream is. */
			#if defined(__amigaos4__)
			{
				ces = ErrorOutput();
			}
			#else
			{
				struct Process * this_process = (struct Process *)FindTask(NULL);

				ces = this_process->pr_CES;
			}
			#endif /* __amigaos4__ */

			PROFILE_ON();

			/* Is the standard error stream configured? If so, use it.
			   Otherwise, try to duplicate the standard output stream. */
			if(ces != ZERO)
			{
				__fd[STDERR_FILENO]->fd_File = ces;

				SET_FLAG(__fd[STDERR_FILENO]->fd_Flags,FDF_NO_CLOSE);
			}
			else
			{
				__fd[STDERR_FILENO]->fd_File = Open("CONSOLE:",MODE_NEWFILE);
			}
		}

		PROFILE_OFF();

		/* Figure out if the standard error stream is bound to a console. */
		if(FLAG_IS_CLEAR(__fd[STDERR_FILENO]->fd_Flags,FDF_STDIO))
		{
			if(IsInteractive(__fd[STDERR_FILENO]->fd_File))
				SET_FLAG(__fd[STDERR_FILENO]->fd_Flags,FDF_IS_INTERACTIVE);
		}

		PROFILE_ON();
	}
	#endif /* __THREAD_SAFE */

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}
