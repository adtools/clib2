/*
 * $Id: ftw_ftw.c,v 1.5 2006-01-08 12:04:22 obarthel Exp $
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

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

/*
 * This ftw() implementation builds heavily on existing functionality and could certainly
 * be made more efficient (e.g. by combining  walk & stat using ExAll()/ExNext()) but this
 * way the Unix pathname translation is free.
 */

/****************************************************************************/

#ifndef	_FTW_HEADERS_H
#include "ftw_headers.h"
#endif /* _FTW_HEADERS_H */

/****************************************************************************/

static int
walk(const char *path,int (*func)(const char *,const struct stat *,int),int depth,int level)
{
	int result = OK;
	DIR *dp;
	struct dirent *de;
	struct stat st;
	char *next_name;
	int type;
	int old_length;
	int errtmp;

	ENTER();

	SHOWSTRING(path);
	SHOWPOINTER(func);
	SHOWVALUE(depth);
	SHOWVALUE(level);

	if(__check_abort_enabled)
		__check_abort();

	if(level > depth)
		goto out;

	if(stat(path,&st) == OK)
	{
		if (S_ISDIR(st.st_mode))
		{
			if(FLAG_IS_SET(st.st_mode,S_IRUSR))
				type = FTW_D;	/* Directory */
			else
				type = FTW_DNR;	/* Directory No Read-permission */
		}
		else if (S_ISREG(st.st_mode))
		{
			type = FTW_F; /* File */
		}
		else
		{
			/* Skip */
			goto out;
		}
	}
	else
	{
		type = FTW_NS; /* No Stat */
	}

	result = (*func)(path,&st,type);
	if(result != 0)
		goto out;

	if(type == FTW_D)
	{
		dp = opendir(path);	/* Also takes care of Unix->Amiga pathname conversion. */
		if(dp == NULL)
		{
			result = ERROR; /* Pass errno from opendir() */
			goto out;
		}

		old_length = strlen(path);

		next_name = malloc(old_length + NAME_MAX + 2); /* Allocate new for each recursive step to handle extremely long path names. */
		if(next_name == NULL)
		{
			__set_errno(ENOMEM);

			result = ERROR;
			goto out;
		}

		strcpy(next_name,path);

		if(old_length > 0 && next_name[old_length - 1] != ':' && next_name[old_length-1] != '/')
			next_name[old_length++] = '/';

		while((result == 0) && (de = readdir(dp)) != NULL)
		{
			strlcpy(&next_name[old_length],de->d_name,NAME_MAX + 2);

			result = walk(next_name,func,depth,level+1);
		}

		errtmp = __get_errno();

		closedir(dp);
		free(next_name);

		__set_errno(errtmp);
	}

 out:

	RETURN(result);
	return(result);
}

int
ftw(const char *path,int (*func)(const char *,const struct stat *,int),int depth)
{
	int result = ERROR;
	char *base;
	int len;

	ENTER();

	SHOWSTRING(path);
	SHOWPOINTER(func);
	SHOWVALUE(depth);
	
	if(path == NULL)
	{
		SHOWMSG("NULL pathname to ftw().");

		__set_errno(EFAULT);
		goto out;
	}

	if(func == NULL)
	{
		SHOWMSG("No function supplied.");

		__set_errno(EFAULT);
		goto out;
	}

	if(depth < 0)
	{
		SHOWMSG("Invalid depth.\n");

		__set_errno(EINVAL);
		goto out;
	}

	/* Make sure the path never ends with '/' unless used to indicate parent directory. */
	len = strlen(path);
	if(len > 1 && path[len - 1] == '/' && path[len - 2] != '/')
	{
		int error;

		base = strdup(path);
		if(base == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}

		base[len - 1] = '\0';

		result = walk(base,func,depth,0);

		error = __get_errno();

		free(base);

		__set_errno(error);
	}
	else
	{
		result = walk(path,func,depth,0);
	}

 out:

	RETURN(result);
	return(result);
}
