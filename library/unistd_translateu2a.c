/*
 * $Id: unistd_translateu2a.c,v 1.2 2005-01-02 09:07:19 obarthel Exp $
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

/*#define DEBUG*/

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

int
__translate_unix_to_amiga_path_name(char const ** name_ptr,struct name_translation_info * nti)
{
	char volume_name[sizeof(nti->substitute)];
	int volume_name_len;
	BOOL have_double_slash;
	BOOL have_slash_dot_slash;
	BOOL have_slash_dot_dot_slash;
	int result = -1;
	char * replace;
	char * name;
	int len,i,j;

	/*PUSHDEBUGLEVEL(2);*/

	assert( name_ptr != NULL && (*name_ptr) != NULL && nti != NULL );

	name = (char *)(*name_ptr);
	replace = nti->substitute;

	nti->is_root = FALSE;

	/* Check if the name would become too long to handle. */
	len = strlen(name);
	if(len >= (int)sizeof(nti->substitute))
	{
		D(("path name '%s' is too long (%ld characters total; maximum is %ld)!",name,len,sizeof(nti->substitute)-1));

		errno = ENAMETOOLONG;
		goto out;
	}

	/* Prepend an absolute path to the name, if such a path was previously set
	   as the current directory. */
	if(__translate_relative_path_name((const char **)&name,nti->substitute,sizeof(nti->substitute)) != 0)
	{
		SHOWMSG("relative path name could not be worked into the pattern");
		goto out;
	}

	D(("initial name = '%s'",name));

	len = strlen(name);

	/* If there is one, strip the trailing slash ('foo/' -> 'foo'). */
	if(len > 1 && name[len-1] == '/')
	{
		if(name != replace)
		{
			memmove(replace,name,(size_t)len-1);
			name = replace;
		}

		len--;

		name[len] = '\0';

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* If there are neighboring slashes, strip all but one
	   ('foo//bar' -> 'foo/bar'). The "//" pattern in a Unix
	   file name is apparently harmless, but on AmigaDOS it
	   has a very definite meaning. */
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
		BOOL have_slash;
		char * from;
		char * to;
		char c;

		have_slash	= FALSE;
		from		= name;
		to			= replace;

		while((c = (*from++)) != '\0')
		{
			if(c == '/')
			{
				if(NOT have_slash)
					(*to++) = c;

				have_slash = TRUE;
			}
			else
			{
				(*to++) = c;

				have_slash = FALSE;
			}
		}

		(*to) = '\0';

		name = replace;

		len = strlen(name);

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Ditch all leading './' ('./foo' -> 'foo'). */
	if(name[0] == '.' && name[1] == '/')
	{
		do
		{
			name	+= 2;
			len		-= 2;
		}
		while(name[0] == '.' && name[1] == '/');

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Ditch all embedded '/./' ('foo/./bar' -> 'foo/bar'). */
	have_slash_dot_slash = FALSE;

	for(i = j = 0 ; i < len - 2 ; i++)
	{
		if(name[i] == '/' && name[i+1] == '.' && name[i+2] == '/')
		{
			have_slash_dot_slash = TRUE;
			break;
		}
	}

	if(have_slash_dot_slash)
	{
		for(i = j = 0 ; i < len ; i++)
		{
			replace[j++] = name[i];

			if(name[i] == '/' && name[i+1] == '.' && name[i+2] == '/')
				i += 2;
		}

		len = j;

		name = replace;

		name[len] = '\0';

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Special case: the path name may end with "/." signifying that the
	   directory itself is requested ('foo/.' -> 'foo'). */
	if(len >= 2 && strncmp(&name[len-2],"/.",2) == SAME)
	{
		/* If it's just '/.' then it's really '/'. */
		if(len == 2)
		{
			name = "/";
			len = 1;
		}
		else
		{
			if(name != replace)
			{
				memmove(replace,name,(size_t)(len - 2));
				name = replace;
			}

			len -= 2;

			name[len] = '\0';
		}

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Check for absolute path. */
	if(name[0] == '/')
	{
		if(name[1] == '\0')
		{
			SHOWMSG("this is the root directory");

			nti->is_root = TRUE;
		}

		/* Ok, so this is an absolute path. We first
		   check for a few special cases, the first
		   being a reference to "/tmp". */
		if((strncmp(name,"/tmp",4) == SAME) && (name[4] == '/' || name[4] == '\0'))
		{
			if(name[4] == '/')
			{
				/* Convert "/tmp/foo" to "T:foo". */
				memmove(&replace[2],&name[5],strlen(&name[5])+1);
				memmove(replace,"T:",2);
			}
			else
			{
				/* Convert "/tmp" to "T:". */
				strcpy(replace,"T:");
			}

			name = replace;

			len = strlen(name);

			D(("name = '%s' (line %ld)",name,__LINE__));
		}
		else if (strcmp(name,"/dev/null") == SAME)
		{
			name = "NIL:";

			len = strlen(name);

			D(("name = '%s' (line %ld)",name,__LINE__));
		}
		else
		{
			int path_name_start = 0;

			volume_name_len = 0;

			/* Find out how long the first component
			   of the absolute path is. */
			len = strlen(name);
			for(i = 1 ; i <= len ; i++)
			{
				if(name[i] == '/' || name[i] == '\0')
				{
					volume_name_len = i-1;

					/* Is there anything following
					 * the path name?
					 */
					if(name[i] == '/')
						path_name_start = i+1;

					break;
				}
			}

			/* Copy the first component and attach a colon. "/foo" becomes
			   "foo:" (without the trailing NUL byte, this will get attached
			   later). */
			memmove(replace,&name[1],(size_t)volume_name_len);
			replace[volume_name_len++] = ':';

			/* Now add the finishing touches. "/foo/bar" finally
			   becomes "foo:bar" and "/foo" becomes "foo:" with the
			   trailing NUL byte attached. */
			if(path_name_start > 0)
				memmove(&replace[volume_name_len],&name[path_name_start],strlen(&name[path_name_start])+1);
			else
				replace[volume_name_len] = '\0';

			name = replace;

			len = strlen(name);

			D(("name = '%s' (line %ld)",name,__LINE__));
		}
	}

	/* Extract and remove the volume name from the path. We
	   are going to need it later. */
	volume_name_len = 0;

	for(i = 0 ; i < len ; i++)
	{
		if(name[i] == ':')
		{
			/* Look for extra colon characters embedded in the name
		       (as in "foo/bar:baz") which really don't belong here. */
			for(j = 0 ; j < i ; j++)
			{
				if(name[j] == '/')
				{
					D(("name '%s' still contains colon characters",name));

					errno = EINVAL; /* invalid name */
					goto out;
				}
			}

			volume_name_len = i+1;

			memmove(volume_name,name,(size_t)volume_name_len);
			volume_name[volume_name_len] = '\0';

			name += volume_name_len;
			len -= volume_name_len;

			D(("name = '%s' (line %ld)",name,__LINE__));

			break;
		}
	}

	/* Look for extra colon characters embedded in the name
       (as in "foo:bar:baz") which really don't belong here. */
	for(i = 0 ; i < len ; i++)
	{
		if(name[i] == ':')
		{
			D(("name '%s' still contains colon characters",name));

			errno = EINVAL; /* invalid name */
			goto out;
		}
	}

	/* Now parse the path name and replace all embedded '..' with
	   the AmigaDOS counterparts ('foo/../bar' -> 'foo//bar'). */
	have_slash_dot_dot_slash = FALSE;

	for(i = j = 0 ; i < len - 3 ; i++)
	{
		if(name[i] == '/' && name[i+1] == '.' && name[i+2] == '.' && name[i+3] == '/')
		{
			have_slash_dot_dot_slash = TRUE;
			break;
		}
	}

	if(have_slash_dot_dot_slash)
	{
		for(i = j = 0 ; i < len ; i++)
		{
			replace[j++] = name[i];

			if(name[i] == '/' && name[i+1] == '.' && name[i+2] == '.' && name[i+3] == '/')
			{
				replace[j++] = '/';
				i += 3;
			}
		}

		len = j;

		name = replace;

		name[len] = '\0';

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Translate a trailing '/..' to '//' */
	if(len >= 3 && strncmp(&name[len-3],"/..",3) == SAME)
	{
		if(name != replace)
		{
			memmove(replace,name,(size_t)(len - 2));
			name = replace;
		}

		len -= 2;

		name[len++] = '/';

		name[len] = '\0';

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Translate a leading '../' ('../foo' -> '/foo') */
	if(len >= 3 && strncmp(name,"../",3) == SAME)
	{
		memmove(replace,&name[2],(size_t)(len - 2));
		name = replace;

		len -= 2;

		name[len] = '\0';

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Translate the '..' ('..' -> '/') */
	if(strcmp(name,"..") == SAME)
	{
		name = "/";
		len = 1;

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Translate the '.' ('.' -> '') */
	if(strcmp(name,".") == SAME)
	{
		name = "";
		len = 0;

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Now put it all together again. */
	if(volume_name_len > 0)
	{
		memmove(&replace[volume_name_len],name,(size_t)len);
		memmove(replace,volume_name,(size_t)volume_name_len);

		len += volume_name_len;

		name = replace;

		name[len] = '\0';

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* Reduce any "//" embedded in the name, if necessary. */
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
		if(name != replace)
		{
			strcpy(replace,name);
			name = replace;
		}

		__strip_double_slash(name,len);

		D(("name = '%s' (line %ld)",name,__LINE__));
	}

	/* The following is somewhat controversial. It assumes that what comes out
	   as a path name that ends with ":/" should translate into the root
	   directory. This would not be the case for the names of assignments,
	   though, for which the "/" suffix would actually do something useful. */
	len = strlen(name); /* ZZZ this should not be necessary! */
	if(len >= 2 && strncmp(&name[len-2],":/",2) == 0)
		nti->is_root = TRUE;

	nti->original_name = (char *)(*name_ptr);
	(*name_ptr) = name;

	D(("translated '%s' to '%s'",nti->original_name,name));

	result = 0;

out:

	/*POPDEBUGLEVEL();*/

	return(result);
}
