/*
 * $Id: amiga_rexxvars.c,v 1.13 2008-04-14 15:07:37 obarthel Exp $
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

/* We don't want to pull in <clib/alib_protos.h> */
#define CLIB_ALIB_PROTOS_H

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

#include <rexx/rxslib.h>
#include <rexx/errors.h>

/****************************************************************************/

struct Environment;

/****************************************************************************/

#define __NOLIBBASE__
#include <proto/rexxsyslib.h>

/****************************************************************************/

/* This side-steps issues with very old inline header files, predating the
   OS 3.5 SDK, when the library is built for the 68k platform. */
#if defined(__GNUC__) && !defined(__amigaos4__)

#define __GetRexxVarFromMsg(name, buffer, message) ({ \
  STRPTR _GetRexxVarFromMsg_name = (name); \
  STRPTR _GetRexxVarFromMsg_buffer = (buffer); \
  struct RexxMsg * _GetRexxVarFromMsg_message = (message); \
  LONG _GetRexxVarFromMsg__re = \
  ({ \
  register struct RxsLib * const __GetRexxVarFromMsg__bn __asm("a6") = (struct RxsLib *) (RexxSysBase);\
  register LONG __GetRexxVarFromMsg__re __asm("d0"); \
  register STRPTR __GetRexxVarFromMsg_name __asm("a0") = (_GetRexxVarFromMsg_name); \
  register STRPTR __GetRexxVarFromMsg_buffer __asm("a1") = (_GetRexxVarFromMsg_buffer); \
  register struct RexxMsg * __GetRexxVarFromMsg_message __asm("a2") = (_GetRexxVarFromMsg_message); \
  __asm volatile ("jsr a6@(-492:W)" \
  : "=r"(__GetRexxVarFromMsg__re) \
  : "r"(__GetRexxVarFromMsg__bn), "r"(__GetRexxVarFromMsg_name), "r"(__GetRexxVarFromMsg_buffer), "r"(__GetRexxVarFromMsg_message) \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetRexxVarFromMsg__re; \
  }); \
  _GetRexxVarFromMsg__re; \
})

#undef GetRexxVarFromMsg
#define GetRexxVarFromMsg(name, buffer, message) __GetRexxVarFromMsg(name, buffer, message)

#define __SetRexxVarFromMsg(name, value, message) ({ \
  STRPTR _SetRexxVarFromMsg_name = (name); \
  STRPTR _SetRexxVarFromMsg_value = (value); \
  struct RexxMsg * _SetRexxVarFromMsg_message = (message); \
  LONG _SetRexxVarFromMsg__re = \
  ({ \
  register struct RxsLib * const __SetRexxVarFromMsg__bn __asm("a6") = (struct RxsLib *) (RexxSysBase);\
  register LONG __SetRexxVarFromMsg__re __asm("d0"); \
  register STRPTR __SetRexxVarFromMsg_name __asm("a0") = (_SetRexxVarFromMsg_name); \
  register STRPTR __SetRexxVarFromMsg_value __asm("a1") = (_SetRexxVarFromMsg_value); \
  register struct RexxMsg * __SetRexxVarFromMsg_message __asm("a2") = (_SetRexxVarFromMsg_message); \
  __asm volatile ("jsr a6@(-498:W)" \
  : "=r"(__SetRexxVarFromMsg__re) \
  : "r"(__SetRexxVarFromMsg__bn), "r"(__SetRexxVarFromMsg_name), "r"(__SetRexxVarFromMsg_value), "r"(__SetRexxVarFromMsg_message) \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __SetRexxVarFromMsg__re; \
  }); \
  _SetRexxVarFromMsg__re; \
})

#undef SetRexxVarFromMsg
#define SetRexxVarFromMsg(name, value, message) __SetRexxVarFromMsg(name, value, message)

#endif /* __GNUC__ && !__amigaos4__ */

/****************************************************************************/

static struct Library * RexxSysBase;

/****************************************************************************/

#if defined(__amigaos4__)
static struct RexxSysIFace * IRexxSys;
#endif /* __amigaos4__ */

/****************************************************************************/

BOOL CheckRexxMsg(struct RexxMsg *message);
LONG GetRexxVar(struct RexxMsg *message,STRPTR variable_name,STRPTR *buffer_pointer);
LONG SetRexxVar(struct RexxMsg *message,STRPTR variable_name,STRPTR value,LONG length);

/****************************************************************************/

