/* 
 * $Id: crtend.c,v 1.2 2005-03-09 21:07:25 obarthel Exp $
 *
 * :ts=4
 *
 * End markers for the CTOR and DTOR list.
 */

#if defined(__amigaos4__)

/****************************************************************************/

static void (*__CTOR_LIST__[1]) (void) __attribute__((used, section(".ctors"), aligned(sizeof(void (*)(void))) ));
static void (*__DTOR_LIST__[1]) (void) __attribute__((used, section(".dtors"), aligned(sizeof(void (*)(void))) ));

/****************************************************************************/

#endif /*__amigaos4__ */

