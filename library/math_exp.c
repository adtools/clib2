/*
 * $Id: math_exp.c,v 1.6 2006-01-08 12:04:23 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

extern double __exp(double x);

/****************************************************************************/

asm("

	.text
	.even

	.globl	_MathIeeeDoubTransBase
	.globl	___exp

___exp:

	movel	a6,sp@-
	movel	"A4(_MathIeeeDoubTransBase)",a6
	moveml	sp@(8),d0/d1
	jsr		a6@(-78:W)
	movel	sp@+,a6
	rts

");

/****************************************************************************/

#else

/****************************************************************************/

INLINE STATIC const double
__exp(double x)
{
	double result;

	result = IEEEDPExp(x);

	return(result);
}

/****************************************************************************/

#endif /* __GNUC__ */

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__exp(double x)
{
	double result;

	__asm ("fetox%.x %1,%0"
	       : "=f" (result)
	       : "f" (x));

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

static const double
one	= 1.0,
halF[2]	= {0.5,-0.5,},
huge	= 1.0e+300,
twom1000= 9.33263618503218878990e-302,     /* 2**-1000=0x01700000,0*/
o_threshold=  7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
u_threshold= -7.45133219101941108420e+02,  /* 0xc0874910, 0xD52D3051 */
ln2HI[2]   ={ 6.93147180369123816490e-01,  /* 0x3fe62e42, 0xfee00000 */
	     -6.93147180369123816490e-01,},    /* 0xbfe62e42, 0xfee00000 */
ln2LO[2]   ={ 1.90821492927058770002e-10,  /* 0x3dea39ef, 0x35793c76 */
	     -1.90821492927058770002e-10,},    /* 0xbdea39ef, 0x35793c76 */
invln2 =  1.44269504088896338700e+00,      /* 0x3ff71547, 0x652b82fe */
P1   =  1.66666666666666019037e-01,        /* 0x3FC55555, 0x5555553E */
P2   = -2.77777777770155933842e-03,        /* 0xBF66C16C, 0x16BEBD93 */
P3   =  6.61375632143793436117e-05,        /* 0x3F11566A, 0xAF25DE2C */
P4   = -1.65339022054652515390e-06,        /* 0xBEBBBD41, 0xC5D26BF1 */
P5   =  4.13813679705723846039e-08;        /* 0x3E663769, 0x72BEA4D0 */

INLINE STATIC double
__exp(double x)
{
	double y,hi,lo,c,t;
	int k,xsb;
	unsigned int hx;

	GET_HIGH_WORD(hx,x);
	xsb = (hx>>31)&1;		/* sign bit of x */
	hx &= 0x7fffffff;		/* high word of |x| */

    /* filter out non-finite argument */
	if(hx >= 0x40862E42) 
	{			/* if |x|>=709.78... */
        if(hx>=0x7ff00000) 
		{
			unsigned int lx;

			GET_LOW_WORD(lx,x);
			if(((hx&0xfffff)|lx)!=0) 
				return x+x; 		/* NaN */
			else 
				return (xsb==0)? x:0.0;	/* exp(+-inf)={inf,0} */
	    }
	    if(x > o_threshold) 
			return huge*huge; /* overflow */
	    if(x < u_threshold) 
			return twom1000*twom1000; /* underflow */
	}

    /* argument reduction */
	if(hx > 0x3fd62e42) 
	{		/* if  |x| > 0.5 ln2 */ 
	    if(hx < 0x3FF0A2B2) 
		{	/* and |x| < 1.5 ln2 */
			hi = x-ln2HI[xsb]; lo=ln2LO[xsb]; k = 1-xsb-xsb;
	    } 
		else 
		{
			k  = (int)(invln2*x+halF[xsb]);
			t  = k;
			hi = x - t*ln2HI[0];	/* t*ln2HI is exact here */
			lo = t*ln2LO[0];
	    }
	    x  = hi - lo;
	} 
	else if(hx < 0x3e300000)  
	{	/* when |x|<2**-28 */
	    if(huge+x>one) 
			return one+x;  /* trigger inexact */
	}
	else 
		k = 0;

    /* x is now in primary range */
	t  = x*x;
	c  = x - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	if(k==0) 	
		return one-((x*c)/(c-2.0)-x); 
	else
 		y = one-((lo-(x*c)/(2.0-c))-hi);

	if(k >= -1021) 
	{
	    unsigned int hy;
	    GET_HIGH_WORD(hy,y);
	    SET_HIGH_WORD(y,hy+(k<<20));	/* add k to y's exponent */
	    return y;
	} 
	else 
	{
	    unsigned int hy;
	    GET_HIGH_WORD(hy,y);
	    SET_HIGH_WORD(y,hy+((k+1000)<<20));	/* add k to y's exponent */
	    return y*twom1000;
	}
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
exp(double x)
{
	double result;

	result = __exp(x);

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
