*
*	$Id: amiga_callhooka.asm,v 1.3 2006-01-08 12:04:22 obarthel Exp $
*
*	:ts=8
*
*	Portable ISO 'C' (1994) runtime library for the Amiga computer
*	Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

	section	text,code

h_Entry equ 8

	xdef	_CallHookA

****************************************************************************
* new hook standard
* use struct Hook (with minnode at the top)
*
* *** register calling convention: ***
*       A0 - pointer to hook itself
*       A1 - pointer to parameter packed ("message")
*       A2 - Hook specific address data ("object," e.g, gadget )
*
* ***  C conventions: ***
* Note that parameters are in unusual register order: a0, a2, a1.
* This is to provide a performance boost for assembly language
* programming (the object in a2 is most frequently untouched).
* It is also no problem in "register direct" C function parameters.
*
* calling through a hook
*       CallHook( hook, object, msgid, p1, p2, ... );
*       CallHookA( hook, object, msgpkt );
*
* using a C function:   CFunction( hook, object, message );
*       hook.h_Entry = HookEntry;
*       hook.h_SubEntry = CFunction;
*
****************************************************************************

* C calling hook interface for prepared message packet
_CallHookA:
	movem.l	a2/a6,-(sp)
	move.l	12(sp),a0
	move.l	16(sp),a2
	move.l	20(sp),a1
	pea.l	hook_return(pc)
	move.l	h_Entry(a0),-(sp)
	rts

hook_return:

	movem.l	(sp)+,a2/a6
	rts

	end
