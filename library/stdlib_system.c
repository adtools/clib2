/*
 * $Id: stdlib_system.c,v 1.1.1.1 2004-07-26 16:32:12 obarthel Exp $
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

int
system(const char * command)
{
	char * command_copy = NULL;
	int result;

	ENTER();

	if(command == NULL)
		SHOWPOINTER(command);
	else
		SHOWSTRING(command);

	if(__check_abort_enabled)
		__check_abort();

	/* A NULL pointer for the name of the command to execute is
	 * really a query to find out whether a shell is available.
	 * We return 1 (TRUE).
	 */
	if(command == NULL)
	{
		result = 1;
	}
	else
	{
		STATIC const struct TagItem system_tags[2] =
		{
			{ SYS_UserShell,	TRUE },
			{ TAG_END,			0 }
		};

		#if defined(UNIX_PATH_SEMANTICS)
		struct name_translation_info command_nti;
		#endif /* UNIX_PATH_SEMANTICS */

		#if defined(UNIX_PATH_SEMANTICS)
		{
			if(__unix_path_semantics)
			{
				char just_the_command_name[MAXPATHLEN+1];
				BOOL need_quotes = FALSE;
				char * command_name;
				size_t command_len;
				BOOL have_quote;
				size_t len;
				size_t i;

				/* We may need to replace the path specified for the command,
				   so let's figure out first how long the command name,
				   including everything, really is. */
				len = strlen(command);
				command_len = len;

				have_quote = FALSE;
				for(i = 0 ; i < len ; i++)
				{
					if(command[i] == '\"')
					{
						need_quotes = TRUE;
						have_quote ^= TRUE;
					}

					if((command[i] == ' ' || command[i] == '\t') && NOT have_quote)
					{
						command_len = i;
						break;
					}
				}

				/* This may be too long for proper translation... */
				if(command_len > MAXPATHLEN)
				{
					errno = ENAMETOOLONG;

					result = -1;
					goto out;
				}

				/* Grab the command name itself, then have it translated. */
				command_name = just_the_command_name;
				for(i = 0 ; i < command_len ; i++)
				{
					if(command[i] != '\"')
						(*command_name++) = command[i];
				}

				(*command_name) = '\0';

				command_name = just_the_command_name;

				if(__translate_unix_to_amiga_path_name((const char **)&command_name,&command_nti) != 0)
				{
					result = -1;
					goto out;
				}

				/* Now put it all together again */
				command_copy = malloc(1 + strlen(command_name) + 1 + strlen(&command[command_len]) + 1);
				if(command_copy == NULL)
				{
					errno = ENOMEM;
					result = -1;
					goto out;
				}

				if(need_quotes)
				{
					command_copy[0] = '\"';

					strcpy(&command_copy[1],command_name);
					strcat(command_copy,"\"");
				}
				else
				{
					strcpy(command_copy,command_name);
				}

				strcat(command_copy,&command[command_len]);

				command = command_copy;
			}
		}
		#endif /* UNIX_PATH_SEMANTICS */

		SHOWSTRING(command);

		PROFILE_OFF();
		result = SystemTagList((STRPTR)command, (struct TagItem *)system_tags);
		PROFILE_ON();
	}

 out:

	if(command_copy != NULL)
		free(command_copy);

	RETURN(result);
	return(result);
}
