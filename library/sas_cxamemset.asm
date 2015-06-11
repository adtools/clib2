*
* $Id: sas_cxamemset.asm,v 1.1.1.1 2004-07-26 16:31:03 obarthel Exp $
*
* :ts=8
*
* Adapted from reassembled SAS/C runtime library code.
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
*

	section	text,code

	xdef	__CXAMEMSET

__CXAMEMSET:

	cmpi.l	#$18,d1
	blt.s	L2A

L10:	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	subi.l	#$18,d1
	cmpi.l	#$18,d1
	bge.s	L10

L2A:	add.w	d1,d1
	move.w	L34(pc,d1.w),d1
	jmp	L34(pc,d1.w)

L34:	dc.w	$66
	dc.w	$58
	dc.w	$4A
	dc.w	$3A
	dc.w	$64
	dc.w	$56
	dc.w	$48
	dc.w	$38
	dc.w	$62
	dc.w	$54
	dc.w	$46
	dc.w	$36
	dc.w	$60
	dc.w	$52
	dc.w	$44
	dc.w	$34
	dc.w	$5E
	dc.w	$50
	dc.w	$42
	dc.w	$32
	dc.w	$5C
	dc.w	$4E
	dc.w	$40
	dc.w	$30

	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.w	d0,(a0)+
	move.b	d0,(a0)+
	rts

	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.w	d0,(a0)+
	rts

	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.b	d0,(a0)+
	rts

	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	rts

	end
