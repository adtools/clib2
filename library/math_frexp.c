/*
 * $Id: math_frexp.c,v 1.5 2005-10-27 08:58:41 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__frexp(double x,int * nptr)
{
	int int_exponent = 0;
	BOOL is_negative;

	if(x < 0)
	{
		is_negative = TRUE;

		x = (-x);
	}
	else
	{
		is_negative = FALSE;
	}

	if(x >= 1)
	{
		while(x >= 1)
		{
			int_exponent++;
			x /= 2;
		}
	}
	else if (0 < x && x < 0.5)
	{
		while(x < 0.5)
		{
			int_exponent--;
			x *= 2;
		}
	}

	(*nptr) = int_exponent;

	if(is_negative)
		x = (-x);

	return(x);
}

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__frexp(double x,int * nptr)
{
	double float_exponent;
	int int_exponent;
	double mantissa;

	__asm ("fgetexp%.x %1,%0"
	       : "=f" (float_exponent) /* integer-valued float */
	       : "f" (x));

	int_exponent = (int)float_exponent;

	__asm ("fgetman%.x %1,%0"
	       : "=f" (mantissa) /* 1.0 <= mantissa < 2.0 */
	       : "f" (x));

	if (mantissa != 0)
	{
		__asm ("fscale%.b %#-1,%0"
		       : "=f" (mantissa) /* mantissa /= 2.0 */
		       : "0" (mantissa));

		int_exponent += 1;
	}

	(*nptr) = int_exponent;

	return(mantissa);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

static const double
two54 =  1.80143985094819840000e+16; /* 0x43500000, 0x00000000 */

INLINE STATIC double
__frexp(double x,int * eptr)
{
	int hx, ix, lx;

	EXTRACT_WORDS(hx,lx,x);

	ix = 0x7fffffff&hx;
	*eptr = 0;

	if(ix>=0x7ff00000||((ix|lx)==0))
		return x;	                          /* 0,inf,nan */
	if (ix<0x00100000)  		              /* subnormal */
	{
	    x *= two54;
	    GET_HIGH_WORD(hx,x);
	    ix = hx&0x7fffffff;
	    *eptr = -54;
	}

	*eptr += (ix>>20)-1022;

	hx = (hx&0x800fffff)|0x3fe00000;
	SET_HIGH_WORD(x,hx);

	return x;
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
frexp(double x,int *nptr)
{
	double result;

	assert( nptr != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(nptr == NULL)
		{
			__set_errno(EFAULT);

			result = __get_huge_val();
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(x != 0.0)
	{
		result = __frexp(x,nptr);
	}
	else
	{
		result = 0.0;

		(*nptr) = 0;
	}

 out:

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
