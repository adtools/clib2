/*
 * $Id: math.h,v 1.11 2005-05-29 10:09:55 obarthel Exp $
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
 */

#ifndef _MATH_H
#define _MATH_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifndef _STDLIB_H
#include <stdlib.h>
#endif /* _STDLIB_H */

/****************************************************************************/

extern double __huge_val;

/****************************************************************************/

#define	HUGE_VAL ((const double)__huge_val)

/****************************************************************************/

extern double acos(double x);
extern double asin(double x);
extern double atan(double x);
extern double atan2(double y,double x);
extern double ceil(double x);
extern double cos(double x);
extern double cosh(double x);
extern double exp(double x);
extern double fabs(double x);
extern double floor(double x);
extern double fmod(double x,double y);
extern double frexp(double x,int *nptr);
extern double ldexp(double x,int n);
extern double log(double x);
extern double log10(double x);
extern double modf(double x,double *nptr);
extern double pow(double x,double y);
extern double sin(double x);
extern double sinh(double x);
extern double sqrt(double x);
extern double tan(double x);
extern double tanh(double x);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#define	M_E			2.7182818284590452354
#define	M_LOG2E		1.4426950408889634074
#define	M_LOG10E	0.43429448190325182765
#define	M_LN2		0.69314718055994530942
#define	M_LN10		2.30258509299404568402
#define	M_PI		3.14159265358979323846
#define	M_PI_2		1.57079632679489661923
#define	M_PI_4		0.78539816339744830962
#define	M_1_PI		0.31830988618379067154
#define	M_2_PI		0.63661977236758134308
#define	M_2_SQRTPI	1.12837916709551257390
#define	M_SQRT2		1.41421356237309504880
#define	M_SQRT1_2	0.70710678118654752440

/****************************************************************************/

extern double rint(double x);
extern float rintf(float x);
extern double logb(double x);
extern double hypot(double x,double y);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

/* Note that the comparison operations performed on the floating point
   data types ought to include float, double and long double. However,
   because the current (2005-06-12) compiler technology available on the
   Amiga does not support the long double type yet, this library is restricted
   to operations on float and double. */

/****************************************************************************/

extern float __huge_val_float;

/****************************************************************************/

#define	HUGE_VALF ((const float)__huge_val_float)

/****************************************************************************/

extern float __infinity;
extern float __nan;

/****************************************************************************/

#define INFINITY ((const float)__infinity)
#define NAN ((const float)__nan)

/****************************************************************************/

#define FP_INFINITE		0	/* -/+ infinity */
#define FP_NAN			1	/* not a number */
#define FP_NORMAL		2	/* normalized floating point number */
#define FP_SUBNORMAL	3	/* very small floating point number; special
							   case of IEEE 754 */
#define FP_ZERO			4	/* exponent/fraction are zero */

/****************************************************************************/

extern int __fpclassify_float(float x);
extern int __fpclassify_double(double x);
extern int __isfinite_float(float x);
extern int __isfinite_double(double x);
extern int __signbit_float(float x);
extern int __signbit_double(double x);

/****************************************************************************/

#define fpclassify(x) \
	(sizeof(x) == sizeof(float) ?	\
		__fpclassify_float(x) :		\
		__fpclassify_double(x))

#define isfinite(x) \
	(sizeof(x) == sizeof(float) ?	\
		__isfinite_float(x) :		\
		__isfinite_double(x))

#define isinf(x) \
	((sizeof(x) == sizeof(float) ?	\
		__fpclassify_float(x) :		\
		__fpclassify_double(x))		\
	== FP_INFINITE)

#define isnan(x) \
	((sizeof(x) == sizeof(float) ?	\
		__fpclassify_float(x) :		\
		__fpclassify_double(x))		\
	== FP_NAN)

#define isnormal(x) \
	((sizeof(x) == sizeof(float) ?	\
		__fpclassify_float(x) :		\
		__fpclassify_double(x))		\
	== FP_NORMAL)

#define signbit(x) \
	(sizeof(x) == sizeof(float) ?	\
		__signbit_float(x) :		\
		__signbit_double(x))

/****************************************************************************/

extern int __isunordered_float(float x,float y);
extern int __isunordered_float_double(float x,double y);
extern int __isunordered_double(double x,double y);

/****************************************************************************/

#define isunordered(x,y) \
	(sizeof(x) == sizeof(float) ?					\
		(sizeof(y) == sizeof(float) ?				\
			__isunordered_float((x),y) :			\
			__isunordered_float_double((x),(y))) :	\
		(sizeof(y) == sizeof(float) ?				\
			__isunordered_float_double((y),(x)) :	\
			__isunordered_double((x),(y))))

#define isgreater(x,y) \
	(isunordered(x,y) ? 0 : (x) > (y))

#define isgreaterequal(x,y) \
	(isunordered(x,y) ? 0 : (x) >= (y))

#define isless(x,y) \
	(isunordered(x,y) ? 0 : (x) < (y))

#define islessequal(x,y) \
	(isunordered(x,y) ? 0 : (x) <= (y))

#define islessgreater(x,y) \
	(isunordered(x,y) ? 0 : (x) < (y) || (x) > (y))	/* ZZZ don't evaluate twice! */

/****************************************************************************/

extern float fabsf(float x);

/****************************************************************************/

extern float nanf(const char *tagp);
extern double nan(const char *tagp);

/****************************************************************************/

extern float nextafterf(float x,float y);
extern double nextafter(double x,double y);

/****************************************************************************/

extern double copysign(double x, double y);
extern float copysignf(float x, float y);

/****************************************************************************/

#define FLT_EVAL_METHOD 0

/****************************************************************************/

typedef float	float_t;
typedef double	double_t;

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _MATH_H */
