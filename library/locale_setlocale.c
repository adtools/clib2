/*
 * $Id: locale_setlocale.c,v 1.5 2006-01-08 12:04:23 obarthel Exp $
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

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

/****************************************************************************/

char *
setlocale(int category, const char *locale)
{
	DECLARE_LOCALEBASE();
	char * result = NULL;

	ENTER();

	SHOWVALUE(category);

	if(locale == NULL)
		SHOWPOINTER(locale);
	else
		SHOWSTRING(locale);

	__locale_lock();

	if(category < LC_ALL || category > LC_TIME)
	{
		SHOWMSG("invalid category");

		__set_errno(EINVAL);
		goto out;
	}

	if(locale != NULL)
	{
		struct Locale * loc = NULL;

		/* We have to keep the locale name for later reference.
		 * On the Amiga this will be a path and file name so it
		 * can become rather long. But we can't store an arbitrarily
		 * long name either.
		 *
		 * ZZZ change this to dynamic allocation of the locale name.
		 */
		if(strlen(locale) >= MAX_LOCALE_NAME_LEN)
		{
			SHOWMSG("locale name is too long");

			__set_errno(ENAMETOOLONG);
			goto out;
		}

		/* Unless we are switching to the "C" locale,
		 * try to open a locale if we managed to open
		 * locale.library before.
		 */
		if(LocaleBase != NULL)
		{
			if(strcmp(locale,"C") != SAME)
			{
				SHOWMSG("this is not the 'C' locale");

				PROFILE_OFF();

				/* The empty string stands for the default locale. */
				if(locale[0] == '\0')
					loc = OpenLocale(NULL);
				else
					loc = OpenLocale((STRPTR)locale);

				PROFILE_ON();

				if(loc == NULL)
				{
					SHOWMSG("couldn't open the locale");

					__set_errno(ENOENT);
					goto out;
				}
			}
		}

		if(category == LC_ALL)
		{
			int i;

			SHOWMSG("closing all locales");

			/* We have to replace all locales. We
			 * start by closing them all.
			 */
			__close_all_locales();

			SHOWMSG("reinitializing all locales");

			/* And this puts the new locale into all table entries. */
			for(i = 0 ; i < NUM_LOCALES ; i++)
			{
				__locale_table[i] = loc;
				strcpy(__locale_name_table[i],locale);
			}
		}
		else
		{
			SHOWMSG("closing the locale");

			/* Close this single locale unless it's actually just a
			 * copy of the 'all' locale entry.
			 */
			if(__locale_table[category] != NULL && __locale_table[category] != __locale_table[LC_ALL])
			{
				assert( LocaleBase != NULL );
				CloseLocale(__locale_table[category]);
			}

			SHOWMSG("reinitializing the locale");

			__locale_table[category] = loc;
			strcpy(__locale_name_table[category],locale);
		}
	}

	result = __locale_name_table[category];

	SHOWSTRING(result);

 out:

	__locale_unlock();

	RETURN(result);
	return(result);
}
