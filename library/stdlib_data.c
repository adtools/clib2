/*
 * $Id: stdlib_data.c,v 1.1.1.1 2004-07-26 16:31:53 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

/* NOTE: for Knuth's algorithm below the seed must not be zero. */
unsigned __random_seed = 1;

/****************************************************************************/

#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)

struct MemoryTree __memory_tree;

#endif /* __USE_MEM_TREES && __MEM_DEBUG */

/****************************************************************************/

APTR			__memory_pool;
struct MinList	__memory_list;

/****************************************************************************/

struct MinList __alloca_memory_list;

/****************************************************************************/

struct WBStartup * __WBenchMsg;

/****************************************************************************/

char *	__program_name;
BOOL	__free_program_name;

/****************************************************************************/

BOOL __stack_overflow;
UBYTE * __stk_limit;
/*UBYTE ** __stackborders;*/
/*UBYTE * __stk_initial;*/
ULONG __stk_maxsize;
ULONG __stk_size;
ULONG __stk_extensions;

/****************************************************************************/

struct Library * __UtilityBase;
struct Library * __LocaleBase;

/****************************************************************************/

#if defined(__amigaos4__)

struct UtilityIFace *	__IUtility;
struct LocaleIFace *	__ILocale;

#endif /* __amigaos4__ */

/****************************************************************************/

#if defined(__SASC)

UBYTE * __base;

#endif /* __SASC */
