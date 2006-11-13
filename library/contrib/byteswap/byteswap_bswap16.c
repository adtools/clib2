
#if defined(__PPC__) && defined(__GNUC__)

asm("\
	.text\n\
	.align 2\n\
	.globl bswap16\n\
	.type	bswap16, @function\n\
bswap16:\n\
#	rlwinm	%r4,%r3,8,16,24\n\
#	rlwimi	%r4,%r3,24,24,31\n\
#	or			%r3,%r4,%r4\n\
	rlwimi	%r3,%r3,16,8,15\n\
	srwi		%r3,%r3,8\n\
	blr\n\
");

#else

#include <stdint.h>

uint16_t bswap16(uint16_t u16)
{
return(u16>>8|u16<<8);
}

#endif

