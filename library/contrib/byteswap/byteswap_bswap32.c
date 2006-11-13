
#if defined(__PPC__) && defined(__GNUC__)

asm("	.text\n\
	.align 2\n\
	.globl bswap32\n\
	.type	bswap32, @function\n\
bswap32:\n\
	rlwinm	%r4,%r3,8,8,31\n\
	rlwimi	%r4,%r3,24,0,7\n\
	rlwimi	%r4,%r3,24,16,23\n\
	or			%r3,%r4,%r4\n\
	blr\n\
");

#else

#include <stdint.h>

uint32_t bswap32(uint32_t u32)
{
	return(
		((u32&0xff)<<24)|
		((u32&0xff00)<<8)|
		((u32&0xff0000)>>8)|
		((u32&0xff000000)>>24)
	);
}

#endif

