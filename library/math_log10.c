/*
 * $Id: math_log10.c,v 1.5 2005-02-25 10:14:21 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * PowerPC math library based in part on work by Sun Microsystems
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(__GNUC__)

/****************************************************************************/

#if defined(SMALL_DATA)
#define A4(x) "a4@(" #x ":W)"
#elif defined(SMALL_DATA32)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) #x
#endif /* SMALL_DATA */

/****************************************************************************/

extern double __log10(double x);

/****************************************************************************/

asm("

	.text
	.even

	.globl	_MathIeeeDoubTransBase
	.globl	___log10

___log10:

	movel	a6,sp@-
	movel	"A4(_MathIeeeDoubBasBase)",a6
	moveml	sp@(8),d0/d1
	jsr		a6@(-126:W)
	movel	sp@+,a6
	rts

");

/****************************************************************************/

#else

/****************************************************************************/

INLINE STATIC const double
__log10(double x)
{
	double result;

	result = IEEEDPLog10(x);

	return(result);
}

/****************************************************************************/

#endif /* __GNUC__ */

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__log10(double x)
{
	double result;

	__asm ("flog10%.x %1,%0"
	       : "=f" (result)
	       : "f" (x));

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

static const double
two54      =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
ivln10     =  4.34294481903251816668e-01, /* 0x3FDBCB7B, 0x1526E50E */
log10_2hi  =  3.01029995663611771306e-01, /* 0x3FD34413, 0x509F6000 */
log10_2lo  =  3.69423907715893078616e-13, /* 0x3D59FEF3, 0x11F12B36 */
zero       =  0.0;

INLINE STATIC const double
__log10(double x)
{
	double y,z;
	int i,k,hx;
	unsigned int lx;

	EXTRACT_WORDS(hx,lx,x);

	k=0;
	if (hx < 0x00100000)                    /* x < 2**-1022  */
	{
		if (((hx&0x7fffffff)|lx)==0)
			return -two54/zero;             /* log(+-0)=-inf */
		if (hx<0) 
			return (x-x)/zero;              /* log(-#) = NaN */
		k -= 54; x *= two54;                /* subnormal number, scale up x */
	    GET_HIGH_WORD(hx,x);
	}

	if (hx >= 0x7ff00000)
		return x+x;

	k += (hx>>20)-1023;
	i  = ((unsigned int)k&0x80000000)>>31;
	hx = (hx&0x000fffff)|((0x3ff-i)<<20);
	y  = (double)(k+i);
	SET_HIGH_WORD(x,hx);
	z  = y*log10_2lo + ivln10*log(x);

	return  z+y*log10_2hi;
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
log10(double x)
{
	double result;

	if(x > DBL_EPSILON)
	{
		result = __log10(x);
	}
	else
	{
		__set_errno(ERANGE);

		/* This should really be minus infinity. */
		result = (-__get_huge_val());
	}

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
