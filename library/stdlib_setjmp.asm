*
* $Id: stdlib_setjmp.asm,v 1.2 2005-01-02 09:07:18 obarthel Exp $
*
* :ts=8
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
*

	include "stdlib_setjmp.i"

******************************************************************************

	section	text,code

******************************************************************************

	xref	_SysBase

******************************************************************************

	xdef	@setjmp
	xdef	_setjmp

@setjmp:
	move.l	a0,-(sp)			; "save" A0
	bra.b	l1				; we don't need to load A0 from the stack

_setjmp:
	move.l	a0,-(sp)			; save A0
	move.l	8(sp),a0			; (struct __jmp_buf *) env

l1	move.l	4(sp),JB_RETURNADDRESS(a0)	; remember the return address
	movem.l	d1-d7/a0-a6,JB_D1(a0)		; save all registers, except for D0 and A7
	move.l	(sp)+,JB_A0(a0)			; put the old A0 where it belongs
	move.l	sp,JB_A7(a0)			; put the old A7 where it belongs

	move.l	_SysBase,a1
	btst	#AFB_68881,AttnFlags+1(a1)	; is there an FPU installed?
	beq.b	l2				; skip the following if not

	fmovem.x fp0-fp7,JB_FP0(a0)		; save all floating point registers

l2	moveq	#0,d0				; always return 0
	rts

******************************************************************************

	xdef	_longjmp
	xdef	@longjmp

_longjmp:
	move.l	4(sp),a0			; (struct __jmp_buf *) env
	move.l	8(sp),d0			; (int) status
@longjmp:
	tst.l	d0
	bne.b	l3
	moveq	#1,d0				; make sure that the result is always non-zero

l3	move.l	_SysBase,a1
	btst	#AFB_68881,AttnFlags+1(a1)	; is there an FPU installed?
	beq.b	l4				; skip the following if not

	fmovem.x JB_FP0(a0),fp0-fp7

l4	movem.l	JB_D1(a0),d1-d7			; restore all data registers
	movem.l	JB_A1(a0),a1-a7			; restore almost all address registers, except for A0
	move.l	JB_RETURNADDRESS(a0),(sp)	; fix up the return address
	move.l	JB_A0(a0),a0			; restore A0
	rts					; and return to the address SetJump() was called from

******************************************************************************

	end
