/*
 * $Id: math_modf.c,v 1.3 2005-01-02 09:07:07 obarthel Exp $
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

INLINE static const double
__modf(double x,double *nptr)
{
	double int_n;
	double result;

	if(x < 0.0)
	{
		int_n = ceil(x);

		result = int_n - x;
	}
	else
	{
		int_n = floor(x);

		result = x - int_n;
	}

	(*nptr) = int_n;

	return(result);
}

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE static const double
__modf(double x,double *nptr)
{
	double result;
	double temp;

	__asm ("fintrz%.x %1,%0"
	       : "=f" (temp)	/* integer-valued float */
	       : "f" (x));

	(*nptr) = temp;

	result = x - temp;

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(__PPC__)

INLINE static const double
__modf(double x,double *nptr)
{
	double int_n;
	double result;

	if(x < 0.0)
	{
		int_n = ceil(x);

		result = int_n - x;
	}
	else
	{
		int_n = floor(x);

		result = x - int_n;
	}

	(*nptr) = int_n;

	return(result);
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
modf(double x,double *nptr)
{
	double result;

	assert( nptr != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(nptr == NULL)
		{
			errno = EFAULT;
			result = HUGE_VAL;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	result = __modf(x,nptr);

 out:

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
