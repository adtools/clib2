/*
 * $Id: unistd_wildcard_expand.c,v 1.1.1.1 2004-07-26 16:32:33 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#include "stdlib_protos.h"

/****************************************************************************/

#include <clib/alib_protos.h>

/****************************************************************************/

UBYTE *
__allocate_quote_vector(int num_parameters)
{
	UBYTE * vector;
	size_t num_bytes;

	assert( num_parameters > 0 );

	num_bytes = (num_parameters + 7) / 8;

	vector = malloc(num_bytes);
	if(vector != NULL)
		memset(vector,0,num_bytes);

	return(vector);
}

/****************************************************************************/

static struct AnchorPath * anchor;

/****************************************************************************/

void
__wildcard_expand_exit(void)
{
	if(anchor != NULL)
	{
		MatchEnd(anchor);

		free(anchor);
		anchor = NULL;
	}
}

/****************************************************************************/

struct name_node
{
	struct MinNode	nn_min_node;
	char *			nn_name;
	BOOL			nn_wild;
};

/****************************************************************************/

static int
compare(const char **a,const char **b)
{
	DECLARE_UTILITYBASE();

	assert( UtilityBase != NULL );

	return(Stricmp((STRPTR)*a,(STRPTR)*b));
}

/****************************************************************************/

