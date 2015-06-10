/*
 * $Id: amiga_invertstring.c,v 1.4 2006-01-08 12:04:22 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

#include <libraries/commodities.h>
#include <devices/keymap.h>
#include <exec/memory.h>

#include <string.h>

/****************************************************************************/

#include <proto/exec.h>
#include <proto/commodities.h>
#include <clib/alib_protos.h>

/****************************************************************************/

#include "debug.h"

/****************************************************************************/

STATIC int
do_escape(int cc)
{
	int result;

	switch(cc)
	{
		case 'n':
        case 'r':

			result = '\r';
			break;

        case 't':

			result = '\t';
			break;

        case '0':

			result = '\0';
			break;

        case '\\':
        case '\"':
        case '\'':
        case '<':

			result = cc;
			break;

        default:

			result = -1;
			break;
    }

    return(result);
}

/****************************************************************************/

STATIC ULONG
do_angle(STRPTR * strp, struct InputEvent *ie)
{
	ULONG result;
	char * cp;
	IX ix;

	/* find closing angle '>', put a null there   */
	for(cp = (*strp) ; (*cp) ; cp++)
	{
		if((*cp) == '>')
		{
			(*cp) = '\0';
			break;
		}
	}

	result = ParseIX((*strp),&ix);

	if(cp != NULL)
	{
		(*cp)	= '>';	/* fix it */
		(*strp)	= cp;	/* point to char following '>' */
	}
	else
	{
		(*strp) = cp - 1;   /* ++will point to terminating null */
	}

	if(result == 0)
	{
		/* use IX to describe a suitable InputEvent */
		ie->ie_Class		= ix.ix_Class;
		ie->ie_Code			= ix.ix_Code;
		ie->ie_Qualifier	= ix.ix_Qualifier;
	}

	return(result);
}

/****************************************************************************/

extern VOID FreeIEvents(struct InputEvent *ie);

/****************************************************************************/

struct InputEvent *
InvertString(CONST_STRPTR str, CONST struct KeyMap *km)
{
	/* bugs:
	   can't escape '>'
	   puts '\0' on closing angles */

	/* allocate input event for each character in string   */

	struct InputEvent * result = NULL;
	struct InputEvent * chain = NULL;
	struct InputEvent * ie;
	int cc;

	if(CxBase == NULL || str == NULL || (*str) == '\0')
		goto out;

	do /* have checked that str is not null    */
	{
		/* allocate the next ie and link it in */
		ie = AllocVec(sizeof(*ie),MEMF_ANY|MEMF_CLEAR|MEMF_PUBLIC);
		if(ie == NULL)
			goto out;

		ie->ie_NextEvent = chain;
		chain = ie;

		/* now fill in the input event   */
		cc = (*str);
		switch(cc)
		{
			case '<':

		        str++;

				if(do_angle((STRPTR*)&str, ie) != 0)
					goto out;

				break;

			case '\\':

		        str++;

				cc = do_escape(*str); /* get escaped character */
				if(cc < 0)
					goto out;

				/* fall through  */

			default:

				InvertKeyMap((ULONG) cc, ie, (struct KeyMap *)km);
				break;
		}

		str++;
	}
	while((*str) != '\0');

	result = chain;

 out:

	if(result == NULL && chain != NULL)
		FreeIEvents(chain);

	return(result);
}
