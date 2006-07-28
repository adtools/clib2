/*
 * $Id: systeminfo_sysinfo.c,v 1.2 2006-07-28 14:37:27 obarthel Exp $
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

#include <sys/systeminfo.h>
#include <sys/utsname.h>

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#if defined(SOCKET_SUPPORT) && !defined(_SOCKET_HEADERS_H)
#include "socket_headers.h"
#endif /* SOCKET_SUPPORT && !_SOCKET_HEADERS_H */

#ifndef	_STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#if !defined(__amigaos4__)
#include <exec/execbase.h>

/* The order here is important. */
static const struct { UWORD flags; char *name; } isanames[] =
{
	/* Apparently there is a 060 card sans FPU, but how to detect it without a LineF exception?
	   Perhaps I should see if I can find my old MC68060 manual. */
	{AFF_68060|AFF_68060,	"68060+fpu060 "},
	{AFF_68060|AFF_68060,	"68060+fpu "},
	{AFF_68040|AFF_FPU40,	"68040+fpu040 "},
	{AFF_68040|AFF_68882,	"68040+fpu "},
	{AFF_68040|AFF_68881,	"68040+fpu "},
	{AFF_68040,				"68040 "},
	{AFF_68030|AFF_68882,	"68030+fpu "},
	{AFF_68030|AFF_68881,	"68030+fpu "},
	{AFF_68020|AFF_68882,	"68020+fpu "},
	{AFF_68020|AFF_68881,	"68020+fpu "},
	{AFF_68030,				"68030 "},
	{AFF_68020,				"68020 "},
	{AFF_68010,				"68010 "},
};
#endif /* !__amigaos4__ */

/****************************************************************************/

long
sysinfo(int cmd,char *buf,long buflen)
{
	struct utsname info;
	long ret = -1;
	
	ENTER();
	
	if(buf == NULL)
	{
		__set_errno(EFAULT);
		goto out;
	}
	
	uname(&info); /* Most of the keys use this. */

	switch(cmd)
	{
		case SI_SYSNAME:

			ret = strlcpy(buf,info.sysname,buflen);
			break;

		case SI_HOSTNAME:

			ret = strlcpy(buf,info.nodename,buflen);
			break;

		case SI_SET_HOSTNAME:

			__set_errno(EPERM);
			goto out;

		case SI_RELEASE:

			ret = strlcpy(buf,info.release,buflen);
			break;

		case SI_VERSION:

			ret = strlcpy(buf,info.version,buflen);
			break;

		case SI_MACHINE:

			ret = strlcpy(buf,"amiga",buflen);
			break;

		case SI_ARCHITECTURE:

			ret = strlcpy(buf,info.machine,buflen);
			break;

		case SI_ISALIST:

			#if defined(__amigaos4__)
			{
				/* Solaris uses "ppc" as the isa name, but gcc uses powerpc. "ppc+altivec" follows the
				   convention of specifying isa+ext from Solaris on SPARC machines. This way of implementing
				   SI_ISALIST seemed like a good idea at the time. */
				uint32 vecu;

				GetCPUInfoTags(GCIT_VectorUnit,&vecu,TAG_DONE);

				if(vecu == VECTORTYPE_ALTIVEC || vecu == VECTORTYPE_VMX) /* AltiVec and VMX are the same. */
					s = "ppc+altivec ppc common";
				else
					s = "ppc common";

				ret = strlcpy(buf,s,buflen);
			}
			#else
			{
				UWORD attention_flags = ((struct ExecBase *)SysBase)->AttnFlags; 
				char * last_added = "";
				size_t i;

				ret = 0;

				strlcpy(buf,"",buflen);

				for(i = 0 ; i < NUM_ENTRIES(isanames) ; i++)
				{
					if((attention_flags & isanames[i].flags) == isanames[i].flags)
					{
						if(strcmp(last_added,isanames[i].name) != SAME)
						{
							ret += strlcat(buf,isanames[i].name,buflen);

							last_added = isanames[i].name;
						}
					}
				}

				ret += strlcat(buf,"68000",buflen);
			}
			#endif

			break;

		case SI_PLATFORM:	/* TODO: Figure out a good way of checking this. */

			ret = strlcpy(buf,"Amiga,Unknown",buflen);
			break;

		case SI_HW_PROVIDER:	/* TODO: Figure out a good way of checking this. */

			ret = strlcpy(buf,"Unknown",buflen);
			break;

		case SI_HW_SERIAL:

			#if defined(SOCKET_SUPPORT)
			{
				ret = snprintf(buf,buflen,"%u",gethostid());
			}
			#else
			{
				ret = strlcpy(buf,"0",buflen);
			}
			#endif

			break;

		default:

			__set_errno(EINVAL);
			goto out;
	}
	
 out:
	
	RETURN(ret);
	return(ret);
}
