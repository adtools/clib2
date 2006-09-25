/*
 * $Id: libgen_dirname.c,v 1.7 2006-09-25 18:19:44 obarthel Exp $
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
 */

#include <string.h>
#include <libgen.h>
#include <stdio.h>

/****************************************************************************/

#include "debug.h"

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

char *
dirname(const char *path)
{
	static char new_path[MAXPATHLEN];
	const char * str;
	char * result;
	size_t len;

	ENTER();

	if(path == NULL)
		SHOWPOINTER(path);
	else
		SHOWSTRING(path);

	str = ".";
	len = 1;

	if(path != NULL && path[0] != '\0')
	{
		len = strlen(path);

		while(len > 1 && path[len-1] == '/')
			len--;

		if(len > 0)
		{
			size_t i;

			for(i = len-1 ;  ; i--)
			{
				if(path[i] == '/')
				{
					str = path;
					len = i;

					if(i == 0)
						len++;

					break;
				}

				if(i == 0)
					break;
			}
		}
	}

	if(len >= sizeof(new_path))
		len = sizeof(new_path)-1;

	memcpy(new_path,str,len);
	new_path[len] = '\0';

	result = new_path;

	RETURN(result);
	return(result);
}
