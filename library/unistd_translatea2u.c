/*
 * $Id: unistd_translatea2u.c,v 1.4 2005-02-28 13:32:11 obarthel Exp $
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
__translate_amiga_to_unix_path_name(char const ** name_ptr,struct name_translation_info * nti)
{
	const int max_unix_len = sizeof(nti->substitute)-1;
	char local_replacement[sizeof(nti->substitute)];
	BOOL have_double_slash;
	int len,replace_len;
	int result = -1;
	int i;
	char * replace;
	char * name;

	assert( name_ptr != NULL && (*name_ptr) != NULL && nti != NULL );

	replace = nti->substitute;

	name = (char *)(*name_ptr);

	/* Check if the name would become too long to handle. */
	len = strlen(name);
	if(len >= (int)sizeof(nti->substitute))
	{
		D(("path name '%s' is too long (%ld characters total; maximum is %ld)!",name,len,sizeof(nti->substitute)-1));

		__set_errno(ENAMETOOLONG);
		goto out;
	}

	/* Reduce any '//' embedded in the name if possible. */
	have_double_slash = FALSE;

	for(i = 0 ; i < len-1 ; i++)
	{
		if(name[i] == '/' && name[i+1] == '/')
		{
			have_double_slash = TRUE;
			break;
		}
	}

	if(have_double_slash)
	{
		/* We will need to make a copy of the name. Make sure that it fits. */
		if(len > max_unix_len)
		{
			__set_errno(ENAMETOOLONG);
			goto out;
		}

		strcpy(local_replacement,name);
		name = local_replacement;

		__strip_double_slash(name,len);

		len = strlen(name);
	}

	/* The empty string corresponds to the current
	 * directory, which is the ".".
	 */
	if(len == 0)
	{
		strcpy(replace,".");
	}
	else
	{
		char * volume_name;
		int volume_name_len;

		/* Figure out if the path includes a volume, device or assignment name.
		   If so, strip it from the path, to be translated and added later. */
		volume_name = NULL;
		volume_name_len = 0;

		for(i = 0 ; i < len ; i++)
		{
			if(name[i] == ':')
			{
				volume_name = name;
				volume_name_len = i;

				name += volume_name_len+1;
				len -= volume_name_len+1;

				break;
			}
		}

		/* Replace all "/" characters in the name which don't just act as
		 * separators with "..". Otherwise, copy the name string to the
		 * replacement buffer.
		 */
		replace_len = 0;

		while(name[0] == '/')
		{
			if(replace_len + 3 > max_unix_len)
			{
				__set_errno(ENAMETOOLONG);
				goto out;
			}

			memmove(&replace[replace_len],"../",3);
			replace_len += 3;

			name++;
			len--;

			if(len == 0)
			{
				/* Strip the trailing '/'. */
				replace[--replace_len] = '\0';
			}
		}

		for(i = 0 ; i < len ; i++)
		{
			if(name[i] == '/' && (name[i+1] == '/' || name[i+1] == '\0'))
			{
				if(replace_len + 3 > max_unix_len)
				{
					__set_errno(ENAMETOOLONG);
					goto out;
				}

				memmove(&replace[replace_len],"/..",3);
				replace_len += 3;
			}
			else
			{
				if(replace_len + 1 > max_unix_len)
				{
					__set_errno(ENAMETOOLONG);
					goto out;
				}

				replace[replace_len++] = name[i];
			}
		}

		assert( replace_len <= max_unix_len );

		replace[replace_len] = '\0';

		/* If necessary, add and translate the volume name found above. */
		if(volume_name != NULL)
		{
			/* Check if the complete path will fit. This assumes the worst
			   case scenario. */
			if(1 + volume_name_len + 1 + replace_len > max_unix_len)
			{
				__set_errno(ENAMETOOLONG);
				goto out;
			}

			if(volume_name_len > 0)
			{
				if(replace_len > 0)
				{
					/* Add the volume name in front of the path. */
					memmove(&replace[1 + volume_name_len + 1],replace,(size_t)(replace_len+1));
					replace[0] = replace[1 + volume_name_len] = '/';
					memmove(&replace[1],volume_name,(size_t)volume_name_len);
				}
				else
				{
					/* There is only the volume name. */
					replace[0] = '/';
					memmove(&replace[1],volume_name,(size_t)volume_name_len);
					replace[volume_name_len + 1] = '\0';
				}
			}
			else
			{
				int path_prefix_len = 0;

				/* We need to expand ':'. If the current directory name is
				   known and refers to an absolute path, we borrow the
				   absolute path name component. */
				if(__current_path_name[0] != '\0')
				{
					int path_name_len;

					path_name_len = strlen(__current_path_name);

					/* Figure out where the absolute path name portion ends. */
					for(i = 1 ; i <= path_name_len ; i++)
					{
						if(__current_path_name[i] == '/' ||
						   __current_path_name[i] == '\0')
						{
							path_prefix_len = i;
							break;
						}
					}

					/* If we found the absolute path name portion, use it. */
					if(path_prefix_len > 0)
					{
						if(path_prefix_len + 1 + replace_len > max_unix_len)
						{
							__set_errno(ENAMETOOLONG);
							goto out;
						}

						if(replace_len > 0)
						{
							memmove(&replace[path_prefix_len + 1],replace,(size_t)(replace_len+1));
							memmove(replace,__current_path_name,(size_t)path_prefix_len);
							replace[path_prefix_len] = '/';
						}
						else
						{
							memmove(replace,__current_path_name,(size_t)path_prefix_len);
							replace[path_prefix_len] = '\0';
						}
					}
				}

				if(path_prefix_len == 0)
				{
					/* The ':foo' path gets translated into '/foo'. */
					memmove(&replace[1],replace,(size_t)(replace_len+1));
					replace[0] = '/';
				}
			}
		}
	}

	name = replace;

	assert( strlen(name) < (size_t)max_unix_len );

	nti->original_name = (char *)(*name_ptr);
	(*name_ptr) = name;

	D(("translated '%s' to '%s'",nti->original_name,name));

	result = 0;

 out:

	return(result);
}
