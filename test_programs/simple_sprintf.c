/*
 * $Id: simple_sprintf.c,v 1.1 2005-05-17 19:15:32 obarthel Exp $
 *
 * :ts=4
 */

#include <exec/execbase.h>

/****************************************************************************/

#define __NOLIBBASE__
#define __USE_INLINE__
#define __NOGLOBALIFACE__

/****************************************************************************/

#include <proto/exec.h>
#include <proto/dos.h>

/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <dos.h>

/****************************************************************************/

struct Library * SysBase;
struct Library * DOSBase;

/****************************************************************************/

#if defined(__amigaos4__)

struct ExecIFace *	IExec;
struct DOSIFace *	IDOS;

#endif /* __amigaos4__ */

/****************************************************************************/

BOOL __check_abort_enabled = FALSE;

/****************************************************************************/

int _start(void);

/****************************************************************************/

int
_start(void)
{
	char string[80];

	SysBase = *(struct Library **)4;

	DOSBase = OpenLibrary("dos.library",37);
	if(DOSBase == NULL)
		goto out;

	#if defined(__amigaos4__)
	{
		IExec = (struct ExecIFace *)((struct ExecBase *)SysBase)->MainInterface;

		IDOS = (struct DOSIFace *)GetInterface(DOSBase, "main", 1, 0);
		if(IDOS == NULL)
			goto out;
	}
	#endif /* __amigaos4__ */

	sprintf(string,"a %s c\n","b");

	Write(Output(),string,strlen(string));

 out:

	#if defined(__amigaos4__)
	{
		if(IDOS != NULL)
			DropInterface((struct Interface *)IDOS);
	}
	#endif /* __amigaos4__ */

	if(DOSBase != NULL)
		CloseLibrary(DOSBase);

	return(0);
}

/****************************************************************************/

void
__check_abort(void)
{
}
