*
*	$Id: stdlib_swapstack.asm,v 1.1.1.1 2004-07-26 16:32:11 obarthel Exp $
*
*	:ts=8
*
*	Portable ISO 'C' (1994) runtime library for the Amiga computer
*	Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
*	All rights reserved.
*
*	Redistribution and use in source and binary forms, with or without
*	modification, are permitted provided that the following conditions
*	are met:
*
*	  - Redistributions of source code must retain the above copyright
*	    notice, this list of conditions and the following disclaimer.
*
*	  - Neither the name of Olaf Barthel nor the names of contributors
*	    may be used to endorse or promote products derived from this
*	    software without specific prior written permission.
*
*	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
*	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*	POSSIBILITY OF SUCH DAMAGE.
*

	include	"exec/macros.i"

	section	text,code

_LVOStackSwap equ -732

	xref	_SysBase

	xdef	___swap_stack_and_call

___swap_stack_and_call:

	movem.l	d2/a2/a3/a6,-(sp)

	move.l	_SysBase,a6

	move.l	20(sp),a2
	move.l	24(sp),a3

	move.l	a2,a0
	jsr	_LVOStackSwap(a6)

	jsr	(a3)
	move.l	d0,d2

	move.l	a2,a0
	jsr	_LVOStackSwap(a6)

	move.l	d2,d0

	movem.l	(sp)+,d2/a2/a3/a6
	rts

	end
