/*
 * $Id: signal_headers.h,v 1.2 2004-11-14 11:06:27 obarthel Exp $
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
#define _SIGNAL_HEADERS_H

/****************************************************************************/

#include <exec/libraries.h>

#include <dos/dosextens.h>

/****************************************************************************/

#include <proto/exec.h>
#include <proto/dos.h>

/****************************************************************************/

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dos.h>

/****************************************************************************/

#include "macros.h"
#include "debug.h"

/****************************************************************************/

#define NUM_SIGNALS (SIGTERM - SIGABRT + 1)

/****************************************************************************/

typedef void (*signal_handler_t)(int sig);

/****************************************************************************/

extern signal_handler_t	NOCOMMON __signal_handler_table[NUM_SIGNALS];

/****************************************************************************/

extern int NOCOMMON __signals_blocked;

/****************************************************************************/

extern BOOL NOCOMMON __termination_message_printed;
extern BOOL NOCOMMON __termination_message_disabled;

/****************************************************************************/

extern const char NOCOMMON __abnormal_program_termination[];

/****************************************************************************/

#ifndef _STDLIB_PROTOS_H
#include "stdlib_protos.h"
#endif /* _STDLIB_PROTOS_H */

/****************************************************************************/

#endif /* _SIGNAL_HEADERS_H */
