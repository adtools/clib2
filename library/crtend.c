/* 
 * $Id: crtend.c,v 1.1.1.1 2004-07-26 16:30:22 obarthel Exp $
 *
 * :ts=4
 *
 * End markers for the CTOR and DTOR list.
 */

#if defined(__amigaos4__)

/****************************************************************************/

static volatile void (*__CTOR_LIST__[1]) (void) __attribute__((used,section(".ctors"))) = { (void *)0 };
static volatile void (*__DTOR_LIST__[1]) (void) __attribute__((used,section(".dtors"))) = { (void *)0 };

/****************************************************************************/

#endif /*__amigaos4__ */