CLIB_CONSTRUCTOR(rexxvars_init)
{
	ENTER();

	RexxSysBase = OpenLibrary(RXSNAME,0);

	#if defined(__amigaos4__)
	{
		if(RexxSysBase != NULL)
		{
			IRexxSys = (struct RexxSysIFace *)GetInterface(RexxSysBase, "main", 1, 0);
			if(IRexxSys == NULL)
			{
				CloseLibrary(RexxSysBase);
				RexxSysBase = NULL;
			}
		}
	}
	#endif /* __amigaos4__ */

	LEAVE();

	CONSTRUCTOR_SUCCEED();
}

/****************************************************************************/

CLIB_DESTRUCTOR(rexxvars_exit)
{
	ENTER();

	#if defined(__amigaos4__)
	{
		if(IRexxSys != NULL)
			DropInterface((struct Interface *)IRexxSys);

		IRexxSys = NULL;
	}
	#endif /* __amigaos4__ */

	if(RexxSysBase != NULL)
	{
		CloseLibrary(RexxSysBase);
		RexxSysBase = NULL;
	}

	LEAVE();
}

/****************************************************************************/

/* This is modeled after the original assembly language code. Except for the
   fact that we compare the library base against a local, static variable
   rather then opening the library for each check. */
BOOL
CheckRexxMsg(struct RexxMsg *message)
{
	BOOL result = FALSE;

	if(RexxSysBase == NULL)
		goto out;

	if(message->rm_LibBase != RexxSysBase)
		goto out;

	if(message->rm_TaskBlock == NULL)
		goto out;

	if(NOT IsRexxMsg(message))
		goto out;

	result = TRUE;

 out:

	return(result);
}

/****************************************************************************/

#if 1

/****************************************************************************/

/* The following function works in about like the original, except that it's
   not reentrant, does not fill in a pointer to the variable itself and
   requires rexxsyslib.library V45. */
LONG
GetRexxVar(struct RexxMsg *message,STRPTR variable_name,STRPTR *buffer_pointer)
{
	static TEXT buffer[256];
	LONG result;

	/* The following uses a function which was added to rexxsyslib.library V45.
	   We therefore have a minimum library version requirement. */
	if(RexxSysBase == NULL || RexxSysBase->lib_Version < 45 || NOT IsRexxMsg(message))
	{
		result = ERR10_010; /* invalid message packet */
		goto out;
	}

	/* The 256 character limit isn't good. This should be done differently. */
	result = GetRexxVarFromMsg(variable_name,buffer,message);
	if(result != 0)
		goto out;

	(*buffer_pointer) = buffer;

 out:

	return(result);
}

/****************************************************************************/

/* The following function works in about like the original, except that it
   ignores the length parameter (the value needs to be a NUL-terminated string)
   and requires rexxsyslib.library V45. */
LONG
SetRexxVar(struct RexxMsg *message,STRPTR variable_name,STRPTR value,ULONG length)
{
	LONG result;

	/* The following uses a function which was added to rexxsyslib.library V45.
	   We therefore have a minimum library version requirement. */
	if(RexxSysBase == NULL || RexxSysBase->lib_Version < 45 || NOT IsRexxMsg(message))
	{
		result = ERR10_010; /* invalid message packet */
		goto out;
	}

	result = SetRexxVarFromMsg(variable_name,value,message);

 out:

	return(result);
}

/****************************************************************************/

#else

/****************************************************************************/

#if defined(__GNUC__) && !defined(__amigaos4__)

/****************************************************************************/

/* A selection of lovingly hand-crafted 68k stub functions which call
  into rexxsyslib.library LVOs which still used to be documented back
  in 1987. */

/****************************************************************************/

#if defined(SMALL_DATA)
#define A4(x) "a4@(" #x ":W)"
#elif defined(SMALL_DATA32)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) #x
#endif /* SMALL_DATA */

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__FreeSpace

__FreeSpace:

	moveal	sp@(4),a0
	moveal	sp@(8),a1
	movel	sp@(12),d0
	movel	a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr		a6@(-120)
	moveal	sp@+,a6
	rts

");

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__GetSpace

__GetSpace:

	moveal	sp@(4),a0
	movel	sp@(8),d0
	movel	a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr		a6@(-114)
	moveal	sp@+,a6
	rts

");

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__IsSymbol

__IsSymbol:
	moveal	sp@(4),a0
	movel	a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr		a6@(-102)
	moveal	sp@+,a6
	moveal	sp@(8),a1
	movel	d1,a1@
	rts

");

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__CurrentEnv

__CurrentEnv:
	moveal	sp@(4),a0
	movel	a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr		a6@(-108)
	moveal	sp@+,a6
	moveal	sp@(8),a1
	movel	a0,a1@
	rts

