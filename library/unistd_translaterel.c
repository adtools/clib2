/*
 * $Id: unistd_translaterel.c,v 1.2 2005-01-02 09:07:19 obarthel Exp $
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

int
__translate_relative_path_name(char const ** name_ptr,char *replacement_buffer,size_t replacement_buffer_size)
{
	int result = -1;
	char * name;

	assert( name_ptr != NULL && (*name_ptr) != NULL && replacement_buffer != NULL );

	/* If we have a current directory which all references should
	 * be made relative to, do just that. Absolute paths
	 * are not modified, though.
	 */
	name = (char *)(*name_ptr);
	if(__current_path_name[0] != '\0' && name[0] != '/')
	{
		size_t total_len;
		BOOL add_slash;

		/* Get the current directory name and get
		 * ready to attach the file name at the end.
		 */
		total_len = strlen(__current_path_name);

		if(__current_path_name[total_len-1] != '/' &&
		   __current_path_name[total_len-1] != ':' &&
		   name[0] != '\0')
		{
			add_slash = TRUE;

			total_len++;
		}
		else
		{
			add_slash = FALSE;
		}

		total_len += strlen(name);

		/* Check if the complete string will fit, including the terminating NUL byte. */
		if(total_len+1 > replacement_buffer_size)
		{
			errno = ENAMETOOLONG;
			goto out;
		}

		/* Put the file name together. */
		strcpy(replacement_buffer,__current_path_name);

		if(add_slash)
			strcat(replacement_buffer,"/");

		strcat(replacement_buffer,name);

		assert( strlen(replacement_buffer) < replacement_buffer_size );

		(*name_ptr) = replacement_buffer;
	}

	result = 0;

 out:

	return(result);
}
