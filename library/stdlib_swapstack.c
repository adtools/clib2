/*
 * $Id: stdlib_swapstack.c,v 1.1.1.1 2004-07-26 16:32:11 obarthel Exp $
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

__asm("																		\n\
																			\n\
	.text																	\n\
	.align 2																\n\
																			\n\
	.set MainInterface, 632													\n\
	.set IExec_StackSwap, 272												\n\
																			\n\
	.globl	SysBase															\n\
	.globl	__swap_stack_and_call											\n\
																			\n\
__swap_stack_and_call:														\n\
	stwu	r1, -32(r1)					# Make a stack frame				\n\
	mflr	r0																\n\
	stw		r0, 36(r1)														\n\
	stmw	r29, 8(r1)					# Get us three GPRs					\n\
										# r29 will store IExec				\n\
	mr		r4, r31						# Backup ptr to function to call	\n\
	mr		r3, r30						# Backup ptr to StackSwapStruct		\n\
																			\n\
	lis		r3, SysBase@ha				# Retrieve sysbase pointer			\n\
	lwz		r3, SysBase@l(r3)												\n\
	lwz		r29, MainInterface(r3)		# Get IExec							\n\
																			\n\
	addi	r3, r29, IExec_StackSwap	# StackSwap entry					\n\
	mr		r4, r30						# StackSwap struct in r4			\n\
	mtlr	r3																\n\
	blrl								# IExec->StackSwap(r4)				\n\
																			\n\
	mtlr	r31							# Call payload						\n\
	blrl																	\n\
																			\n\
	addi	r3, r29, IExec_StackSwap	# Revert to old stack				\n\
	mr		r4, r30															\n\
	mtlr	r3																\n\
	blrl																	\n\
																			\n\
	lmw		r29, 8(r1)					# Restore registers					\n\
	lwz		r0, 36(r1)														\n\
	mtlr	r0																\n\
	addi	r1, r1, 32					# Clean up stack					\n\
																			\n\
	blr																		\n\
																			\n\
");

/****************************************************************************/

#endif /* __PPC__ */

/****************************************************************************/

#endif /* __GNUC__ */
