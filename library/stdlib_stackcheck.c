/*
 * $Id: stdlib_stackcheck.c,v 1.8 2005-03-18 12:38:25 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

#if (defined(__GNUC__) && !defined(__PPC__))

/****************************************************************************/

/* The stack extension code has its own set of these routines. */
#ifndef STACK_EXTENSION

/****************************************************************************/

#if defined(SMALL_DATA)
#define A4(x) "a4@(" #x ":W)"
#elif defined(SMALL_DATA32)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) #x
#endif /* SMALL_DATA */

/****************************************************************************/

asm("                                                                     \n\
                                                                          \n\
	.text                                                                  \n\
	.even                                                                  \n\
                                                                          \n\
	.globl	___stkovf                                                     \n\
	.globl	___stk_limit                                                  \n\
                                                                          \n\
	.globl	___stkchk_d0                                                  \n\
                                                                          \n\
___stkchk_d0:                                                             \n\
                                                                          \n\
	negl	d0                                                               \n\
	addl	sp,d0                                                            \n\
	cmpl	"A4(___stk_limit)",d0                                            \n\
	jcs		overflow				| if (sp-d0) < __stk_limit then we have an overflow\n\
	rts                                                                    \n\
                                                                          \n\
	.globl	___stkchk_0                                                   \n\
                                                                          \n\
___stkchk_0:                                                              \n\
                                                                          \n\
	cmpl	"A4(___stk_limit)",sp                                            \n\
	jcs		overflow				| if sp < __stk_limit then we have an overflow\n\
	rts                                                                    \n\
                                                                          \n\
overflow:                                                                 \n\
                                                                          \n\
	movel	#0,"A4(___stk_limit)"	| disable stack checking                 \n\
	jra		___stkovf                                                     \n\
                                                                          \n\
");

/****************************************************************************/

#endif /* STACK_EXTENSION && !__PPC__ */

/****************************************************************************/

#endif /* __GNUC__ */

/****************************************************************************/

UBYTE * __stk_limit;

/****************************************************************************/

#if defined(__SASC)

UBYTE * __base;

#endif /* __SASC */

/****************************************************************************/

STK_CONSTRUCTOR(stk_init)
{
	struct Task * this_task = FindTask(NULL);
	ULONG lower = (ULONG)this_task->tc_SPLower;

	ENTER();

	#if defined(__GNUC__)
	{
		__stk_limit = (UBYTE *)(lower + __stk_safezone + __stk_argbytes);
	}
	#endif /* __GNUC__ */

	#if defined(__SASC)
	{
		__base = (UBYTE *)(lower + __stk_safezone + __stk_argbytes);
	}
	#endif /* __SASC */

	LEAVE();

	CONSTRUCTOR_SUCCEED();
}
