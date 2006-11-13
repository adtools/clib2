
#ifndef	__BYTESWAP_H
#define	__BYTESWAP_H

#include <sys/types.h>
#include <stdint.h>

#if defined(__GNUC__)
#define	__CONST_FUNC	__attribute__((const))
#else
#define	__CONST_FUNC	/* Nothing */
#endif

/* Single value byteswap functions. */

extern __CONST_FUNC uint16_t bswap16(uint16_t);
extern __CONST_FUNC uint32_t bswap24(uint32_t);
extern __CONST_FUNC uint32_t bswap32(uint32_t);

#ifdef	INT64_MIN
extern __CONST_FUNC uint64_t bswap64(uint64_t);
#endif

/* Block byteswap functions. The swab() function usually resides in unistd.h, so perhaps it should be moved there? */
/* NOTE: Contrary to the standard swab(), this version returns the "to" pointer and the pointers are not restrict
 * qualified - so swapping buffer-contents in-place is supported.
 * Also, swab24(), swab32() and swab64() are non-standard functions.
 */

extern void *swab(void *from,void *to,ssize_t nbytes);
extern void *swab24(void *from,void *to,ssize_t nbytes);	/* Same as swab(), but operates on 24-bit words instead. */
extern void *swab32(void *from,void *to,ssize_t nbytes);	/* Same as swab(), but operates on 32-bit words instead. */
extern void *swab64(void *from,void *to,ssize_t nbytes);	/* Same as swab(), but operates on 64-bit words instead. */

#define	swab16(x)	swab(x)

/*
 * Optimized inline-versions for the single-value functions follow.
 * Only GCC+PPC and GCC+m68k support for now.
 */

#if defined(__GNUC__)

/* Select implementation. */

#define	bswap16(x)	(__builtin_constant_p(x))?__const_swap16(x):__swap16(x)
#define	bswap24(x)	(__builtin_constant_p(x))?__const_swap24(x):__swap24(x)
#define	bswap32(x)	(__builtin_constant_p(x))?__const_swap32(x):__swap32(x)
#define	bswap64(x)	(__builtin_constant_p(x))?__const_swap64(x):__swap64(x)

/* Assembler implementations */

#if defined(__PPC__)

static __inline__  __CONST_FUNC uint16_t __swap16(uint16_t u16) {
	uint_fast16_t result;
	__asm__("\
		rlwinm	%[result],%[u16],8,16,24\n\
		rlwimi	%[result],%[u16],24,24,31\n\
	":[result]"=&r"(result):[u16]"r"(u16));
	return(result);
}

static __inline__ __CONST_FUNC uint32_t __swap24(uint32_t u32) {
	uint_fast32_t result;
	__asm__("\
	rlwinm	%[result],%[u32],16,8,31\n\
	rlwimi	%[result],%[u32],0,16,24\n\
	":[result]"=&r"(result):[u32]"r"(u32));
	return(result);
}

static __inline__ __CONST_FUNC uint32_t __swap32(uint32_t u32) {
	uint_fast32_t result;
	__asm__("\
	rlwinm	%[result],%[u32],8,8,31\n\
	rlwimi	%[result],%[u32],24,0,7\n\
	rlwimi	%[result],%[u32],24,16,23\n\
	":[result]"=&r"(result):[u32]"r"(u32));
	return(result);
}

/*
 * Note: __swap64() might perhaps be optimized a bit more by scheduling the
 * instructions to alternate register-use, but this instead means there
 * are two less registers free since "u64" and "result" may no longer overlap.
 * Decisions, decisions....
 */

static __inline__ __CONST_FUNC uint64_t __swap64(uint64_t u64) {
	uint_fast64_t result;
	uint_fast32_t tmp;
	__asm__("\
	rlwinm	%[tmp],%[u64],8,8,31\n\
	rlwimi	%[tmp],%[u64],24,0,7\n\
	rlwimi	%[tmp],%[u64],24,16,23\n\
	rlwinm	%[result],%L[u64],8,8,31\n\
	rlwimi	%[result],%L[u64],24,0,7\n\
	rlwimi	%[result],%L[u64],24,16,23\n\
	or			%L[result],%[tmp],%[tmp]\n\
	":[result]"=r"(result),[tmp]"=&r"(tmp):[u64]"r"(u64));
	return(result);
}

#elif defined(__mc68020__)

static __inline__  __CONST_FUNC uint16_t __swap16(uint16_t u16) {
	__asm__("\
		rol.w	#8,%[u16]\n\
	":[u16]"+d"(u16)::"cc");
	return(u16);
}

static __inline__ __CONST_FUNC uint32_t __swap24(uint32_t u32) {
	__asm__("\
		rol.w	#8,%[u32]\n\
		swap	%[u32]\n\
		rol.w	#8,%[u32]\n\
		ror.l	#8,%[u32]\n\
	":[u32]"+d"(u32)::"cc");
	return(u32);
}

static __inline__ __CONST_FUNC uint32_t __swap32(uint32_t u32) {
	__asm__("\
		rol.w	#8,%[u32]\n\
		swap	%[u32]\n\
		rol.w	#8,%[u32]\n\
	":[u32]"+d"(u32)::"cc");
	return(u32);
}

static __inline__ __CONST_FUNC uint64_t __swap64(uint64_t u64) {
	__asm__("\
		rol.w	#8,%[u64]\n\
		rol.w	#8,%L[u64]\n\
		swap	%[u64]\n\
		swap	%L[u64]\n\
		rol.w	#8,%[u64]\n\
		rol.w	#8,%L[u64]\n\
		exg	%[u64],L%[u64]\n\
	":[u64]"+d"(u64)::"cc");
	return(u64);
}

#else
/* Unknown or undefined architecture. Perhaps compiling with "-strict -ansi", but should not use this header then anyway. */
#undef	bswap16
#undef	bswap24
#undef	bswap32
#undef	bswap64
#define	bswap16(x)	(__builtin_constant_p(x))?__const_swap16(x):bswap16(x)
#define	bswap24(x)	(__builtin_constant_p(x))?__const_swap24(x):bswap24(x)
#define	bswap32(x)	(__builtin_constant_p(x))?__const_swap32(x):bswap32(x)
#define	bswap64(x)	(__builtin_constant_p(x))?__const_swap64(x):bswap64(x)
#endif

/* C implementations for constant values */

static __inline__ uint16_t __const_swap16(uint16_t u16) {
	return(u16>>8|u16<<8);
}

static __inline__ uint32_t __const_swap24(uint32_t u32) {
	return(((u32&0xff)<<16)|((u32&0xff00))|((u32&0xff0000)>>16));
}

static __inline__ uint32_t __const_swap32(uint32_t u32) {
	return(((u32&0xff)<<24)|((u32&0xff00)<<8)|((u32&0xff0000)>>8)|((u32&0xff000000)>>24));
}

static __inline__ uint64_t __const_swap64(uint64_t u64) {
	return(((u64&0xffLL)<<56)|((u64&0xff00LL)<<40)|((u64&0xff0000LL)<<24)|((u64&0xff000000LL)<<8)|
		((u64&0xff00000000LL)>>8)|((u64&0xff0000000000LL)>>24)|((u64&0xff000000000000LL)>>40)|((u64&0xff00000000000000LL)>>56));
}

#endif	/* __GNUC__ */


#endif	/* __BYTESWAP_H */

/* vi:set ts=3: */

