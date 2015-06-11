//
// $Id$
//
// :ts=4
//
// Portable ISO 'C' (1994) runtime library for the Amiga computer
// Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   - Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//   - Neither the name of Olaf Barthel nor the names of contributors
//     may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

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

