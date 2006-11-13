
#if defined(__PPC__) && defined(__GNUC__)

asm("	.text\n\
	.align 2\n\
	.globl bswap24\n\
	.type	bswap24, @function\n\
bswap32:\n\
	rlwinm	%r4,%r3,16,8,31\n\
	rlwimi	%r4,%r3,0,16,24\n\
	or			%r3,%r4,%r4\n\
	blr\n\
");

#else

#include <stdint.h>

uint32_t bswap24(uint32_t u32)
{
	return(
		((u32&0xff)<<16)|
		((u32&0xff00))|
		((u32&0xff0000)>>16)
	);
}

#endif

