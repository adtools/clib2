/*
 * $Id: utsname_uname.c,v 1.7 2006-11-13 09:25:28 obarthel Exp $
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

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#include <sys/utsname.h>

/****************************************************************************/

#if defined(SOCKET_SUPPORT) && !defined(_SOCKET_HEADERS_H)
#include "socket_headers.h"
#endif /* SOCKET_SUPPORT && !_SOCKET_HEADERS_H */

#ifndef	_STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#define	OSNAME "AmigaOS"

/****************************************************************************/

#ifdef	__amigaos4__
#define	ARCH "ppc"
#else
#define	ARCH "m68k" /* XXX: How should Coldfire accelerators be handled? */
#endif /* __amigaos4__ */

/****************************************************************************/

int
uname(struct utsname *info)
{
	struct Library * VersionBase;
	int Version,Revision;
	const char * version_string;
	int result = ERROR;

	ENTER();

	SHOWPOINTER(info);

	if(info == NULL)
	{
		__set_errno(EFAULT);
		goto out;
	}

	strlcpy(info->sysname,OSNAME,sizeof(info->sysname));

	#if defined(SOCKET_SUPPORT)
	{
		__gethostname((STRPTR)info->nodename,sizeof(info->nodename));
	}
	#else
	{
		strlcpy(info->nodename,"localhost",sizeof(info->nodename));
	}
	#endif /* SOCKET_SUPPORT */

	VersionBase = OpenLibrary("version.library",0L);
	if(VersionBase != NULL)
	{
		Version		= VersionBase->lib_Version;
		Revision	= VersionBase->lib_Revision;

		CloseLibrary(VersionBase);
	}
	else
	{
		Version		= 0;
		Revision	= 0;
	}

	snprintf(info->release,sizeof(info->release),"%d.%d",Version,Revision);

	/*
	 * This is mostly a stab in the dark. Is there any "official" way of finding out the OS version?
	 * Also, this could be more detailed e.g. "3.9-BB2" instead of just "3.9".
	 * For the curious:
	 *
	 *  V.R		OS Version
	 *
	 * 30.x		1.0 (Never released)
	 * 31.x		1.1 (NTSC)
	 * 32.x		1.1 (PAL)
	 * 33.x		1.2
	 * 34.x		1.3
	 * 35.x		1.3 (with A2024 support)
	 * 36.x		2.0-2.02
	 * 37.x		2.04-2.05
	 * 38.x		2.1
	 * 39.x		3.0
	 * 40.x		3.1
	 * 44.2		3.5
	 * 44.4		3.5-BB1
	 * 44.5		3.5-BB2
	 * 45.1		3.9
	 * 45.2		3.9-BB1
	 * 45.3		3.9-BB2
	 */

	if (46 <= Version && Version <= 52)
		version_string = "4.0";
	else if (Version == 45)
		version_string = "3.9";
	else if (Version == 44)
		version_string = "3.5";
	else if (40 <= Version && Version <= 43)
		version_string = "3.1";
	else if (Version == 39)
		version_string = "3.0";
	else if (Version == 38)
		version_string = "2.1";
	else if (Version == 37)
		version_string = "2.0";
	else
		version_string = "unknown";

	strlcpy(info->version,version_string,sizeof(info->version));

	strlcpy(info->machine,ARCH,sizeof(info->machine));

	SHOWSTRING(info->sysname);
	SHOWSTRING(info->nodename);
	SHOWSTRING(info->release);
	SHOWSTRING(info->version);
	SHOWSTRING(info->machine);

	result = OK;

out:

	RETURN(result);
	return(result);
}
