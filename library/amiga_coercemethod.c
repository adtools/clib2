/*
 * $Id: amiga_coercemethod.c,v 1.2 2005-01-02 09:07:07 obarthel Exp $
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

#ifndef __PPC__

/****************************************************************************/

#include <intuition/intuition.h>
#include <intuition/classes.h>

#include <clib/alib_protos.h>

/****************************************************************************/

#include "debug.h"

/****************************************************************************/

ULONG
CoerceMethodA(Class * cl,Object * obj,Msg msg)
{
	ULONG result = 0;

	ENTER();

	SHOWPOINTER(cl);
	SHOWPOINTER(obj);
	SHOWPOINTER(msg);

	assert( cl != NULL && obj != NULL );

	if(cl != NULL && obj != NULL)
		result = CallHookA(&cl->cl_Dispatcher,obj,msg);

	RETURN(result);
	return(result);
}

/****************************************************************************/

ULONG
CoerceMethod(Class *cl, Object *obj, ULONG method_id, ...)
{
	ULONG result = 0;

	ENTER();

	SHOWPOINTER(cl);
	SHOWPOINTER(obj);
	SHOWVALUE(method_id);

	assert( cl != NULL && obj != NULL );

	if(cl != NULL && obj != NULL)
		result = CoerceMethodA(cl,obj,(Msg)&method_id);

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* __PPC__ */
