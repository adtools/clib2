#ifndef _GMON_H
#define _GMON_H

#include <exec/types.h>

struct gmonhdr
{
	uint32 	lpc;
	uint32 	hpc;
	int 	ncnt;
	int 	version;
	int 	profrate;
	int		reserved[3];
};

#define GMONVERSION 0x00051879

#define HISTCOUNTER uint16
// I am sure we can make these bigger
#define HISTFRACTION 2
#define HASHFRACTION 4

#define ARCDENSITY 	2
#define MINARCS		50
#define MAXARCS		((1 << (8 * sizeof(HISTCOUNTER)))-2)

struct tostruct
{
	uint32 	selfpc;
	int32	count;
	uint16	link;
	uint16 	pad;
};

struct rawarc
{
	uint32 	raw_frompc;
	uint32	raw_selfpc;
	int32	raw_count;
};

#define ROUNDDOWN(x,y) (((x)/(y))*(y))
#define ROUNDUP(x,y)   ((((x)+(y)-1)/(y))*(y))

struct gmonparam
{
	int 				state;
	uint16 *			kcount;
	uint32				kcountsize;
	uint16 *			froms;
	uint32				fromssize;
	struct tostruct *	tos;
	uint32				tossize;
	int32 				tolimit;
	uint32				lowpc;
	uint32				highpc;
	uint32				textsize;
	uint32				hashfraction;
	uint8 *				memory;
};

extern struct gmonparam _gmonparam;

enum 
{
	kGmonProfOn 		= 0,
	kGmonProfBusy 		= 1,
	kGmonProfError		= 2,
	kGmonProfOff		= 3
};

enum
{
	kGprofState 		= 0,
	kGprofCount			= 1,
	kGprofFroms			= 2,
	kGprofTos			= 3,
	kGprofGmonParam		= 4
};

#endif
