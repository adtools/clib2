/*
 * $Id: math_fmod.c,v 1.5 2005-02-25 10:14:21 obarthel Exp $
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

INLINE STATIC const double
__fmod(double x,double y)
{
	double q,p,result;

	q = x / y;

	if(q < 0.0)
		p = ceil(q) - q;
	else
		p = q - floor(q);

	result = p * y;

	return(result);
}

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__fmod(double x,double y)
{
	double result;

	__asm ("fmod%.x %2,%0"
	       : "=f" (result)
	       : "0" (x),
	       "f" (y));

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

static const double 
one = 1.0, 
Zero[] = {0.0, -0.0,};

INLINE STATIC const double
__fmod(double x,double y)
{
	int n,hx,hy,hz,ix,iy,sx,i;
	unsigned int lx,ly,lz;

	EXTRACT_WORDS(hx,lx,x);
	EXTRACT_WORDS(hy,ly,y);

	sx = hx&0x80000000;         		        /* sign of x */
	hx ^=sx;		                            /* |x| */
	hy &= 0x7fffffff;	                        /* |y| */

    /* purge off exception values */
	if((hy|ly)==0||(hx>=0x7ff00000)||	/* y=0,or x not finite */
	  ((hy|((ly|-ly)>>31))>0x7ff00000))	/* or y is NaN */
	    return (x*y)/(x*y);
	if(hx<=hy) 
	{
	    if((hx<hy)||(lx<ly)) 
			return x;	                        /* |x|<|y| return x */
	    if(lx==ly) 
			return Zero[(unsigned int)sx>>31];	/* |x|=|y| return x*0*/
	}

    /* determine ix = ilogb(x) */
	if(hx<0x00100000) 	/* subnormal x */
	{
	    if(hx==0) 
		{
			for (ix = -1043, i=lx; i>0; i<<=1) 
				ix -=1;
	    } 
		else 
		{
			for (ix = -1022,i=(hx<<11); i>0; i<<=1) 
				ix -=1;
	    }
	} 
	else 
		ix = (hx>>20)-1023;

    /* determine iy = ilogb(y) */
	if(hy<0x00100000) 	/* subnormal y */
	{
	    if(hy==0) 
		{
			for (iy = -1043, i=ly; i>0; i<<=1) 
				iy -=1;
	    } 
		else 
		{
			for (iy = -1022,i=(hy<<11); i>0; i<<=1) 
				iy -=1;
	    }
	} 
	else 
		iy = (hy>>20)-1023;

    /* set up {hx,lx}, {hy,ly} and align y to x */
	if(ix >= -1022) 
	    hx = 0x00100000|(0x000fffff&hx);
	else 		/* subnormal x, shift x to normal */
	{
	    n = -1022-ix;
	    if(n<=31) 
		{
	        hx = (hx<<n)|(lx>>(32-n));
	        lx <<= n;
	    }
		else 
		{
			hx = lx<<(n-32);
			lx = 0;
	    }
	}

	if(iy >= -1022) 
	    hy = 0x00100000|(0x000fffff&hy);
	else 		/* subnormal y, shift y to normal */
	{
	    n = -1022-iy;
	    if(n<=31) 
		{
	        hy = (hy<<n)|(ly>>(32-n));
	        ly <<= n;
	    }
		else 
		{
			hy = ly<<(n-32);
			ly = 0;
	    }
	}

    /* fix point fmod */
	n = ix - iy;
	while(n--) 
	{
	    hz=hx-hy;lz=lx-ly; 
		if(lx<ly) 
			hz -= 1;

	    if(hz<0)
		{
			hx = hx+hx+(lx>>31); 
			lx = lx+lx;
		}
	    else 
		{
	    	if((hz|lz)==0) 		/* return sign(x)*0 */
				return Zero[(unsigned int)sx>>31];
	    	hx = hz+hz+(lz>>31); 
			lx = lz+lz;
	    }
	}
	
	hz=hx-hy;
	lz=lx-ly; 
	
	if(lx<ly) 
		hz -= 1;
	if(hz>=0) 
	{
		hx=hz;
		lx=lz;
	}

    /* convert back to floating value and restore the sign */
	if((hx|lx)==0) 			/* return sign(x)*0 */
	    return Zero[(unsigned int)sx>>31];	
	while(hx<0x00100000) 		/* normalize x */
	{
	    hx = hx+hx+(lx>>31); 
		lx = lx+lx;
	    iy -= 1;
	}

	if(iy>= -1022) 	/* normalize output */
	{
	    hx = ((hx-0x00100000)|((iy+1023)<<20));
	    INSERT_WORDS(x,hx|sx,lx);
	}
	else 
	{		/* subnormal output */
	    n = -1022 - iy;
	    if(n<=20) 
		{
			lx = (lx>>n)|((unsigned int)hx<<(32-n));
			hx >>= n;
	    } 
		else 
			if (n<=31) 
			{
				lx = (hx<<(32-n))|(lx>>n); 
				hx = sx;
			} 
			else 
			{
				lx = hx>>(n-32); hx = sx;
			}

	    INSERT_WORDS(x,hx|sx,lx);
	    x *= one;		/* create necessary signal */
	}
	return x;		/* exact output */
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
fmod(double x,double y)
{
	double result;

	if(y != 0.0)
	{
		result = __fmod(x,y);
	}
	else
	{
		result = x;
		__set_errno(EDOM);
	}

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
