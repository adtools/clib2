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

	xdef	__CXV52
	xdef	__CXV53

	xref	__CXFERR

	section	text,code

__CXV52:

	MOVEM.L	D2/D3,-(SP)
	MOVEQ	#-1,D3
	MOVE.L	D0,D2
	BPL.W	lab04A
	CMPI.L	#$BFF00000,D0
	BCS.W	lab07A
	MOVEM.L	D0/D1/A0/A1,-(SP)
	PEA	(2).L
	JSR	__CXFERR
	ADDQ.W	#4,SP
	MOVEM.L	(SP)+,D0/D1/A0/A1
	BRA.W	lab07A

__CXV53:

	MOVEM.L	D2/D3,-(SP)
	MOVE.L	#$7FFFFFFF,D3
	MOVE.L	D0,D2
	BPL.W	lab04A
	ADDQ.L	#1,D3
	EOR.L	D3,D0
lab04A:	SWAP	D0
	MOVE.W	D0,D2
	ANDI.W	#$7FF0,D2
	EOR.W	D2,D0
	SUBI.W	#$3FF0,D2
	BLT.W	lab07A
	EORI.W	#$10,D0
	SWAP	D0
	ASR.W	#4,D2
	SUBI.W	#$14,D2
	BGT.W	lab09A
	NEG.W	D2
	LSR.L	D2,D0
	TST.L	D2
	BMI.W	lab0B2
	BRA.W	lab0B4

lab07A:	MOVEQ	#0,D0
	BRA.W	lab0B4

lab080:	MOVEM.L	D0/D1/A0/A1,-(SP)
	PEA	(2).L
	JSR	__CXFERR
	ADDQ.W	#4,SP
	MOVEM.L	(SP)+,D0/D1/A0/A1
	MOVE.L	D3,D0
	BRA.W	lab0B4

lab09A:	CMPI.W	#11,D2
	BGT.B	lab080
	EOR.L	D1,D0
	ROL.L	D2,D0
	LSL.L	D2,D1
	EOR.L	D1,D0
	CMP.L	D3,D0
	BHI.B	lab080
	TST.L	D2
	BPL.W	lab0B4
lab0B2:	NEG.L	D0
lab0B4:	MOVEM.L	(SP)+,D2/D3
	RTS

	end
