/*
 * $Id: signal_raise.c,v 1.9 2005-07-03 10:36:47 obarthel Exp $
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

/* This table holds pointers to all signal handlers configured at a time. */
signal_handler_t NOCOMMON __signal_handler_table[NUM_SIGNALS] =
{
	SIG_DFL,	/* SIGABRT */
	SIG_DFL,	/* SIGFPE */
	SIG_DFL,	/* SIGILL */
	SIG_DFL,	/* SIGINT */
	SIG_DFL,	/* SIGSEGV */
	SIG_DFL		/* SIGTERM */
};

/****************************************************************************/

/* This holds a mask of all signals whose delivery is currently blocked.
   The sigaddset(), sigblock(), sigprocmask() and sigsetmask() functions
   modify or query it. */
int NOCOMMON __signals_blocked;

/****************************************************************************/

int
raise(int sig)
{
	static int local_signals_blocked;

	int result = ERROR;

	ENTER();

	SHOWVALUE(sig);

	assert( SIGABRT <= sig && sig <= SIGTERM );

	/* This has to be a well-known and supported signal. */
	if(sig < SIGABRT || sig > SIGTERM)
	{
		SHOWMSG("unknown signal number");

		__set_errno(EINVAL);
		goto out;
	}

	/* Can we deliver the signal? */
	if(FLAG_IS_CLEAR(__signals_blocked,		(1 << sig)) &&
	   FLAG_IS_CLEAR(local_signals_blocked,	(1 << sig)))
	{
		signal_handler_t handler;

		/* Which handler is installed for this signal? */
		handler = __signal_handler_table[sig - SIGABRT];

		/* Should we ignore this signal? */
		if(handler != SIG_IGN)
		{
			/* Block delivery of this signal to prevent recursion. */
			SET_FLAG(local_signals_blocked,(1 << sig));

			/* The default behaviour is to drop into abort(), or do
			   something very much like it. */
			if(handler == SIG_DFL)
			{
				SHOWMSG("this is the default handler");

				if(sig == SIGINT)
				{
					char break_string[80];

					/* Turn off ^C checking for good. */
					__check_abort_enabled = FALSE;

					Fault(ERROR_BREAK,NULL,break_string,(LONG)sizeof(break_string));

					__print_termination_message(break_string);

					SHOWMSG("bye, bye...");
				}

				/* Drop straight into abort(), which might call signal()
				   again but is otherwise guaranteed to eventually
				   land us in _exit(). */
				abort();
			}
			else 
			{
				SHOWMSG("calling the handler");

				(*handler)(sig);

				SHOWMSG("done.");
			}

			/* Unblock signal delivery again. */
			CLEAR_FLAG(local_signals_blocked,(1 << sig));
		}
	}
	else
	{
		SHOWMSG("that signal is blocked");
	}

	result = OK;

 out:

	RETURN(result);
	return(result);
}
