/*
 * $Id: stdlib_modsi3.c,v 1.3 2006-01-08 12:04:26 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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
asm("                         \n\
                              \n\
	.text                      \n\
	.even                      \n\
                              \n\
	.globl	___modsi3         \n\
	.globl	___divsi4         \n\
                              \n\
| D1.L = D0.L % D1.L signed   \n\
                              \n\
___modsi3:                    \n\
                              \n\
	moveml	sp@(4:W),d0/d1    \n\
	jbsr	___divsi4            \n\
	movel	d1,d0                \n\
	rts                        \n\
                              \n\
");
#else
__asm("							\n\
	.text						\n\
	.align 2					\n\
								\n\
	.globl __modsi3				\n\
	.globl __divsi4 			\n\
								\n\
__modsi3: 						\n\
	stw    r0, 20(r1)			\n\
    divw   r0, r3, r4			\n\
    mullw  r0, r0, r4			\n\
    subf   r3, r0, r3			\n\
	blr							\n\
");
#endif

/****************************************************************************/

#endif /* __GNUC__ */
