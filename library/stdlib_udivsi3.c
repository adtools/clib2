/*
 * $Id: stdlib_udivsi3.c,v 1.2 2005-01-02 09:07:19 obarthel Exp $
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

#if defined(__GNUC__)

/****************************************************************************/
#ifndef __PPC__
asm("                                                                     \n\
                                                                          \n\
	.text                                                                  \n\
	.even                                                                  \n\
                                                                          \n\
	.globl	___udivsi3                                                    \n\
	.globl	___udivsi4                                                    \n\
                                                                          \n\
| D0.L = D0.L / D1.L unsigned                                             \n\
                                                                          \n\
___udivsi3:                                                               \n\
                                                                          \n\
	moveml	sp@(4:W),d0/d1                                                \n\
	jbra	___udivsi4                                                       \n\
                                                                          \n\
");
#else
__asm("							\n\
    .text						\n\
    .align 2					\n\
								\n\
    .globl __udivsi3			\n\
__udivsi3:						\n\
     divwu r3, r3, r4			\n\
     blr						\n\
");
#endif

/****************************************************************************/

#endif /* __GNUC__ */
