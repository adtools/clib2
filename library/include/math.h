/*
 * $Id: math.h,v 1.4 2004-08-16 09:33:13 obarthel Exp $
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

extern double rint(double x);
extern float rintf(float x);
extern int isinf(double x);
extern int isnan(double x);
extern double logb(double x);
extern double hypot(double x,double y);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _MATH_H */
