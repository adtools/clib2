/*
 * $Id: stdlib_setenv.c,v 1.12 2006-09-25 14:12:15 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

struct LocalVariable
{
	struct LocalVariable *	lv_Next;
	char *					lv_Name;
};

/****************************************************************************/

static struct LocalVariable * __lv_root;

/****************************************************************************/

CLIB_DESTRUCTOR(__setenv_exit)
{
	ENTER();

	/* Now for the local variables that may still be set. */
	if(__lv_root != NULL)
	{
		do
		{
			D(("deleting variable '%s'",__lv_root->lv_Name));

			DeleteVar(__lv_root->lv_Name,0);
		}
		while((__lv_root = __lv_root->lv_Next) != NULL);
	}

	LEAVE();
}

/****************************************************************************/

int
setenv(const char *original_name, const char *original_value, int overwrite)
{
	const char * name = original_name;
	const char * value = original_value;
	char * name_copy = NULL;
	struct LocalVariable * lv = NULL;
	struct LocalVar * found;
	int status;
	int result = ERROR;
	size_t i;

	ENTER();

	SHOWSTRING(original_name);
	SHOWSTRING(original_value);
	SHOWVALUE(overwrite);

	assert(original_name != NULL || original_value != NULL);

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(original_name == NULL && original_value == NULL)
		{
			SHOWMSG("invalid parameters");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(name != NULL)
	{
		for(i = 0 ; i < strlen(name) ; i++)
		{
			if(name[i] == '=')
			{
				name_copy = malloc(i+1);
				if(name_copy == NULL)
				{
					SHOWMSG("could not create copy of name");
					goto out;
				}

				memmove(name_copy,name,i);
				name_copy[i] = '\0';

				value = &name[i+1];
				name = name_copy;
				break;
			}
		}
	}
	else if (value != NULL) /* && name == NULL */
	{
		for(i = 0 ; i < strlen(value) ; i++)
		{
			if(value[i] == '=')
			{
				name_copy = malloc(i+1);
				if(name_copy == NULL)
				{
					SHOWMSG("could not create copy of name");
					goto out;
				}

				memmove(name_copy,value,i);
				name_copy[i] = '\0';

				name = name_copy;
				value = &value[i+1];
				break;
			}
		}
	}

	if(name == NULL || name[0] == '\0' || value == NULL)
	{
		SHOWMSG("invalid name");

		__set_errno(EINVAL);
		goto out;
	}

	if(NOT overwrite)
	{
		char buffer[10];

		PROFILE_OFF();
		status = GetVar((STRPTR)name,buffer,sizeof(buffer),0);
		PROFILE_ON();

		if(status != -1)
		{
			SHOWMSG("variable already exists; leaving...");

			result = OK;
			goto out;
		}
	}

	PROFILE_OFF();
	found = FindVar((STRPTR)name,0);
	PROFILE_ON();

	if(found == NULL)
	{
		SHOWMSG("the local variable is not yet set; remembering that");

		lv = malloc(sizeof(*lv) + strlen(name)+1);
		if(lv == NULL)
		{
			SHOWMSG("not enough memory to remember local variable to be deleted");
			goto out;
		}

		lv->lv_Next = __lv_root;
		__lv_root = lv;

		lv->lv_Name = (char *)(lv+1);
		strcpy(lv->lv_Name,name);
	}

	if(value == NULL)
		value = "";

	SHOWSTRING(name);
	SHOWSTRING(value);

	PROFILE_OFF();
	status = SetVar((STRPTR)name,(STRPTR)value,(LONG)strlen(value),0);
	PROFILE_ON();

	if(status == DOSFALSE)
	{
		SHOWMSG("could not set variable");

		if(lv != NULL)
		{
			__lv_root = lv->lv_Next;
			free(lv);
		}

		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	result = OK;

 out:

	if(name_copy != NULL)
		free(name_copy);

	RETURN(result);
	return(result);
}
