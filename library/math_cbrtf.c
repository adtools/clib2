/*
 * $Id: math_cbrtf.c,v 1.3 2006-01-08 12:04:23 obarthel Exp $
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
 *
 *
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

static const ULONG
	B1 = 709958130, /* B1 = (84+2/3-0.03306235651)*2**23 */
	B2 = 642849266; /* B2 = (76+2/3-0.03306235651)*2**23 */

static const float
C =  5.4285717010e-01, /* 19/35     = 0x3f0af8b0 */
D = -7.0530611277e-01, /* -864/1225 = 0xbf348ef1 */
E =  1.4142856598e+00, /* 99/70     = 0x3fb50750 */
F =  1.6071428061e+00, /* 45/28     = 0x3fcdb6db */
G =  3.5714286566e-01; /* 5/14      = 0x3eb6db6e */

float
cbrtf(float x)
{
	LONG	hx;
	float r,s,t;
	ULONG sign;
	ULONG high;

	GET_FLOAT_WORD(hx,x);
	sign=hx&0x80000000U; 		/* sign= sign(x) */
	hx  ^=sign;
	if(hx>=0x7f800000) return(x+x); /* cbrt(NaN,INF) is itself */
	if(hx==0) 
	    return(x);		/* cbrt(0) is itself */

	SET_FLOAT_WORD(x,hx);	/* x <- |x| */
    /* rough cbrt to 5 bits */
	if(hx<0x00800000) 		/* subnormal number */
	  {SET_FLOAT_WORD(t,0x4b800000); /* set t= 2**24 */
	   t*=x; GET_FLOAT_WORD(high,t); SET_FLOAT_WORD(t,high/3+B2);
	  }
	else
	  SET_FLOAT_WORD(t,hx/3+B1);

    /* new cbrt to 23 bits */
	r=t*t/x;
	s=C+r*t;
	t*=G+F/(s+E+D/s);	

    /* retore the sign bit */
	GET_FLOAT_WORD(high,t);
	SET_FLOAT_WORD(t,high|sign);
	return(t);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
