/*
 * $Id: stdlib_main.c,v 1.20 2005-03-20 11:18:06 obarthel Exp $
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

#ifndef EXEC_EXECBASE_H
#include <exec/execbase.h>
#endif /* EXEC_EXECBASE_H */

/****************************************************************************/

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_MONITORING_H
#include "stdlib_profile_monitoring.h"
#endif /* _STDLIB_PROFILE_MONITORING_H */

/****************************************************************************/

extern int main(int arg_c,char ** arg_v);

/****************************************************************************/

/* This will be set to TRUE in case a stack overflow was detected. */
BOOL __stack_overflow;

/****************************************************************************/

STATIC int
call_main(void)
{
	ENTER();

	/* This plants the return buffer for _exit(). */
	if(setjmp(__exit_jmp_buf) != 0)
		goto out;

	SHOWMSG("now invoking the constructors");

	/* Go through the constructor list */
	_init();

	SHOWMSG("done.");

	/* If the SAS/C profiling code is set up for printing function
	   call chains, switch it on now. */
	__show_profile_names();

	/* After all these preparations, get this show on the road... */
	exit(main((int)__argc,(char **)__argv));

 out:

	/* Switch off function name printing, if it was enabled. */
	__hide_profile_names();

	/* If we end up here with the __stack_overflow variable
	   set then the stack overflow handler dropped into
	   longjmp() and _exit() did not get called. This
	   means that we will have to show the error message
	   and invoke _exit() all on our own. */
	if(__stack_overflow)
	{
		SHOWMSG("we have a stack overflow");

		/* Dump whatever is waiting to be written to the
		   standard I/O streams, and make sure that no
		   break signal is about to make things any more
		   complicated than they already are. */
		__check_abort_enabled = FALSE;

		/* Dump all currently unwritten data, especially to the console. */
		__flush_all_files(-1);

		__show_error("Stack overflow detected");

		if(setjmp(__exit_jmp_buf) == 0)
			_exit(RETURN_FAIL);
	}

	/* If necessary, print stack size usage information. */
	#ifndef NDEBUG
	{
		__stack_usage_exit();
	}
	#endif /* NDEBUG */

	SHOWMSG("invoking the destructors");

	/* If one of the destructors drops into exit(), either directly
	   or through a failed assert() call, processing will resume with
	   the next following destructor. */
	(void)setjmp(__exit_jmp_buf);

	/* Go through the destructor list */
	_fini();

	SHOWMSG("done.");

	RETURN(__exit_value);
	return(__exit_value);
}

/****************************************************************************/

STATIC VOID ASM
detach_cleanup(REG(d0, LONG UNUSED unused_return_code),REG(d1, BPTR segment_list))
{
	#if defined(__amigaos4__)
	{
		if(__IUtility != NULL)
			DropInterface((struct Interface *)__IUtility);

		if(IDOS != NULL)
			DropInterface((struct Interface *)IDOS);
	}
	#else
	{
		/* The following trick is necessary only under dos.library V40 and below. */
		if(((struct Library *)DOSBase)->lib_Version < 50)
		{
			/* Now for the slightly shady part. We need to unload the segment
			   list this program was originally loaded with. We have to close
			   dos.library, though, which means that either we can close the
			   library or unload the code, but not both. But there's a loophole
			   in that we can enter Forbid(), unload the code, close the library
			   and exit and nobody will be able to allocate this program's
			   memory until after the process has been terminated. */
			Forbid();

			UnLoadSeg(segment_list);
		}
	}
	#endif /* __amigaos4__ */

	if(__UtilityBase != NULL)
		CloseLibrary(__UtilityBase);

	if(DOSBase != NULL)
		CloseLibrary(DOSBase);
}

/****************************************************************************/

STATIC ULONG
get_stack_size(void)
{
	struct Task * tc = FindTask(NULL);
	ULONG upper,lower;
	ULONG result;

	/* How much stack size was provided? */
	upper = (ULONG)tc->tc_SPUpper;
	lower = (ULONG)tc->tc_SPLower;

	result = upper - lower;

	return(result);
}

/****************************************************************************/

/* The global utility.library base, as required by clib2. */
struct Library * __UtilityBase;

/****************************************************************************/

#if defined(__amigaos4__)
struct UtilityIFace * __IUtility;
#endif /* __amigaos4__ */

/****************************************************************************/

