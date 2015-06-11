*
* $Id: sas_cxv.asm,v 1.1.1.1 2004-07-26 16:31:04 obarthel Exp $
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

	xdef	__CXV25
	xdef	__CXV35
	xdef	__CXNRM5
	xdef	__CXTAB5

__CXV25:

	MOVEM.L	D2-D5/A1,-(SP)
	MOVEQ	#0,D4
	MOVEQ	#0,D1
	BRA	L2C

__CXV35:

	MOVEM.L	D2-D5/A1,-(SP)
	MOVEQ	#0,D4
	MOVEQ	#0,D1
	TST.L	D0
	BEQ	L54
	BPL	L2C
	MOVE.W	#$8000,D4
	NEG.L	D0

L2C:	CMP.L	#$200000,D0
	BCC	L44
	MOVE.W	#$4120,D5
	JSR	__CXNRM5(PC)
L54:	MOVEM.L	(SP)+,D2-D5/A1
	RTS

L44:	MOVE.W	D0,D1
	CLR.W	D0
	SWAP	D0
	SWAP	D1
	MOVE.W	#$4220,D5
	JSR	__CXNRM5(PC)
	MOVEM.L	(SP)+,D2-D5/A1
	RTS

__CXNRM5:

	CMP.L	#$20,D0
	BGE	L98
	SWAP	D0
	SWAP	D1
	MOVE.W	D1,D0
	CLR.W	D1
	SUB.W	#$100,D5
	BGE.S	__CXNRM5
	BRA	LDC

__CXTAB5:

	dc.b	5
	dc.b	4
	dc.b	3
	dc.b	3
	dc.b	2
	dc.b	2
	dc.b	2
	dc.b	2
	dc.b	1
	dc.b	1
	dc.b	1
	dc.b	1
	dc.b	1
	dc.b	1
	dc.b	1
	dc.b	1
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0
	dc.b	0

L98:	MOVEQ	#0,D3
	CMP.L	#$2000,D0
	BGE	LA8
	LSL.L	#8,D0
	ADDQ.W	#8,D3

LA8:	SWAP	D0
	TST.W	D0
	BNE	LB4
	ROL.L	#4,D0
	ADDQ.W	#4,D3

LB4:	MOVEQ	#0,D2
	MOVE.B	__CXTAB5(PC,D0.W),D2
	ROL.L	D2,D0
	ADD.W	D2,D3
	SWAP	D0
	MOVE.L	D1,D2
	LSL.L	D3,D1
	ROL.L	D3,D2
	EOR.W	D1,D2
	EOR.W	D2,D0
	LSL.W	#4,D3
	SUB.W	D3,D5
	BLT	LDC
	SWAP	D0
	ADD.W	D5,D0
	OR.W	D4,D0
	SWAP	D0
	RTS	

LDC:	NEG.W	D5
	LSR.W	#4,D5
	MOVE.L	D0,D2
	LSR.L	D5,D0
	ROR.L	D5,D2
	LSR.L	D5,D1
	EOR.L	D0,D2
	EOR.L	D2,D1
	SWAP	D0
	EOR.W	D4,D0
	SWAP	D0
	RTS

	end
