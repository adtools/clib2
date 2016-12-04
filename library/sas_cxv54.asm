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

	xdef	__CXV54
	xref	__CXFERR

__CXV54:

	MOVEM.L	A0/A1,-(SP)
	MOVE.L	D4,A0
	SWAP	D0
	MOVE.W	D0,D4
	AND.W	#$8000,D4
	EOR.W	D4,D0
	SUB.W	#$3800,D0
	CMP.W	#$10,D0
	BLT	lab098
	CMP.W	#$FEF,D0
	BLT	lab102
	CMP.W	#$47F0,D0
	BLT	lab058
	SWAP	D0
	LSL.L	#3,D0
	ROL.L	#3,D1
	AND.L	#7,D1
	EOR.L	D1,D0
	SWAP	D0
	OR.W	#$7F80,D0
	BRA	lab112

lab058:	CMP.W	#$FF0,D0
	BGE	lab074
	CMP.L	#$FFFF0FEF,D0
	BNE	lab102
	CMP.L	#$F0000000,D1
	BCS	lab102

lab074:	MOVEM.L	D0/D1/A0/A1,-(SP)
	PEA	2.L
	JSR	__CXFERR
	ADDQ.W	#4,SP
	MOVEM.L	(SP)+,D0/D1/A0/A1
	MOVE.L	#$7F80,D0
	EOR.W	D4,D0
	SWAP	D0
	MOVEQ	#0,D1
	BRA	lab116

lab098:	CMP.W	#$FE90,D0
	BGE	lab0C4
	ADD.W	#$3800,D0
	OR.L	D1,D0
	BEQ	lab112
	MOVEM.L	D0/D1/A0/A1,-(SP)
	PEA	1.L
	JSR	__CXFERR
	ADDQ.W	#4,SP
	MOVEM.L	(SP)+,D0/D1/A0/A1
	MOVEQ	#0,D0
	BRA	lab112

lab0C4:	MOVE.L	D5,A1
	MOVE.W	D0,D5
	AND.W	#15,D0
	EOR.W	#$10,D0
	SWAP	D0
	ASR.W	#4,D5
	ADDQ.W	#2,D5
	BGE	lab0E6
	NEG.W	D5
	LSR.L	D5,D0
	MOVEQ	#0,D5
	ADDX.L	D5,D0
	BRA	lab0F6

lab0E6:	CLR.W	D1
	LSL.L	D5,D0
	ADDQ.W	#1,D5
	ROXL.L	D5,D1
	AND.L	#15,D1
	ADDX.L	D1,D0
lab0F6:	MOVE.L	A1,D5
	SWAP	D0
	EOR.W	D4,D0
	SWAP	D0
	BRA	lab116

lab102:	SWAP	D0
	LSL.L	#3,D0
	ROXL.L	#4,D1
	AND.L	#7,D1
	ADDX.L	D1,D0
	SWAP	D0
lab112:	EOR.W	D4,D0
	SWAP	D0
lab116:	MOVE.L	A0,D4
	MOVEM.L	(SP)+,A0/A1
	RTS

	end
