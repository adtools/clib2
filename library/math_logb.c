/*
 * $Id: math_logb.c,v 1.8 2005-10-27 08:58:41 obarthel Exp $
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

#if defined(IEEE_FLOATING_POINT_SUPPORT) || defined(M68881_FLOATING_POINT_SUPPORT)

/****************************************************************************/

INLINE STATIC const double
__logb(double x)
{
	double result;

	result = log(x) / log((double)FLT_RADIX);

	return(result);
}

#endif /* IEEE_FLOATING_POINT_SUPPORT || M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

INLINE STATIC double
__logb(double x)
{
	unsigned int lx,ix;

	EXTRACT_WORDS(ix,lx,x);

	ix &= 0x7fffffff;	/* high |x| */
	if((ix|lx)==0)
		return -1.0/fabs(x);

	if(ix>=0x7ff00000)
		return x*x;

	if((ix>>=20)==0)	/* IEEE 754 logb */
		return -1022.0;
	else
		return (double) (ix-1023);
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
logb(double x)
{
	double result;

	if(x == 0.0)
	{
		result = -__inf();
		goto out;
	}

	if(isnan(x))
	{
		result = x;
		goto out;
	}

	if(isinf(x))
	{
		if(x < 0)
			result = (-x);
		else
			result = x;

		goto out;
	}

	result = __logb(x);

 out:

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
