    .globl	_mcount
    .type	_mcount,@function
_mcount:
	stwu	r1,-64(r1)
	stw	    r3,16(r1)
	stw	    r4,20(r1)
	stw	    r5,24(r1)
	stw	    r6,28(r1)
	stw	    r7,32(r1)
	stw	    r8,36(r1)
	stw	    r9,40(r1)
	stw	    r10,44(r1)

	mflr	r4
	stw	    r4,48(r1)
	lwz	    r3,68(r1)
	
	bl	    __mcount
	lwz	    r3,68(r1)
	mtlr	r3
	lwz	    r4,48(r1)
	mtctr	r4

	lwz	    r3,16(r1)
	lwz	    r4,20(r1)
	lwz	    r5,24(r1)
	lwz	    r6,28(r1)
	lwz	    r7,32(r1)
	lwz	    r8,36(r1)
	lwz	    r9,40(r1)
	lwz	    r10,44(r1)
	addi	r1,r1,64
	bctr
_mcount_end:
	.size	_mcount,_mcount_end-_mcount