/*
 * $Id: lib_base.h,v 1.3 2006-01-08 12:06:14 obarthel Exp $
 *
 * :ts=4
 *
 * Amiga shared library skeleton example
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

#ifndef _LIB_BASE_H
#define _LIB_BASE_H

/****************************************************************************/

#ifndef EXEC_EXECBASE_H
#include <exec/execbase.h>
#endif /* EXEC_EXECBASE_H */

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif /* EXEC_SEMAPHORES_H */

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif /* DOS_DOS_H */

/****************************************************************************/

#ifndef _LIB_USER_H
#include "lib_user.h"
#endif /* _LIB_USER_H */

/****************************************************************************/

#if defined(USE_EXEC)
#undef USE_EXEC
#endif /* USE_EXEC */

/****************************************************************************/

#if defined(__amigaos4__)

/****************************************************************************/

#define USE_EXEC(sb) \
	struct ExecIFace * IExec = (sb)->sb_IExec

#define USE_LIBBASE(base) \
	struct SkeletonIFace * ISkeleton = (struct SkeletonIFace *)(base); \
	struct SkeletonBase * sb = (struct SkeletonBase *)ISkeleton->Data.LibBase

/****************************************************************************/

#else

/****************************************************************************/

#define USE_EXEC(sb) \
	struct Library * SysBase = (sb)->sb_SysBase

#define USE_LIBBASE(base) \
	struct SkeletonBase * sb = (struct SkeletonBase *)(base)

/****************************************************************************/

#endif /* __amigaos4__ */

/****************************************************************************/

struct SkeletonBase
{
	struct Library			sb_Library;
	struct SignalSemaphore	sb_Semaphore;
	BPTR					sb_SegmentList;
	struct Library *		sb_SysBase;

	#if defined(__amigaos4__)
	struct ExecIFace *		sb_IExec;
	#endif /* __amigaos4__ */

	struct UserData *		sb_UserData;
};

/****************************************************************************/

#endif /* _LIB_BASE_H */
