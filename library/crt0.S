//
//	$Id: crt0.S,v 1.2 2004-11-28 12:31:27 tfrieden Exp $
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

#if defined(BASEREL_DATA)
	/* We also need to set up the data segment pointer */
	lis r2, _DATA_BASE_@ha
	addi r2, r2, _DATA_BASE_@l
#endif

	b	_main

#if defined(BASEREL_DATA)
	.section .rodata

	.globl __data_segment_descriptor
__data_segment_descriptor:
	.long __data_start
	.long __data_end
	.long _DATA_BASE_

	.section .dend, "wa", @nobits
	.space 	4
#endif
