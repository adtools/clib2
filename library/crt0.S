//
//	$Id: crt0.S,v 1.1.1.1 2004-07-26 16:30:22 obarthel Exp $
//
//	:ts=4
//

	.text

	.globl  _main
	.globl  _start

_start:

#if defined(SMALL_DATA)
	/* Don't do a function call here, since that
	   means LR has to be saved */
	lis r13, _SDA_BASE_@ha
	addi r13, r13, _SDA_BASE_@l
#endif

	b	_main

