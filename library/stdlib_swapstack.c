/*
 * $Id: stdlib_swapstack.c,v 1.3 2005-01-02 09:07:19 obarthel Exp $
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

#if defined(__GNUC__)

/****************************************************************************/

#if defined(SMALL_DATA)
#define A4(x) "a4@(" #x ":W)"
#elif defined(SMALL_DATA32)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) #x
#endif /* SMALL_DATA */

/****************************************************************************/

#ifndef __PPC__

/****************************************************************************/

asm("                                                                     \n\
	.text                                                                  \n\
	.even                                                                  \n\
                                                                          \n\
_LVOStackSwap = -732                                                      \n\
                                                                          \n\
	.globl	_SysBase                                                      \n\
	.globl	___swap_stack_and_call                                        \n\
                                                                          \n\
___swap_stack_and_call:                                                   \n\
                                                                          \n\
	moveml	d2/a2/a3/a6,sp@-                                              \n\
	movel	sp@(20),a2                                                       \n\
	movel	sp@(24),a3                                                       \n\
	movel	"A4(_SysBase)",a6                                                \n\
                                                                          \n\
	movel	a2,a0                                                            \n\
	jsr		a6@(_LVOStackSwap:W)                                          \n\
                                                                          \n\
	jsr		a3@                                                           \n\
	movel	d0,d2                                                            \n\
                                                                          \n\
	movel	a2,a0                                                            \n\
	jsr		a6@(_LVOStackSwap:W)                                          \n\
	                                                                       \n\
	movel	d2,d0                                                            \n\
                                                                          \n\
	moveml	sp@+,d2/a2/a3/a6                                              \n\
                                                                          \n\
	rts                                                                    \n\
                                                                          \n\
");

/****************************************************************************/

#else

/****************************************************************************/

/* Swap the current stack configuration out, call a function provided,
   swap the stack configuration back and return. */
int
__swap_stack_and_call(struct StackSwapStruct * stk,APTR function)
{
	register int result;

	StackSwap(stk);
	result = ((int (*)(void))function)();
	StackSwap(stk);

	return(result);
}

/****************************************************************************/

#endif /* __PPC__ */

/****************************************************************************/

#endif /* __GNUC__ */
