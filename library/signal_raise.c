/*
 * $Id: signal_raise.c,v 1.5 2005-03-26 11:01:13 obarthel Exp $
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

#ifndef _SIGNAL_HEADERS_H
#include "signal_headers.h"
#endif /* _SIGNAL_HEADERS_H */

/****************************************************************************/

signal_handler_t __signal_handler_table[NUM_SIGNALS] =
{
	SIG_DFL,
	SIG_DFL,
	SIG_DFL,
	SIG_DFL,
	SIG_DFL,
	SIG_DFL
};

/****************************************************************************/

int __signals_blocked;

/****************************************************************************/

int
raise(int sig)
{
	int result = -1;

	ENTER();

	SHOWVALUE(sig);

	assert( SIGABRT <= sig && sig <= SIGTERM );

	if(sig < SIGABRT || sig > SIGTERM)
	{
		SHOWMSG("unknown signal number");

		__set_errno(EINVAL);
		goto out;
	}

	if(FLAG_IS_CLEAR(__signals_blocked,(1 << sig)))
	{
		int table_entry = sig - SIGABRT;
		signal_handler_t handler;

		handler = __signal_handler_table[table_entry];

		/* Revert to the default handler to prevent recursion. */
		__signal_handler_table[table_entry] = SIG_DFL;

		if(handler == SIG_DFL)
		{
			char break_string[80];

			SHOWMSG("this is the default handler");

			/* Don't trigger a recursion. */
			__check_abort_enabled = FALSE;

			Fault(ERROR_BREAK,NULL,break_string,(LONG)sizeof(break_string));

			__print_termination_message((sig == SIGINT) ? break_string : NULL);

			SHOWMSG("bye, bye...");

			/* Note that we drop into the exit() function which
			 * does not trigger the exit trap.
			 */
			_exit(EXIT_FAILURE);
		}
		else if (handler != SIG_IGN)
		{
			SHOWMSG("calling the handler");

			(*handler)(sig);

			SHOWMSG("done.");
		}
	}
	else
	{
		SHOWMSG("that signal is blocked");
	}

	result = 0;

 out:

	RETURN(result);
	return(result);
}
