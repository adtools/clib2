/*
 * $Id: dos.h,v 1.3 2004-08-25 15:27:28 obarthel Exp $
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

#ifndef _DOS_H
#define _DOS_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifndef _STDIO_H
#include <stdio.h>
#endif /* _STDIO_H */

/****************************************************************************/

#ifndef WORKBENCH_STARTUP_H
#include <workbench/startup.h>
#endif /* WORKBENCH_STARTUP_H */

/****************************************************************************/

/*
 * The Workbench startup message passed to this program; this may be NULL
 * if the program was started from shell instead.
 *
 * The variable name is set up to be __WBenchMsg to make it fit into the
 * ISO 'C' standard context. It is redefined to WBenchMsg to make it
 * compatible with the original Amiga program startup code which would
 * refer to the variable under that name.
 */
extern struct WBStartup * __WBenchMsg;
#define WBenchMsg __WBenchMsg

/****************************************************************************/

/* This is filled in with a pointer to the name of the program being run. */
extern char * __program_name;

/****************************************************************************/

/* Set this to FALSE to disable all Ctrl+C checking in the program. */
extern BOOL __check_abort_enabled;

/*
 * You can replace this function with your own and perform your own
 * Ctrl+C checking.
 */
extern void __check_abort(void);

/****************************************************************************/

/*
 * Call this function to set up the environment information for your
 * program to access its data. This is typically used in Task or Process
 * functions, as launched from the main program.
 *
 * Note: this function is unavailable for residentable programs and may
 *       not be available for PowerPC programs.
 */
extern void geta4(void);

/****************************************************************************/

/*
 * Read and modify the current value of register A4; helpful for
 * programs which are residentable, yet need to be able to pass the
 * context information around A4 points to.
 *
 * Note: this function may not be available for PowerPC programs.
 */
extern unsigned long __get_a4(void);
extern void __set_a4(unsigned long value);

/****************************************************************************/

/*
 * Obtain the low level 'file' handle or socket ID bound to a file
 * descriptor. This function returns 0 for success and non-zero
 * otherwise (if, for example, the file descriptor value you
 * provided is not valid; errno will be set to the appropriate
 * error code, too). Put a pointer to the file handle variable you
 * want to be filled in into the second parameter to this function.
 */
extern int __get_default_file(int file_descriptor,long * file_ptr);

/****************************************************************************/

/*
 * This will be set to TRUE if the current program was launched from
 * the internet superserver ('inetd') or an equivalent facility.
 */
extern BOOL __is_daemon;

/****************************************************************************/

/*
 * If the library is built with memory debugging features enabled,
 * the following variable controls whether memory allocated, to be
 * released, will actually get released. If set to TRUE all memory
 * allocations will persist until the program exits.
 */
extern BOOL __never_free;

/****************************************************************************/

/*
 * Before memory is allocated, a quick check can be performed in order
 * to avoid draining so much system memory that both the operating system
 * and application software becomes unstable. This test checks for the
 * largest available block of memory, which has to be larger than a
 * threshold value for the memory allocation to proceed. That size
 * can be configured here. It defaults to 0, which means that no test
 * is performed. If this feature is enabled, a minimum threshold value
 * of 250000 bytes is recommended.
 */
extern ULONG __free_memory_threshold;

/****************************************************************************/

/*
 * The following section lists variables and function pointers which are used
 * by the startup code right after the program is launched. These variables are
 * looked up before your program's main() function is invoked. Therefore, you
 * would have to declare these variables in your program's data section and have
 * them initialized to certain well-defined values to get the desired effect.
 */

/*
 * If set to TRUE, your program's process->pr_WindowPtr will be set to -1
 * when it starts. The process->pr_WindowPtr will be automatically restored
 * to the initial value before it exits.
 */
extern BOOL __disable_dos_requesters;

/*
 * If set to TRUE, your program will disconnect itself from the shell it was
 * launched from and keep running in the background. This service is unavailable
 * for residentable programs. Note that you should not use this feature for
 * programs which are supposed to be launched by the internet superserver.
 * Also, note that when a program is run in the background, its input and
 * output streams will be connected to NIL:.
 */
extern BOOL __detach;

/*
 * If this function pointer is not NULL, it must point to a function which
 * figures out whether the program should detach itself from the shell it
 * was launched from. The function return value replaces the value of the
 * __detach variable.
 *
 * At the time this function is invoked, dos.library and utility.library
 * have already been opened for you.
 */
extern BOOL (* __check_detach)(void);

/*
 * This variable can be set up to contain the minimum stack size the program
 * should be launched with. If the startup code finds that there is not enough
 * stack space available to start with, it will attempt to allocate more and
 * then proceed to run your program.
 *
 * If this variable is set to 0 (the default) then no stack size test will
 * be performed upon startup.
 */
extern unsigned int __stack_size;

/*
 * If this function pointer is not NULL, it must point to a function which
 * figures out how much stack space is required to run the program. The
 * function return value replaces the value of the __stack_size variable
 * if it is not equal to zero.
 *
 * At the time this function is invoked, dos.library and utility.library
 * have already been opened for you.
 */
extern unsigned int (* __get_default_stack_size)(void);

/****************************************************************************/

/*
 * If linked with -lunix, Unix path names are translated to Amiga path
 * names (and the other way round). If you wish to disable this, set the
 * following variable to FALSE. Only the path name translation is affected
 * by setting this variable to FALSE. You will always get Unix-like behaviour
 * from certain functions regardless of whether the path names are translated
 * or not.
 */
extern BOOL __unix_path_semantics;

/****************************************************************************/

/* A data structures used by the path translation routines below. */
struct name_translation_info
{
	char	substitute[MAXPATHLEN];
	char *	original_name;
	int		is_root;
};

/****************************************************************************/

extern int __translate_relative_path_name(char const ** name_ptr,char *replace,size_t max_replace_len);
extern void __restore_path_name(char const ** name_ptr,struct name_translation_info * nti);
extern int __translate_amiga_to_unix_path_name(char const ** name_ptr,struct name_translation_info * nti);
extern int __translate_unix_to_amiga_path_name(char const ** name_ptr,struct name_translation_info * nti);
extern void __translate_io_error_to_errno(LONG io_error,int * errno_ptr);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _DOS_H */
