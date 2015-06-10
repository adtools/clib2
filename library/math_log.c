/*
 * $Id: math_log.c,v 1.10 2007-11-08 11:23:53 damato Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

extern double __log(double x);

/****************************************************************************/

asm("

	.text
	.even

	.globl	_MathIeeeDoubTransBase
	.globl	___log

___log:

	movel	a6,sp@-
	movel	"A4(_MathIeeeDoubTransBase)",a6
	moveml	sp@(8),d0/d1
	jsr		a6@(-84:W)
	movel	sp@+,a6
	rts

");

/****************************************************************************/

#else

/****************************************************************************/

INLINE STATIC const double
__log(double x)
{
	double result;

	result = IEEEDPLog(x);

	return(result);
}

/****************************************************************************/

#endif /* __GNUC__ */

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__log(double x)
{
	double result;

	__asm ("flogn%.x %1,%0"
	       : "=f" (result)
	       : "f" (x));

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(__PPC__)

static const double
ln2_hi  =  6.93147180369123816490e-01,	/* 3fe62e42 fee00000 */
ln2_lo  =  1.90821492927058770002e-10,	/* 3dea39ef 35793c76 */
two54   =  1.80143985094819840000e+16,  /* 43500000 00000000 */
Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
Lg7 = 1.479819860511658591e-01,  /* 3FC2F112 DF3E5244 */
zero = 0.0;

INLINE STATIC double
__log(double x)
{
	double hfsq,f,s,z,R,w,t1,t2,dk;
	int k,hx,i,j;
	unsigned int lx;

	EXTRACT_WORDS(hx,lx,x);

	k=0;
	if (hx < 0x00100000)          			/* x < 2**-1022  */
	{
	    if (((hx&0x7fffffff)|lx)==0) 
			return -two54/zero;		/* log(+-0)=-inf */
	    if (hx<0) 
			return (x-x)/zero;	/* log(-#) = NaN */
	    k -= 54; 
		x *= two54; /* subnormal number, scale up x */
	    GET_HIGH_WORD(hx,x);
	} 

	if (hx >= 0x7ff00000) 
		return x+x;
	
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;

	SET_HIGH_WORD(x,hx|(i^0x3ff00000));	/* normalize x or x/2 */

	k += (i>>20);
	f = x-1.0;

	if((0x000fffff&(2+hx))<3) 	      /* |f| < 2**-20 */
	{
		if(f==zero) 
		{ 
			if(k==0) 
				return zero;  
			else 
			{
				dk=(double)k;
				return dk*ln2_hi+dk*ln2_lo;
			}
		}

	    R = f*f*(0.5-0.33333333333333333*f);

	    if(k==0) 
			return f-R; 
		else 
		{
			dk=(double)k;
			return dk*ln2_hi-((R-dk*ln2_lo)-f);
		}
	}

 	s = f/(2.0+f); 
	dk = (double)k;
	z = s*s;
	i = hx-0x6147a;
	w = z*z;
	j = 0x6b851-hx;
	t1= w*(Lg2+w*(Lg4+w*Lg6)); 
	t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7))); 
	i |= j;
	R = t2+t1;

	if(i>0) 
	{
	    hfsq=0.5*f*f;
	    if(k==0) 
			return f-(hfsq-s*(hfsq+R)); 
		else
			return dk*ln2_hi-((hfsq-(s*(hfsq+R)+dk*ln2_lo))-f);
	}
	else 
	{
	    if(k==0) 
			return f-s*(f-R); 
		else
			return dk*ln2_hi-((s*(f-R)-dk*ln2_lo)-f);
	}
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
log(double x)
{
	double result;

	if(x > 0)
	{
		result = __log(x);
	}
	else
	{
		__set_errno(ERANGE);

		result = -__inf();
	}

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
