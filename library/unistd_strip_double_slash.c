/*
 * $Id: unistd_strip_double_slash.c,v 1.3 2005-05-15 12:32:59 obarthel Exp $
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* Remove '//' from an AmigaDOS path name. For example, "volume:one/two//three"
   is equivalent to "volume:one/three" and the following function removes the
   redundant part of the path. */
int
__strip_double_slash(char * file_name,int len)
{
	int position;

	position = len;

	while(len > 1)
	{
		position--;

		/* Stop when we hit the volume name or the first character of the name. */
		if((position == 0) || (file_name[position] == ':') || (file_name[position - 1] == ':'))
			break;

		/* Do we have a // embedded in the file_name? That // must stand alone
		   between directory names and not in front of a volume name or
		   yet another /. */
		if((position > 1) && (file_name[position] == '/' && file_name[position - 1] == '/') && (file_name[position - 2] != ':' && file_name[position - 2] != '/'))
		{
			int start,delta;

			start = position;

			/* Back up behind the //. */
			position -= 2;

			/* Find the spot where the previous directory or volume
			   name begins. */
			while((position > 0) && (file_name[position] != ':') && (file_name[position] != '/'))
				position--;

			/* Don't move too far. */
			if((file_name[position] == ':') || (file_name[position] == '/'))
				position++;

			/* Find out how long the directory name is that
			   we are going to remove now. */
			delta = start - position + 1;

			/* Remove the file_name part; we copy one more byte than
			   necessary to account for the NUL at the end. */
			len -= delta;

			memmove(&file_name[position],&file_name[position + delta],len - position + 1);

			/* The string is shorter, and here we go again... */
			position = len;
		}
	}

	return(len);
}
