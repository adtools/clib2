/*
 * $Id: sas_cxferr.c,v 1.1.1.1 2004-07-26 16:31:03 obarthel Exp $
 *
 * :ts=4
 *
 * Adapted from SAS/C runtime library code.
 */

#ifndef _SIGNAL_HEADERS_H
#include "signal_headers.h"
#endif /* _SIGNAL_HEADERS_H */

/****************************************************************************/

void __stdargs
_CXFERR(int code)
{
	raise(SIGFPE);
}
