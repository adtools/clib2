*
*	$Id: sas_cxv45.asm,v 1.1.1.1 2004-07-26 16:31:04 obarthel Exp $
*
*	:ts=8
*
*	Adapted from reassembled SAS/C runtime library code.
*

	section	text,code

	xdef	__CXV45

__CXV45
	MOVE.L	D0,D1
	SWAP	D1
	AND.W	#$7FFF,D1
	CMP.W	#$80,D1
	BLT	lbC00003E
	CMP.W	#$7F80,D1
	BGE	lbC000060
	ASR.L	#3,D0
	AND.L	#$8FFFFFFF,D0
	ADD.L	#$38000000,D0
	SWAP	D1
	AND.L	#7,D1
	ROR.L	#3,D1
lbC00003C
	RTS

lbC00003E
	TST.L	D1
	BEQ.S	lbC00003C
	MOVEM.L	D2-D5,-(SP)
	SWAP	D0
	MOVE.W	D0,D4
	AND.W	#$8000,D4
	MOVE.W	#$39D0,D5
	MOVEQ	#0,D0
	SWAP	D1
	JSR	__CXNRM5(PC)
	MOVEM.L	(SP)+,D2-D5
	RTS

lbC000060
	ASR.L	#3,D0
	OR.L	#$7FF00000,D0
	SWAP	D1
	AND.L	#7,D1
	ROR.L	#3,D1
	RTS

__CXNRM5
	CMP.L	#$20,D0
	BGE	lbC0000B0
	SWAP	D0
	SWAP	D1
	MOVE.W	D1,D0
	CLR.W	D1
	SUB.W	#$100,D5
	BGE.S	__CXNRM5
	BRA	lbC0000F4

__CXTAB5
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

lbC0000B0
	MOVEQ	#0,D3
	CMP.L	#$2000,D0
	BGE	lbC0000C0
	LSL.L	#8,D0
	ADDQ.W	#8,D3
lbC0000C0
	SWAP	D0
	TST.W	D0
	BNE	lbC0000CC
	ROL.L	#4,D0
	ADDQ.W	#4,D3
lbC0000CC
	MOVEQ	#0,D2
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
	BLT	lbC0000F4
	SWAP	D0
	ADD.W	D5,D0
	OR.W	D4,D0
	SWAP	D0
	RTS

lbC0000F4
	NEG.W	D5
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
