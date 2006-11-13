
#if defined(USE_64_BIT_INTS)

#if defined(__PPC__) && defined(__GNUC__)

asm("	.text\n\
	.align 2\n\
	.globl bswap64\n\
	.type	bswap64, @function\n\
bswap64:\n\
	rlwinm	%r5,%r3,8,8,31\n\
	rlwimi	%r5,%r3,24,0,7\n\
	rlwimi	%r5,%r3,24,16,23\n\
	rlwinm	%r3,%r4,8,8,31\n\
	rlwimi	%r3,%r4,24,0,7\n\
	rlwimi	%r3,%r4,24,16,23\n\
	or			%r4,%r5,%r5\n\
	blr\n\
");

#else

#include <stdint.h>

uint64_t bswap64(uint64_t u64)
{
union {
	uint64_t ll;
	uint32_t l[2];
} v={.ll=u64};
uint32_t tmp;
tmp=v.l[0];
v.l[0]=((v.l[1]&0xff)<<24)|
		((v.l[1]&0xff00)<<8)|
		((v.l[1]&0xff0000)>>8)|
		((v.l[1]&0xff000000)>>24);
v.l[1]=((tmp&0xff)<<24)|
		((tmp&0xff00)<<8)|
		((tmp&0xff0000)>>8)|
		((tmp&0xff000000)>>24);
return(v.ll);
}

#endif

#endif