");

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__FetchValue

__FetchValue:
	moveal	sp@(4),a0
	moveal	sp@(8),a1
	movel	sp@(12),d0
	movel	sp@(16),d1
	movel	a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr		a6@(-72)
	moveal	sp@+,a6
	moveal	sp@(20),a1
	movel	a0,a1@
	moveal	sp@(24),a1
	movel	d1,a1@
	rts

");

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__EnterSymbol

__EnterSymbol:
	moveal	sp@(4),a0
	moveal	sp@(8),a1
	movel	sp@(12),d0
	movel	a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr		a6@(-66)
	moveal	sp@+,a6
	rts

");

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__FreeSpace

__SetValue:
	moveal sp@(4),a0
	moveal sp@(8),a1
	movel sp@(12),d0
	movel a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr a6@(-84)
	moveal sp@+,a6
	rts

");

/****************************************************************************/

asm("

	.text
	.even

	.globl	_RexxSysBase
	.globl	__FreeSpace

__StrcpyN:
	moveal sp@(4),a0
	moveal sp@(8),a1
	movel sp@(12),d0
	movel a6,sp@-
	moveal	"A4(_RexxSysBase)",a6
	jsr a6@(-270)
	moveal sp@+,a6
	moveal sp@(16),a1
	movel d1,a1@
	rts

");

/****************************************************************************/

#endif /* __GNUC__ && !__amigaos4__ */

/****************************************************************************/

/* Function prototypes for the 68k stubs. */
extern VOID _FreeSpace(struct Environment * env,APTR mem,LONG size);
extern APTR _GetSpace(struct Environment * env,LONG size);
extern LONG _IsSymbol(STRPTR name,LONG * symbol_length_ptr);
extern VOID _CurrentEnv(struct RexxTask *task,struct Environment ** environment_ptr);
extern struct Node * _FetchValue(struct Environment * env,struct NexxStr * stem,struct NexxStr * compound,struct Node *symbol_table_node,LONG * is_literal_ptr,struct NexxStr ** value_ptr);
extern struct Node * _EnterSymbol(struct Environment * env,struct NexxStr * stem,struct NexxStr * compound);
extern VOID _SetValue(struct Environment * env,struct NexxStr * value,struct Node * symbol_table_node);
extern ULONG _StrcpyN(STRPTR destination,STRPTR source,LONG length);

/****************************************************************************/

/* Releases a string structure, if it's not owned at the time. */
STATIC VOID
FreeString(struct Environment * environment,struct NexxStr * ns)
{
	/* Not currently owned? */
	if(!(ns->ns_Flags & NSF_KEEP))
		_FreeSpace(environment,ns,sizeof(*ns) + ns->ns_Length + 1); /* struct Environment * a0,APTR block a1,LONG d0 */
}

/****************************************************************************/

/* Allocates and initializes a string structure. */
STATIC struct NexxStr *
MakeString(struct Environment * environment,STRPTR value,LONG length)
{
	struct NexxStr * result = NULL;
	struct NexxStr * ns;
	
	/* Allocate memory for the NexxStr and the NUL-terminated string itself. */
	ns = _GetSpace(environment,sizeof(*ns) + length + 1); /* struct Environment * a0,LONG d0 : APTR d0 */
	if(ns == NULL)
		goto out;
	
	/* Fill in the NexxStr structure, copy the string and remember the hash value for it. */
	ns->ns_Length	= length;
	ns->ns_Flags	= NSF_STRING;
	ns->ns_Hash		= _StrcpyN((STRPTR)ns->ns_Buff,value,length); /* STRPTR a0,STRPTR a1,LONG d0 : ULONG d0 */
	
	result = ns;
		
out:

	return(result);
}

/****************************************************************************/

/* Classifies a symbol and returns the stem and compound parts. */
STATIC LONG
TypeString(STRPTR variable_name,struct Environment * environment,struct NexxStr ** compound_ptr,struct NexxStr ** stem_ptr)
{
	struct NexxStr * compound;
	struct NexxStr * stem = NULL;
	LONG error = ERR10_003; /* no memory available */
	LONG stem_length;
	LONG symbol_length;
	STRPTR dot;
	
	(*compound_ptr)	= NULL;
	(*stem_ptr)		= NULL;
	
	/* The 'compound' part is the entire variable name, including all dots and
	  what's in between them. */
	compound = MakeString(environment,variable_name,strlen(variable_name));
	if(compound == NULL)
		goto out;
	
	/* Find the first dot in the variable name. Everything in front of it
	  constitutes the 'stem' part. If there is no dot in the name, then
	  the 'compound' and 'stem' parts are identical. */
	dot = memchr(compound->ns_Buff,'.',compound->ns_Length);
	if(dot != NULL)
		stem_length = ((char *)dot - (char *)compound->ns_Buff) + 1;
	else
		stem_length = compound->ns_Length;
	
	/* Make a copy of the 'stem' part. */
	stem = MakeString(environment,variable_name,stem_length);
	if(stem == NULL)
		goto out;
	
	/* Figure out if this is a symbol after all. */
	_IsSymbol((STRPTR)stem->ns_Buff,&symbol_length); /* STRPTR a0 : LONG d0, LONG d1 */
	
	/* The entire name must match the stem part. */
	if(symbol_length != stem->ns_Length)
	{
		error = ERR10_040; /* invalid variable name */
		goto out;
	}
	
	(*compound_ptr)	= compound;
	(*stem_ptr)		= stem;
	
	error = 0;
	
out:

	if(error != 0)
	{
		if(compound != NULL)
			FreeString(environment,compound);
			
		if(stem != NULL)
			FreeString(environment,stem);
	}

	return(error);
}

/****************************************************************************/

/* Retrieves the value of a variable from the current storage environment. */
LONG
GetRexxVar(struct RexxMsg *context,STRPTR variable_name,STRPTR * return_value)
{
	struct Environment * environment;
	struct NexxStr * ns;
	struct NexxStr * compound_string;
	struct NexxStr * stem_string;
	LONG is_literal;
	LONG error;
	
	(*return_value) = NULL;
	
	if(!CheckRexxMsg(context))
	{
		error = ERR10_010; /* invalid message packet */
		goto out;
	}
	
	/* Find the current storage environment. */
	_CurrentEnv(context->rm_TaskBlock,&environment); /* struct RexxTask * a0 : struct Environment * a0 */
	
	/* Create the stem and component parts. */
	error = TypeString(variable_name,environment,&compound_string,&stem_string);
	if(error != 0)
		goto out;

	/* Look up the value. NOTE: _FetchValue() will free the two 'struct NexxStr *' provided. */
	_FetchValue(environment,stem_string,compound_string,NULL,&is_literal,&ns); /* struct Environment * a0,struct NexxStr * a1,struct NexxStr * d0,struct Node * d1 : struct NexxStr * a0, LONG d1 */
	
	/* If this is not a literal, return a pointer to the string. */
	if(!is_literal)
		(*return_value) = (STRPTR)ns->ns_Buff;
	
	error = 0;
	
out:
	
	return(error);
}

/****************************************************************************/

/* Assigns a value to a variable in the current storage environment. */
LONG
SetRexxVar(struct RexxMsg *context,STRPTR variable_name,STRPTR value,LONG length)
{
	struct Environment * environment;
	struct NexxStr * compound_string;
	struct NexxStr * stem_string;
	struct Node * symbol_table_node;
	struct NexxStr *value_string;
	LONG error;
	
	/* Make sure the value string is not too long */
	if(length > 65535)
	{
		error = ERR10_009; /* symbol or string too long */
		goto out;
	}
	
	if(!CheckRexxMsg(context))
	{
		error = ERR10_010; /* invalid message packet */
		goto out;
	}
	
	/* Find the current storage environment. */
	_CurrentEnv(context->rm_TaskBlock,&environment); /* struct RexxTask * a0 : struct Environment * a0 */

	/* Create the stem and compound parts */
	error = TypeString(variable_name,environment,&compound_string,&stem_string);
	if(error != 0)
		goto out;

	/* Locate or create the symbol node. NOTE: _EnterSymbol() will put the two 'struct NexxStr *' into
	  the symbol table. It is not nececessary to free them. */
	symbol_table_node = _EnterSymbol(environment,stem_string,compound_string); /* struct Environment a0, struct NexxStr *a1, struct NexxStr * d0 : struct Node * d0 */
	if(symbol_table_node == NULL)
	{
		error = ERR10_003; /* no memory available */
		goto out;
	}
	
	/* Create the value string. */
	value_string = MakeString(environment,value,length);
	if(value_string == NULL)
	{
		error = ERR10_003; /* no memory available */
		goto out;
	}

	/* Install the value string. */
	_SetValue(environment,value_string,symbol_table_node); /* struct Environment *a0, struct NexxStr *a1, struct Node * d0 */
	
	error = 0;

out:
	
	return(error);
}

/****************************************************************************/

#endif
