
#if defined(__GNUC__) && defined(__PPC__)

/* r3=from, r4=to, r5=len/count, r6=index, r7=load/store/temp */

asm("\
	.text\n\
	.align 2\n\
	.globl swab\n\
	.type swab,@function\n\
swab:\n\
	dcbt		0,%r3\n\
	srawi.	%r5,%r5,1\n\
	bc			4,gt,.exit\n\
	andi.		%r7,%r3,3		# Check if we start on an address evenly divisible by 4.\n\
	li			%r6,0\n\
	bc			4,gt,.preploop\n\
	lhbrx		%r7,%r6,%r3		# Fix alignment if needed.\n\
	sthx		%r7,%r6,%r4\n\
	addi		%r6,%r6,2\n\
	subi		%r5,%r5,1\n\
.preploop:\n\
	andi.		%r7,%r5,1		# Check if even or odd number of 16-bit words.\n\
	srawi		%r5,%r5,1		# Number of 32-bit words to half-swap.\n\
	mtctr		%r5\n\
	bc			12,gt,.oddloop	# Jump to loop for odd number of 16-bit words.\n\
.loop:							# Loop is 'unrolled' by reading/writing 32-bit words.\n\
	lwbrx		%r7,%r6,%r3\n\
	rotlwi	%r7,%r7,16\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	bc			0,lt,.loop\n\
.exit:\n\
	or			%r3,%r4,%r4\n\
	blr\n\
.oddloop:\n\
	lwbrx		%r7,%r6,%r3\n\
	rotlwi	%r7,%r7,16\n\
	stwx		%r7,%r6,%r4\n\
	addi		%r6,%r6,4\n\
	bc			0,lt,.oddloop\n\
	sub		%r6,%r6,2\n\
	lhbrx		%r7,%r6,%r3		# Fix last 16-bit word.\n\
	sthx		%r7,%r6,%r4\n\
	or			%r3,%r4,%r4\n\
	blr\n\
");

#else

#include <sys/types.h>
#include <stdint.h>

void *swab(void *from,void *to,ssize_t len)
{
int i;
uint16_t u16,*u16in=from,*u16out=to;

for(i=0;i<(len>>1);i++) {
	u16=u16in[i];
	u16out[i]=u16>>8|u16<<8;
}

return(u16out);
}

#endif



