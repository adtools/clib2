/*
 * $Id: math_atan2.c,v 1.3 2004-08-27 11:40:49 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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
extern double __atan(double x);
#else
#define __atan(x) IEEEDPAtan(x)
#endif /* __GNUC__ */

/****************************************************************************/

INLINE static const double
__atan2(double y,double x)
{
	const double pi = 3.14159265358979323846;
	const double pi_over_2 = pi / 2.0;
	double result;

	if(x > 0.0)
	{
		if(y > 0.0)
		{
			if(x > y)
				result = __atan(y / x);
			else
				result = pi_over_2 - __atan(x / y);
		}
		else
		{
			if(x > -y)
				result = __atan(y / x);
			else
				result = -(pi_over_2 + __atan(x / y));
		}
	}
	else
	{
		if(y > 0.0)
		{
			if((-x) > y)
				result = pi + __atan (y / x);
			else
				result = pi_over_2 - __atan(x / y);
        }
		else
		{
			if((-x) > (-y))
			{
				result = -(pi - __atan(y / x));
			}
			else if (y < 0.0)
			{
				result = -(pi_over_2 + __atan(x / y));
			}
			else
			{
				result = 0;
				errno = EDOM;
			}
		}
	}

	return(result);
}

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE static const double
__atan(double x)
{
	double result;

	__asm ("fatan%.x %1,%0"
	       : "=f" (result)
	       : "f" (x));

	return(result);
}

INLINE static const double
__atan2(double y,double x)
{
	double pi,pi_over_2;
	double result;

	__asm ("fmovecr%.x %#0,%0"	/* extended precision pi */
	       : "=f" (pi)
	       : /* no inputs */ );

	__asm ("fscale%.b %#-1,%0"	/* no loss of accuracy */
	       : "=f" (pi_over_2)
	       : "0" (pi));

	if(x > 0.0)
	{
		if(y > 0.0)
		{
			if(x > y)
				result = __atan(y / x);
			else
				result = pi_over_2 - __atan(x / y);
		}
		else
		{
			if(x > -y)
				result = __atan(y / x);
			else
				result = -(pi_over_2 + __atan(x / y));
		}
	}
	else
	{
		if(y > 0.0)
		{
			if((-x) > y)
				result = pi + __atan (y / x);
			else
				result = pi_over_2 - __atan(x / y);
        }
		else
		{
			if((-x) > (-y))
			{
				result = -(pi - __atan(y / x));
			}
			else if (y < 0.0)
			{
				result = -(pi_over_2 + __atan(x / y));
			}
			else
			{
				result = 0;
				errno = EDOM;
			}
		}
	}

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

static const double 
tiny  = 1.0e-300,
zero  = 0.0,
pi_o_4  = 7.8539816339744827900E-01, /* 0x3FE921FB, 0x54442D18 */
pi_o_2  = 1.5707963267948965580E+00, /* 0x3FF921FB, 0x54442D18 */
pi      = 3.1415926535897931160E+00, /* 0x400921FB, 0x54442D18 */
pi_lo   = 1.2246467991473531772E-16; /* 0x3CA1A626, 0x33145C07 */

INLINE static const double
__atan2(double y,double x)
{
	double z;
	int k,m,hx,hy,ix,iy;
	unsigned int lx,ly;

	EXTRACT_WORDS(hx,lx,x);
	ix = hx&0x7fffffff;

	EXTRACT_WORDS(hy,ly,y);
	iy = hy&0x7fffffff;

	if( ((ix|((lx|-lx)>>31))>0x7ff00000) || ((iy|((ly|-ly)>>31))>0x7ff00000) )	/* x or y is NaN */
	   return x+y;

	if((hx-0x3ff00000|lx)==0) 
		return atan(y);                                                         /* x=1.0 */

	m = ((hy>>31)&1)|((hx>>30)&2);	                                            /* 2*sign(x)+sign(y) */

    /* when y = 0 */
	if((iy|ly)==0) 
	{
	    switch(m) 
		{
		case 0: 
		case 1: 
			return y; 	                                                        /* atan(+-0,+anything)=+-0 */
		case 2: 
			return pi+tiny;                                                     /* atan(+0,-anything) = pi */
		case 3: 
			return -pi-tiny;                                                    /* atan(-0,-anything) =-pi */
	    }
	}

    /* when x = 0 */
	if((ix|lx)==0) 
		return (hy<0)?  -pi_o_2-tiny: pi_o_2+tiny;
	    
    /* when x is INF */
	if(ix==0x7ff00000) 
	{
	    if(iy==0x7ff00000) 
		{
			switch(m) 
			{
			case 0: 
				return  pi_o_4+tiny;                                            /* atan(+INF,+INF) */
			case 1: 
				return -pi_o_4-tiny;                                            /* atan(-INF,+INF) */
			case 2: 
				return  3.0*pi_o_4+tiny;                                        /*atan(+INF,-INF)*/
			case 3: 
				return -3.0*pi_o_4-tiny;                                        /*atan(-INF,-INF)*/
			}
	    }
		else 
		{
			switch(m) 
			{
		    case 0: 
				return  zero  ;	                                                /* atan(+...,+INF) */
		    case 1: 
				return -zero  ;	                                                /* atan(-...,+INF) */
		    case 2: 
				return  pi+tiny  ;	                                            /* atan(+...,-INF) */
		    case 3: 
				return -pi-tiny  ;	                                            /* atan(-...,-INF) */
			}
	    }
	}

    /* when y is INF */
	if(iy==0x7ff00000) 
		return (hy<0)? -pi_o_2-tiny: pi_o_2+tiny;

    /* compute y/x */
	k = (iy-ix)>>20;
	if(k > 60) 
		z=pi_o_2+0.5*pi_lo; 	                                                /* |y/x| >  2**60 */
	else 
		if(hx<0&&k<-60) 
			z=0.0; 	                                                            /* |y|/x < -2**60 */
	else 
		z=atan(fabs(y/x));		                                                /* safe to do y/x */

	switch (m) 
	{
	case 0: 
		return z;       	                                                    /* atan(+,+) */
	case 1: 
	    {
			unsigned int  zh;
			GET_HIGH_WORD(zh,z);
			SET_HIGH_WORD(z,zh ^ 0x80000000);
	    }
		return z;       	                                                    /* atan(-,+) */
	case 2: 
		return  pi-(z-pi_lo);                                                   /* atan(+,-) */
	default: 
		return  (z-pi_lo)-pi;                                                   /* atan(-,-) */
	}
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
atan2(double y,double x)
{
	double result;

	result = __atan2(y,x);

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
