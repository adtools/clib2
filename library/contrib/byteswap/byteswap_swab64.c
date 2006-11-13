
#if defined(__GNUC__) && defined(__PPC__)

/* r3=from, r4=to, r5=len/temp, r6/r7=index, r8/r9=load/store temp, r10=cache hint */

/* This version is unrolled and uses cache-hinting. It appears to gain about 10%
 * over a non-unrolled, non-hinting version.
 */

asm("\
	.text\n\
	.align 2\n\
	.globl swab64\n\
	.type swab64,@function\n\
swab64:\n\
	dcbt		0,%r3\n\
	andi.		%r10,%r5,31		# The number of bytes handled in '.pre'. Used for prefetch hint.\n\
	srawi		%r5,%r5,3		# Convert bytes-># of 64-bit words\n\
	andi.		%r7,%r5,3\n\
	li			%r6,0\n\
	bc			4,gt,.preploop\n\
	mtctr		%r7\n\
.pre:								# One 64-bit word at a time until we have (nLeft%4)==0 \n\
	lwbrx		%r8,%r6,%r3\n\
	addi		%r7,%r6,4\n\
	lwbrx		%r9,%r7,%r3\n\
	stwx		%r8,%r7,%r4\n\
	stwx		%r9,%r6,%r4\n\
	addi		%r6,%r6,8\n\
	bc			0,lt,.pre\n\
.preploop:\n\
	srawi.	%r5,%r5,2		# Divide by 4 again to get number of loops.\n\
	addi		%r10,%r10,32	# Start address for next loop.\n\
	bc			4,gt,.exit\n\
	mtctr		%r5\n\
.loop:							# Loop unrolled 4 times = 32 bytes = 1 cache-line (except on the 970).\n\
	dcbt		%r10,%r3			# Cache hint (prefetch) for the next iteration\n\
	lwbrx		%r8,%r6,%r3\n\
	addi		%r7,%r6,4\n\
	lwbrx		%r9,%r7,%r3\n\
	stwx		%r8,%r7,%r4\n\
	stwx		%r9,%r6,%r4\n\
	addi		%r6,%r6,8\n\
	lwbrx		%r8,%r6,%r3\n\
	addi		%r7,%r6,4\n\
	lwbrx		%r9,%r7,%r3\n\
	stwx		%r8,%r7,%r4\n\
	stwx		%r9,%r6,%r4\n\
	addi		%r6,%r6,8\n\
	lwbrx		%r8,%r6,%r3\n\
	addi		%r7,%r6,4\n\
	lwbrx		%r9,%r7,%r3\n\
	stwx		%r8,%r7,%r4\n\
	stwx		%r9,%r6,%r4\n\
	addi		%r6,%r6,8\n\
	lwbrx		%r8,%r6,%r3\n\
	addi		%r7,%r6,4\n\
	lwbrx		%r9,%r7,%r3\n\
	stwx		%r8,%r7,%r4\n\
	stwx		%r9,%r6,%r4\n\
	addi		%r6,%r6,8\n\
	addi		%r10,%r10,32		# Update cache-hint offset\n\
	bc			0,lt,.loop\n\
.exit:\n\
	or			%r3,%r4,%r4\n\
	blr\n\
");

#else

#include <sys/types.h>
#include <stdint.h>

void *swab64(void *from,void *to,ssize_t len)
{
int i;
struct {
	uint32_t	u32[2];
} *u64in=from,*u64out=to;
uint32_t tmp1,tmp2;

for(i=0;i<(len>>3);i++) {
	tmp1=u64in[i].u32[0];
	tmp2=u64in[i].u32[1];
	u64out[i].u32[0]=((tmp2&0xff)<<24)|
		((tmp2&0xff00)<<8)|
		((tmp2&0xff0000)>>8)|
		((tmp2&0xff000000)>>24);
	u64out[i].u32[1]=((tmp1&0xff)<<24)|
		((tmp1&0xff00)<<8)|
		((tmp1&0xff0000)>>8)|
		((tmp1&0xff000000)>>24);
}

return(to);
}

#endif

/* vi:set ts=3: */

