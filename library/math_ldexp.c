/*
 * $Id: math_ldexp.c,v 1.3 2005-02-25 10:14:21 obarthel Exp $
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

#define MANT_MASK	0x800FFFFF	/* Mantissa extraction mask */
#define ZPOS_MASK	0x3FF00000	/* Positive # mask for exp = 0 */
#define ZNEG_MASK	0x3FF00000	/* Negative # mask for exp = 0 */

#define EXP_MASK	0x7FF00000	/* Mask for exponent */
#define EXP_SHIFTS	20			/* Shifts to get into LSB's */
#define EXP_BIAS	1023		/* Exponent bias */

union dtol
{
	double	dval;
	long	ival[2];
};

INLINE STATIC const double
__ldexp(double x,int n)
{
	union dtol number;
	long *iptr, cn;

	number.dval = x;

	iptr = &number.ival[0];

	cn = (((*iptr) & EXP_MASK) >> EXP_SHIFTS) - EXP_BIAS;

	(*iptr) &= ~EXP_MASK;

	n += EXP_BIAS;

	/* ZZZ we can't just muck with the exponent, we
	 * have to check for underflow and overflow, too!
	 */
	(*iptr) |= ((n + cn) << EXP_SHIFTS) & EXP_MASK;

	return(number.dval);
}

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__ldexp(double x,int n)
{
	double result;

	__asm ("fscale%.l %2,%0"
	       : "=f" (result)
	       : "0" (x),
	       "dmi" (n));

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

#define MANT_MASK	0x800FFFFF	/* Mantissa extraction mask */
#define ZPOS_MASK	0x3FF00000	/* Positive # mask for exp = 0 */
#define ZNEG_MASK	0x3FF00000	/* Negative # mask for exp = 0 */

#define EXP_MASK	0x7FF00000	/* Mask for exponent */
#define EXP_SHIFTS	20			/* Shifts to get into LSB's */
#define EXP_BIAS	1023		/* Exponent bias */

union dtol
{
	double	dval;
	long	ival[2];
};

INLINE STATIC const double
__ldexp(double x,int n)
{
	union dtol number;
	long *iptr, cn;

	number.dval = x;

	iptr = &number.ival[0];

	cn = (((*iptr) & EXP_MASK) >> EXP_SHIFTS) - EXP_BIAS;

	(*iptr) &= ~EXP_MASK;

	n += EXP_BIAS;

	/* ZZZ we can't just muck with the exponent, we
	 * have to check for underflow and overflow, too!
	 */
	(*iptr) |= ((n + cn) << EXP_SHIFTS) & EXP_MASK;

	return(number.dval);
}

#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
ldexp(double x,int n)
{
	double result;

	if(x != 0.0)
		result = __ldexp(x,n);
	else
		result = 0.0;

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
