/*
 * $Id: locale_localeconv.c,v 1.1.1.1 2004-07-26 16:30:35 obarthel Exp $
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

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

struct lconv *
localeconv(void)
{
	static struct lconv loc;
	struct Locale * numeric_locale = __locale_table[LC_NUMERIC];
	struct Locale * monetary_locale = __locale_table[LC_MONETARY];
	struct lconv * result;

	ENTER();

	/* This makes up the current locale settings from the various
	 * components in use.
	 */
	loc.decimal_point		= (numeric_locale != NULL) ? ((char *)numeric_locale->loc_DecimalPoint) : ".";
	loc.thousands_sep		= (numeric_locale != NULL) ? ((char *)numeric_locale->loc_GroupSeparator) : "";
	loc.grouping			= (numeric_locale != NULL) ? ((char *)numeric_locale->loc_Grouping) : "";
	loc.int_curr_symbol		= (monetary_locale != NULL) ? ((char *)monetary_locale->loc_MonIntCS) : "";
	loc.currency_symbol		= (monetary_locale != NULL) ? ((char *)monetary_locale->loc_MonCS) : "";
	loc.mon_decimal_point	= (monetary_locale != NULL) ? ((char *)monetary_locale->loc_MonDecimalPoint) : "";
	loc.mon_thousands_sep	= (monetary_locale != NULL) ? ((char *)monetary_locale->loc_MonGroupSeparator) : "";
	loc.mon_grouping		= (monetary_locale != NULL) ? ((char *)monetary_locale->loc_MonGrouping) : "";
	loc.positive_sign		= (monetary_locale != NULL) ? ((char *)monetary_locale->loc_MonPositiveSign) : "";
	loc.negative_sign		= (monetary_locale != NULL) ? ((char *)monetary_locale->loc_MonNegativeSign) : "";
	loc.int_frac_digits		= (monetary_locale != NULL) ? monetary_locale->loc_MonIntFracDigits : CHAR_MAX;
	loc.frac_digits			= (monetary_locale != NULL) ? monetary_locale->loc_MonFracDigits : CHAR_MAX;
	loc.p_cs_precedes		= (monetary_locale != NULL) ? monetary_locale->loc_MonPositiveCSPos : CHAR_MAX;
	loc.p_sep_by_space		= (monetary_locale != NULL) ? monetary_locale->loc_MonPositiveSpaceSep : CHAR_MAX;
	loc.n_cs_precedes		= (monetary_locale != NULL) ? monetary_locale->loc_MonNegativeCSPos : CHAR_MAX;
	loc.n_sep_by_space		= (monetary_locale != NULL) ? monetary_locale->loc_MonNegativeSpaceSep : CHAR_MAX;
	loc.p_sign_posn			= (monetary_locale != NULL) ? monetary_locale->loc_MonPositiveSignPos : CHAR_MAX;
	loc.n_sign_posn			= (monetary_locale != NULL) ? monetary_locale->loc_MonNegativeSignPos : CHAR_MAX;

	result = &loc;

	RETURN(result);
	return(result);
}
