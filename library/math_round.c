/*
 * $Id: math_round.c,v 1.3 2006-01-08 12:04:24 obarthel Exp $
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
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

double
round(double x)
{
  /* Most significant word, least significant word. */
  LONG msw, exponent_less_1023;
  ULONG lsw;

  EXTRACT_WORDS(msw, lsw, x);

  /* Extract exponent field. */
  exponent_less_1023 = ((msw & 0x7ff00000) >> 20) - 1023;

  if (exponent_less_1023 < 20)
    {
      if (exponent_less_1023 < 0)
        {
          msw &= 0x80000000;
          if (exponent_less_1023 == -1)
            /* Result is +1.0 or -1.0. */
            msw |= (1023 << 20);
          lsw = 0;
        }
      else
        {
          ULONG exponent_mask = 0x000fffff >> exponent_less_1023;
          if ((msw & exponent_mask) == 0 && lsw == 0)
            /* x in an integral value. */
            return x;

          msw += 0x00080000 >> exponent_less_1023;
          msw &= ~exponent_mask;
          lsw = 0;
        }
    }
  else if (exponent_less_1023 > 51)
    {
      if (exponent_less_1023 == 1024)
        /* x is NaN or infinite. */
        return x + x;
      else
        return x;
    }
  else
    {
      ULONG exponent_mask = 0xffffffff >> (exponent_less_1023 - 20);
      ULONG tmp;

      if ((lsw & exponent_mask) == 0)
        /* x is an integral value. */
        return x;

      tmp = lsw + (1 << (51 - exponent_less_1023));
      if (tmp < lsw)
        msw += 1;
      lsw = tmp;

      lsw &= ~exponent_mask;
    }
  INSERT_WORDS(x, msw, lsw);

  return x;
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
