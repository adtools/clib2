//
//	$Id: crt0.S,v 1.4 2005-08-20 14:25:20 obarthel Exp $
//
//	:ts=4
//

	.text

	.globl  main
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
