
#if defined(__GNUC__) && defined(__PPC__)

/* r3=from, r4=to, r5=len, r6=index, r7=load/store temp */

asm("\
	.text\n\
	.align 2\n\
	.globl swab32\n\
	.type swab32,@function\n\
swab32:\n\
	srawi.	%r5,%r5,2\n\
	li			%r6,0\n\
	bc			4,gt,.exit\n\
	mtctr		%r5\n\
.loop:\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	bc			0,lt,.loop\n\
.exit:\n\
	or			%r3,%r4,%r4\n\
	blr\n\
");

/* r3=from, r4=to, r5=len/temp, r6=index, r7=load/store temp, r8=cache hint
 *
 * The unrolled, cache-hinting version appears to be about 4.5% faster, but
 * in this case I opted for the smaller implementation. swab64() appears to
 * gain more from cache-hinting - probably because of it using more registers
 * for intermediate storage.
asm("\
	.text\n\
	.align 2\n\
	.globl swab32\n\
	.type swab32,@function\n\
swab32:\n\
	dcbt		0,%r3\n\
	andi.		%r8,%r5,31		# The number of bytes handled in '.pre'. Used for prefetch hint.\n\
	srawi		%r5,%r5,2		# Convert bytes-># of 32-bit words\n\
	andi.		%r7,%r5,7\n\
	li			%r6,0\n\
	bc			4,gt,.preploop\n\
	mtctr		%r7\n\
.pre:								# One 32-bit word at a time until we have (nLeft%8)==0 \n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	bc			0,lt,.pre\n\
.preploop:\n\
	srawi.	%r5,%r5,3		# Divide by 8 again to get number of loops.\n\
	addi		%r8,%r8,32		# Start address for next loop (from r3).\n\
	bc			4,gt,.exit\n\
	mtctr		%r5\n\
.loop:							# Loop unrolled 8 times = 32 bytes = 1 cache-line (except on the 970).\n\
	dcbt		%r8,%r3			# Cache hint (prefetch) for the next loop\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	lwbrx		%r7,%r6,%r3\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	addi		%r8,%r8,32		# Update cache-hint offset\n\
	bc			0,lt,.loop\n\
.exit:\n\
	or			%r3,%r4,%r4\n\
	blr\n\
");
*/

#else

#include <sys/types.h>
#include <stdint.h>

void *swab32(void *from,void *to,ssize_t len)
{
int i;
uint32_t *u32in=from,*u32out=to,tmp;

for(i=0;i<(len>>2);i++) {
	tmp=u32in[i];
	u32out[i]=((tmp&0xff)<<24)|
		((tmp&0xff00)<<8)|
		((tmp&0xff0000)>>8)|
		((tmp&0xff000000)>>24);
}

return(to);
}

#endif


