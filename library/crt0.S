//
// $Id: crt0.S,v 1.6 2005-10-11 09:28:29 obarthel Exp $
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

	.text

	.globl  main	/* This enforces linkage against the main() function */
	.globl  _main
	.globl  _start

_start:

#if defined(SMALL_DATA)
	/* Don't do a function call here, since that
	   means LR has to be saved */
	lis r13, _SDA_BASE_@ha
	addi r13, r13, _SDA_BASE_@l
#endif

#if defined(BASEREL_DATA)
	/* We also need to set up the data segment pointer */
	lis r2, _DATA_BASE_@ha
	addi r2, r2, _DATA_BASE_@l
#endif

	b	_main
