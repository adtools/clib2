/*
 * $Id: stdlib_headers.h,v 1.1.1.1 2004-07-26 16:31:56 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#define _STDLIB_HEADERS_H

/****************************************************************************/

#include <exec/libraries.h>
#include <exec/memory.h>

/****************************************************************************/

/* This enables the legacy compatible 'struct AnchorPathOld'. */
#define USE_OLD_ANCHORPATH

#if (INCLUDE_VERSION >= 50)
#include <dos/anchorpath.h>
#endif /* (INCLUDE_VERSION >= 50) */

#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/dosasl.h>

/****************************************************************************/

#include <workbench/startup.h>

/****************************************************************************/

#include <clib/alib_protos.h>

/****************************************************************************/

#include <proto/exec.h>
#include <proto/dos.h>

/****************************************************************************/

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <dos.h>

/****************************************************************************/

#ifndef _STDLIB_LOCALE_UTILITY_H
#include "stdlib_locale_utility.h"
#endif /* _STDLIB_LOCALE_UTILITY_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)
#include <math.h>
#endif /* FLOATING_POINT_SUPPORT */

/****************************************************************************/

#include "macros.h"
#include "debug.h"

/****************************************************************************/

struct MemoryNode
{
	struct MinNode		mn_MinNode;
	size_t				mn_Size;

	UBYTE				mn_NeverFree;

#ifdef __MEM_DEBUG
	UBYTE				mn_AlreadyFree;
	UBYTE				mn_Pad0[2];

	void *				mn_Allocation;
	size_t				mn_AllocationSize;

	char *				mn_FreeFile;
	int					mn_FreeLine;

	char *				mn_File;
	int					mn_Line;

#ifdef __USE_MEM_TREES
	struct MemoryNode *	mn_Left;
	struct MemoryNode *	mn_Right;
	struct MemoryNode *	mn_Parent;
	UBYTE				mn_IsRed;
	UBYTE				mn_Pad1[3];
#endif /* __USE_MEM_TREES */

#else
	UBYTE				mn_Pad0[3];
#endif /* __MEM_DEBUG */
};

#ifdef __USE_MEM_TREES

struct MemoryTree
{
	struct MemoryNode *	mt_Root;
	struct MemoryNode	mt_RootNode;
	struct MemoryNode	mt_NullNode;
};

#endif /* __USE_MEM_TREES */

/****************************************************************************/

struct LocalVariable
{
	struct LocalVariable *	lv_Next;
	char *					lv_Name;
};

/****************************************************************************/

extern struct LocalVariable * __lv_root;

/****************************************************************************/

extern unsigned __random_seed;

/****************************************************************************/

extern struct MemoryTree	__memory_tree;
extern struct MinList		__memory_list;
extern APTR					__memory_pool;

/****************************************************************************/

extern struct MinList __alloca_memory_list;

/****************************************************************************/

extern unsigned long __maximum_memory_allocated;
extern unsigned long __current_memory_allocated;
extern unsigned long __maximum_num_memory_chunks_allocated;
extern unsigned long __current_num_memory_chunks_allocated;

/****************************************************************************/

extern jmp_buf	__exit_jmp_buf;
extern int		__exit_value;

/****************************************************************************/

extern unsigned int (* __get_default_stack_size)(void);

/****************************************************************************/

extern unsigned int __stack_size;
extern BOOL __stack_overflow;
extern UBYTE * __stk_limit;
/*extern UBYTE ** __stackborders;*/
/*extern UBYTE * __SaveSP;*/
extern UBYTE * __base;

/****************************************************************************/

extern ULONG __stk_argbytes;
extern ULONG __stk_safezone;
extern ULONG __stk_maxsize;
extern ULONG __stk_size;
extern ULONG __stk_extensions;
extern ULONG __stk_minframe;

/****************************************************************************/

extern BOOL __is_resident;

/****************************************************************************/

extern BOOL __free_program_name;

/****************************************************************************/

extern UBYTE __shell_escape_character;

/****************************************************************************/

extern char ** __argv;
extern int __argc;

/****************************************************************************/

extern UBYTE * __quote_vector;

/****************************************************************************/

extern int __default_pool_size;
extern int __default_puddle_size;

/****************************************************************************/

#ifndef _STDLIB_PROTOS_H
#include "stdlib_protos.h"
#endif /* _STDLIB_PROTOS_H */

/****************************************************************************/

extern void kprintf(const char * format,...);

/****************************************************************************/

#endif /* _STDLIB_HEADERS_H */
