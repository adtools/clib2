|
|	$Id: nrcrt0.S,v 1.4 2005-08-26 12:39:33 obarthel Exp $
|
|	:ts=4
|
|	Portable ISO 'C' (1994) runtime library for the Amiga computer
|	Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
|	All rights reserved.
|
|	Redistribution and use in source and binary forms, with or without
|	modification, are permitted provided that the following conditions
|	are met:
|
|	  - Redistributions of source code must retain the above copyright
|	    notice, this list of conditions and the following disclaimer.
|
|	  - Neither the name of Olaf Barthel nor the names of contributors
|	    may be used to endorse or promote products derived from this
|	    software without specific prior written permission.
|
|	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
|	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
|	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
|	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
|	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
|	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
|	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
|	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
|	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
|	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
|	POSSIBILITY OF SUCH DAMAGE.
|

|-----------------------------------------------------------------------------

LVOAllocMem	= -0xc6
LVOFreeMem	= -0xd2
LVOFindTask	= -0x126
LVOCopyMem	= -0x270

|-----------------------------------------------------------------------------

pr_Result2	= 148
pr_CLI		= 172

|-----------------------------------------------------------------------------

RETURN_FAIL			= 20
ERROR_NO_FREE_STORE	= 103

|-----------------------------------------------------------------------------

MEMF_CLEAR = 65536

|-----------------------------------------------------------------------------

	.text

	.globl  _main			| This enforces linkage against the main() function
	.globl	__main
	.globl	___is_resident

|-----------------------------------------------------------------------------

	movel	4:W,a6

	subal	a1,a1
	jsr		a6@(LVOFindTask:W)

	movel	d0,a2
	tstl	a2@(pr_CLI:W)
	bnes	from_shell

|-----------------------------------------------------------------------------

	lea		___a4_init,a4

	movel	#___bss_size,d1
	beqs	no_bss

#if defined(SMALL_DATA)
	lea		a4@(__edata:W),a1
#endif /* SMALL_DATA */

#if defined(SMALL_DATA32)
	lea		a4@(__edata:L),a1
#endif /* SMALL_DATA32 */

clear_bss:

	clrl	a1@+
	subql	#4,d1
	bnes	clear_bss

no_bss:

	jmp		__main

|-----------------------------------------------------------------------------

from_shell:

	movel	#___data_size,d5
	movel	#___bss_size,d0
	addl	d5,d0
	movel	#MEMF_CLEAR,d1
	jsr		a6@(LVOAllocMem:W)
	tstl	d0
	beqs	fail

	movel	d0,a4

	lea		___a4_init-0x7ffe,a5
	movel	d5,d0
	movel	a5,a0
	movel	a4,a1
	jsr		a6@(LVOCopyMem:W)

	lea		___datadata_relocs,a0
	movel	a0@+,d0
	beqs	no_reloc

	movel	a5,d1
	subl	a4,d1

do_reloc:

	movel	a0@+,a1
	subl	d1,a4@(a1:l)
	subql	#1,d0
	bnes	do_reloc

no_reloc:

	lea		a4@(0x7ffe:W),a4

#if defined(SMALL_DATA)
	lea		a4@(___is_resident:W),a2
#endif /* SMALL_DATA */

#if defined(SMALL_DATA32)
	lea		a4@(___is_resident:L),a2
#endif /* SMALL_DATA32 */

	movew	#1,a2@

	jbsr	__main
	movel	d0,d2

	movel	4:W,a6
	movel	#___data_size,d0
	addl	#___bss_size,d0
	lea		a4@(-0x7ffe:W),a1
	jsr		a6@(LVOFreeMem:W)

	movel	d2,d0

	rts

fail:

	movel	#ERROR_NO_FREE_STORE,a2@(pr_Result2:W)
	moveql	#RETURN_FAIL,d0

	rts

|-----------------------------------------------------------------------------

	.globl	___get_a4

___get_a4:

	movel	a4,d0
	rts

|-----------------------------------------------------------------------------

	.globl	___set_a4

___set_a4:

	movel	sp@(4),a4
	rts
