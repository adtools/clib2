/*
 * $Id: math_hypot.c,v 1.1 2004-08-16 09:33:11 obarthel Exp $
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

#if defined(IEEE_FLOATING_POINT_SUPPORT) || defined(M68881_FLOATING_POINT_SUPPORT)

INLINE static const double
__hypot(double x,double y)
{
	double yx;

	x = fabs(x);
	y = fabs(y);
	if ( x < y )
	{
		double temp = x;
		x = y;
		y = temp;
	}

	if (x == 0.)
	{
		return 0.;
	}
	else
	{
		yx = y/x;
		return x*sqrt(1. + yx*yx);
	}
}

#endif /* IEEE_FLOATING_POINT_SUPPORT || M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

static const double one = 1.0;

INLINE static const double
__hypot(double x,double y)
{
	int n0;
	double a=x,b=y,t1,t2,y1,y2,w;
	int j,k,ha,hb;

	n0 = ((*(int*)&one)>>29)^1;	/* high word index */
	ha = *(n0+(int*)&x)&0x7fffffff;	/* high word of  x */
	hb = *(n0+(int*)&y)&0x7fffffff;	/* high word of  y */
	if(hb > ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
	*(n0+(int*)&a) = ha;	/* a <- |a| */
	*(n0+(int*)&b) = hb;	/* b <- |b| */
	if((ha-hb)>0x3c00000) {return a+b;} /* x/y > 2**60 */
	k=0;
	if(ha > 0x5f300000) {	/* a>2**500 */
	   if(ha >= 0x7ff00000) {	/* Inf or NaN */
	       w = a+b;			/* for sNaN */
	       if(((ha&0xfffff)|*(1-n0+(int*)&a))==0) w = a;
	       if(((hb^0x7ff00000)|*(1-n0+(int*)&b))==0) w = b;
	       return w;
	   }
	   /* scale a and b by 2**-600 */
	   ha -= 0x25800000; hb -= 0x25800000;	k += 600;
	   *(n0+(int*)&a) = ha;
	   *(n0+(int*)&b) = hb;
	}
	if(hb < 0x20b00000) {	/* b < 2**-500 */
	    if(hb <= 0x000fffff) {	/* subnormal b or 0 */	
		if((hb|(*(1-n0+(int*)&b)))==0) return a;
		t1=0;
		*(n0+(int*)&t1) = 0x7fd00000;	/* t1=2^1022 */
		b *= t1;
		a *= t1;
		k -= 1022;
	    } else {		/* scale a and b by 2^600 */
	        ha += 0x25800000; 	/* a *= 2^600 */
		hb += 0x25800000;	/* b *= 2^600 */
		k -= 600;
	   	*(n0+(int*)&a) = ha;
	   	*(n0+(int*)&b) = hb;
	    }
	}
    /* medium size a and b */
	w = a-b;
	if (w>b) {
	    t1 = 0;
	    *(n0+(int*)&t1) = ha;
	    t2 = a-t1;
	    w  = sqrt(t1*t1-(b*(-b)-t2*(a+t1)));
	} else {
	    a  = a+a;
	    y1 = 0;
	    *(n0+(int*)&y1) = hb;
	    y2 = b - y1;
	    t1 = 0;
	    *(n0+(int*)&t1) = ha+0x00100000;
	    t2 = a - t1;
	    w  = sqrt(t1*y1-(w*(-w)-(t1*y2+t2*b)));
	}
	if(k!=0) {
	    t1 = 1.0;
	    *(n0+(int*)&t1) += (k<<20);
	    return t1*w;
	} else return w;
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
hypot(double x,double y)
{
	double result;

	result = __hypot(x,y);

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