int
__wildcard_expand_init(void)
{
	struct AnchorPath * ap = NULL;
	struct MinList argument_list;
	size_t argument_list_size;
	BOOL found_pattern = FALSE;
	BOOL match_made;
	struct name_node * node;
	int argc;
	char ** argv;
	int error;
	int i;

	PROFILE_OFF();

	/* No work to be done? */
	if(__quote_vector == NULL || __argc == 0 || __argv == NULL)
	{
		error = OK;
		goto out;
	}

	argc = __argc;
	argv = __argv;

	/* We need some extra room in this data structure as the buffer
	 * will be used to check if a string contains a pattern.
	 */
	ap = malloc(sizeof(*ap) + 2 * MAXPATHLEN);
	if(ap == NULL)
	{
		error = ENOMEM;
		goto out;
	}

	/* This has to be long-word aligned. */
	assert( (((ULONG)ap) & 3) == 0 );

	anchor = ap;

	memset(ap,0,sizeof(*ap));

	/* The argument list will go in here. */
	NewList((struct List *)&argument_list);
	argument_list_size = 0;

	for(i = 0 ; i < argc ; i++)
	{
		match_made = FALSE;

		/* Does this string contain a wildcard pattern? We also check if the
		 * string is quoted. Quoted strings are never expanded.
		 */
		if(i > 0 && (__quote_vector[i / 8] & (1 << (7 - (i & 7)))) == 0 && ParsePatternNoCase(argv[i],ap->ap_Buf,2 * MAXPATHLEN) > 0)
		{
			BOOL is_first = TRUE;
			LONG rc;

			memset(ap,0,sizeof(*ap));

			ap->ap_Strlen = MAXPATHLEN;

			if(__check_abort_enabled)
				ap->ap_BreakBits = SIGBREAKF_CTRL_C;

			rc = MatchFirst(argv[i],ap);

			while(TRUE)
			{
				/* Got a break signal? */
				if(rc == ERROR_BREAK)
				{
					SetSignal(SIGBREAKF_CTRL_C,SIGBREAKF_CTRL_C);
					__check_abort();

					/* If we ever arrive here, retry the previous match. */
					if(is_first)
					{
						MatchEnd(ap);

						memset(ap,0,sizeof(*ap));

						ap->ap_Strlen = MAXPATHLEN;

						if(__check_abort_enabled)
							ap->ap_BreakBits = SIGBREAKF_CTRL_C;

						rc = MatchFirst(argv[i],ap);
					}
					else
					{
						rc = MatchNext(ap);
					}

					continue;
				}
				else if (rc == ERROR_NO_MORE_ENTRIES)
				{
					/* End of the line. */
					break;
				}
				else if (rc != OK)
				{
					/* Some error occured. */
					error = EIO;
					break;
				}

				/* Allocate another node for the expanded parameter. */
				node = malloc(sizeof(*node) + strlen(ap->ap_Buf) + 1);
				if(node == NULL)
				{
					error = ENOMEM;
					goto out;
				}

				/* Remember that this name ended up here due to pattern matching. */
				node->nn_name = (char *)(node + 1);
				node->nn_wild = TRUE;

				strcpy(node->nn_name,ap->ap_Buf);

				AddTail((struct List *)&argument_list,(struct Node *)node);
				argument_list_size++;

				rc = MatchNext(ap);

				is_first = FALSE;

				/* Remember that we found a wildcard pattern among the arguments. */
				match_made = found_pattern = TRUE;
			}

			MatchEnd(ap);
		}

		if(NOT match_made)
		{
			/* Just remember this argument as is. */
			node = malloc(sizeof(*node));
			if(node == NULL)
			{
				error = ENOMEM;
				goto out;
			}

			node->nn_name = argv[i];
			node->nn_wild = FALSE;

			AddTail((struct List *)&argument_list,(struct Node *)node);
			argument_list_size++;
		}
	}

	/* If there were wildcard patterns among the command line parameters,
	 * work them into the argument vector.
	 */
	if(found_pattern)
	{
		size_t last_wild,position;
		char ** table;

		/* This will hold the new arguments. */
		table = malloc(sizeof(*table) * (argument_list_size + 1));
		if(table == NULL)
		{
			error = ENOMEM;
			goto out;
		}

		__argc = argument_list_size;
		__argv = table;

		/* Fill in the table, sorting the wildcard matches. */
		last_wild = 0;

		for(node = (struct name_node *)argument_list.mlh_Head, position = 0 ;
		    node->nn_min_node.mln_Succ != NULL ;
		    node = (struct name_node *)node->nn_min_node.mln_Succ, position++)
		{
			if(node->nn_wild)
			{
				/* Remember where we found the last parameter that was
				 * added due to pattern matching.
				 */
				if(last_wild == 0)
					last_wild = position;
			}
			else
			{
				/* This is not a parameter which was added due to pattern
				 * matching. But if we added one before, we will want to
				 * sort all these parameters alphabetically.
				 */
				if(last_wild != 0)
				{
					size_t num_elements;

					assert( position > last_wild );
					assert( last_wild < argument_list_size );

					/* How many parameters would have to be sorted? */
					num_elements = position - last_wild;
					if(num_elements > 1)
						qsort(&table[last_wild],num_elements,sizeof(*table),(int (*)(const void *,const void *))compare);

					last_wild = 0;
				}
			}

			assert( position < argument_list_size );

			table[position] = node->nn_name;
		}

		/* If necessary, take care of the last entries in the list. */
		if(last_wild != 0)
		{
			size_t num_elements;

			assert( position > last_wild );
			assert( last_wild < argument_list_size );

			/* How many parameters would have to be sorted? */
			num_elements = position - last_wild;
			if(num_elements > 1)
				qsort(&table[last_wild],num_elements,sizeof(*table),(int (*)(const void *,const void *))compare);
		}

		assert( position == argument_list_size );

		/* This terminates the table. */
		table[position] = NULL;
	}
	else
	{
		/* Throw the contents of the list away. There is nothing worth keeping. */
		while((node = (struct name_node *)RemHead((struct List *)&argument_list)) != NULL)
			free(node);
	}

	error = OK;

 out:

	anchor = NULL;

	if(ap != NULL)
	{
		MatchEnd(ap);

		free(ap);
	}

	if(error != OK)
	{
		errno = error;

		perror(__argv[0]);
		abort();
	}

	free(__quote_vector);
	__quote_vector = NULL;

	PROFILE_ON();

	return(error);
}