int
_main(void)
{
	volatile struct Process * child_process = NULL;
	volatile struct WBStartup * startup_message;
	volatile APTR old_window_pointer = NULL;
	volatile BOOL old_window_pointer_valid = FALSE;
	struct Process * this_process;
	int return_code = RETURN_FAIL;
	ULONG current_stack_size;
	int os_version;

	SysBase = *(struct Library **)4;

	#if defined(__amigaos4__)
	{
		/* Get exec interface */
		IExec = (struct ExecIFace *)((struct ExecBase *)SysBase)->MainInterface;
	}
	#endif /* __amigaos4__ */

	/* Pick up the Workbench startup message, if available. */
	this_process = (struct Process *)FindTask(NULL);

	if(this_process->pr_CLI == ZERO)
	{
		struct MsgPort * mp = &this_process->pr_MsgPort;

		WaitPort(mp);

		startup_message = (struct WBStartup *)GetMsg(mp);
	}
	else
	{
		startup_message = NULL;
	}

	__WBenchMsg = (struct WBStartup *)startup_message;

	/* Check which minimum operating system version we actually require. */
	os_version = 37;
	if(__minimum_os_lib_version > 37)
		os_version = __minimum_os_lib_version;

	/* We will need dos.library V37 and utility.library V37. */
	DOSBase			= (struct Library *)OpenLibrary("dos.library",os_version);
	__UtilityBase	= OpenLibrary("utility.library",os_version);

	if(DOSBase == NULL || __UtilityBase == NULL)
	{
		char * error_message;

		/* If available, use the error message provided by the client. */
		if(__minimum_os_lib_error != NULL)
			error_message = __minimum_os_lib_error;
		else
			error_message = "This program requires AmigaOS 2.04 or higher.";

		__show_error(error_message);
		goto out;
	}

	#if defined(__amigaos4__)
	{
		/* Get interfaces for DOS and Utility */
		IDOS		= (struct DOSIFace *)GetInterface(DOSBase, "main", 1, 0);
		__IUtility	= (struct UtilityIFace *)GetInterface(__UtilityBase, "main", 1, 0);

		if (IDOS == NULL || __IUtility == NULL)
		{
			__show_error("This program requires AmigaOS 4.0 or higher.");
			goto out;
		}
	}
	#endif /* __amigaos4__ */

	if(__disable_dos_requesters)
	{
		/* Don't display any requesters. */
		old_window_pointer = __set_process_window((APTR)-1);
	}
	else
	{
		/* Just remember the original pointer. */
		old_window_pointer = __set_process_window(NULL);

		__set_process_window(old_window_pointer);
	}

	old_window_pointer_valid = TRUE;

	/* If a callback was provided which can fill us in on which
	   minimum stack size should be used, invoke it now and
	   store its result in the global __stack_size variable. */
	if(__get_default_stack_size != NULL)
	{
		unsigned int size;

		size = (*__get_default_stack_size)();
		if(size > 0)
			__stack_size = size;
	}

	/* How much stack size was provided? */
	current_stack_size = get_stack_size();

	/* If this is a resident program, don't allow for the detach
	   code to run. Same goes for launching the program from
	   Workbench. */
	if(__is_resident || startup_message != NULL)
	{
		__detach = FALSE;
	}
	else if (__check_detach != NULL)
	{
		/* Check if we may need to detach from the shell. */
		__detach = (*__check_detach)();
	}

	/* The following code will be executed if the program is to keep
	   running in the shell or was launched from Workbench. */
	if(DO_NOT __detach)
	{
		int old_priority = this_process->pr_Task.tc_Node.ln_Pri;

		/* Change the task priority, if requested. */
		if(-128 <= __priority && __priority <= 127)
			SetTaskPri((struct Task *)this_process,__priority);

		/* Was a minimum stack size requested and do we
		   need more stack space than was provided for? */
		if(__stack_size > 0 && current_stack_size < (ULONG)__stack_size)
		{
			struct StackSwapStruct * stk;
			unsigned int stack_size;
			APTR new_stack;

			/* Make the stack size a multiple of 32 bytes. */
			stack_size = 32 + ((__stack_size + 31UL) & ~31UL);

			/* Allocate the stack swapping data structure
			   and the stack space separately. */
			stk = AllocVec(sizeof(*stk),MEMF_PUBLIC|MEMF_ANY);
			if(stk == NULL)
				goto out;

			new_stack = AllocMem(stack_size,MEMF_PUBLIC|MEMF_ANY);
			if(new_stack == NULL)
			{
				FreeVec(stk);
				goto out;
			}

			/* Fill in the lower and upper bounds, then take care of
			   the stack pointer itself. */
			stk->stk_Lower		= new_stack;
			stk->stk_Upper		= (ULONG)(new_stack) + stack_size;
			stk->stk_Pointer	= (APTR)(stk->stk_Upper - 32);

			/* If necessary, set up for stack size usage measurement. */
			#ifndef NDEBUG
			{
				__stack_usage_init(stk);
			}
			#endif /* NDEBUG */

			return_code = __swap_stack_and_call(stk,(APTR)call_main);

			FreeMem(new_stack, stack_size);
			FreeVec(stk);
		}
		else
		{
			/* We have enough room to make the call or just don't care. */
			return_code = call_main();
		}

		/* Restore the task priority. */
		SetTaskPri((struct Task *)this_process,old_priority);
	}
	else
	{
		struct CommandLineInterface * cli = Cli();
		struct TagItem tags[12];
		UBYTE program_name[256];
		unsigned int stack_size;
		int i;

		/* Now for the interesting part: detach from the shell we're
		   currently executing in. This works only if the program is
		   not reentrant and has not been launched from Workbench. */

		stack_size = __stack_size;

		if(stack_size < current_stack_size)
			stack_size = current_stack_size;

		if(stack_size < cli->cli_DefaultStack * sizeof(LONG))
			stack_size = cli->cli_DefaultStack * sizeof(LONG);

		GetProgramName(program_name,sizeof(program_name));

		i = 0;

		tags[i].	ti_Tag	= NP_Entry;
		tags[i++].	ti_Data	= (ULONG)call_main;
		tags[i].	ti_Tag	= NP_StackSize;
		tags[i++].	ti_Data	= stack_size;
		tags[i].	ti_Tag	= NP_Name;
		tags[i++].	ti_Data	= (ULONG)(__process_name != NULL ? (UBYTE *)__process_name : FilePart(program_name));
		tags[i].	ti_Tag	= NP_CommandName;
		tags[i++].	ti_Data	= (ULONG)FilePart(program_name);
		tags[i].	ti_Tag	= NP_Cli;
		tags[i++].	ti_Data	= TRUE;
		tags[i].	ti_Tag	= NP_Arguments;
		tags[i++].	ti_Data	= (ULONG)GetArgStr();
		tags[i].	ti_Tag	= NP_ExitCode;
		tags[i++].	ti_Data	= (ULONG)detach_cleanup;
		tags[i].	ti_Tag	= NP_ExitData;
		tags[i++].	ti_Data	= (ULONG)cli->cli_Module;

		/* Use a predefined task priority, if requested. */
		if(-128 <= __priority && __priority <= 127)
		{
			tags[i].	ti_Tag	= NP_Priority;
			tags[i++].	ti_Data	= (ULONG)__priority;
		}

		/* dos.library V50 will free the segment list upon exit. */
		if(((struct Library *)DOSBase)->lib_Version >= 50)
		{
			tags[i].	ti_Tag	= NP_Seglist;
			tags[i++].	ti_Data	= (ULONG)cli->cli_Module;;
			tags[i].	ti_Tag	= NP_FreeSeglist;
			tags[i++].	ti_Data	= TRUE;
		}

		tags[i].ti_Tag = TAG_END;

		Forbid();

		child_process = CreateNewProc(tags);
		if(child_process == NULL)
		{
			Permit();

			PrintFault(IoErr(),program_name);

			return_code = RETURN_FAIL;
			goto out;
		}

		/* The standard I/O streams are no longer attached to a console. */
		__no_standard_io = TRUE;

		cli->cli_Module = ZERO;

		return_code = RETURN_OK;

		Permit();
	}

 out:

	if(old_window_pointer_valid)
		__set_process_window(old_window_pointer);

	if(child_process == NULL)
	{
		#if defined(__amigaos4__)
		{
			if(__IUtility != NULL)
			{
				DropInterface((struct Interface *)__IUtility);
				__IUtility = NULL;
			}
		}
		#endif /* __amigaos4__ */

		if(__UtilityBase != NULL)
		{
			CloseLibrary(__UtilityBase);
			__UtilityBase = NULL;
		}

		#if defined(__amigaos4__)
		{
			if(IDOS != NULL)
			{
				DropInterface((struct Interface *)IDOS);
				IDOS = NULL;
			}
		}
		#endif /* __amigaos4__ */

		if(DOSBase != NULL)
		{
			CloseLibrary(DOSBase);
			DOSBase = NULL;
		}
	}

	if(startup_message != NULL)
	{
		Forbid();

		ReplyMsg((struct Message *)startup_message);
	}

	return(return_code);
}

/****************************************************************************/

/* The following is automatically called by the main() function through code
   inserted by GCC. In theory, this could be removed by updating the machine
   definition, but for now we'll just keep this stub around. It is intended
   to call the constructor functions, but we do this in our own _main()
   anyway. */

#if defined(__GNUC__)

/****************************************************************************/

void __main(void);

/****************************************************************************/

void
__main(void)
{
}

/****************************************************************************/

#endif /* __GNUC__ */
