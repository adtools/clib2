/*
 * $Id: signal_kill.c,v 1.1.1.1 2004-07-26 16:31:05 obarthel Exp $
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

#ifndef _SIGNAL_HEADERS_H
#include "signal_headers.h"
#endif /* _SIGNAL_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

int
kill(pid_t pid, int signal_number)
{
	int result = -1;

	ENTER();

	SHOWVALUE(pid);
	SHOWVALUE(signal_number);

	Forbid();

	if(pid > 0)
	{
		#if 0
		{
			struct Process * cli_process;

			cli_process = FindCliProc((ULONG)pid);
			if(cli_process == NULL)
			{
				SHOWMSG("didn't find the process");

				errno = ESRCH;
				goto out;
			}

			SHOWMSG("found the process");

			if(signal_number == SIGTERM || signal_number == SIGINT)
				Signal((struct Task *)cli_process,SIGBREAKF_CTRL_C);
			else
				SHOWMSG("but won't shut it down");
		}
		#else
		{
			ULONG max_cli_number,i;
			BOOL found = FALSE;

			max_cli_number = MaxCli();

			for(i = 1 ; i <= max_cli_number ; i++)
			{
				if(FindCliProc(i) == (struct Process *)pid)
				{
					found = TRUE;
					break;
				}
			}

			if(NOT found)
			{
				SHOWMSG("didn't find the process");

				errno = ESRCH;
				goto out;
			}

			SHOWMSG("found the process");

			if(signal_number == SIGTERM || signal_number == SIGINT)
				Signal((struct Task *)pid,SIGBREAKF_CTRL_C);
			else
				SHOWMSG("but won't shut it down");
		}
		#endif
	}

	result = 0;

 out:

	Permit();

	RETURN(result);
	return(result);
}
