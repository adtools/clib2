/*
 * $Id: sas_profile.c,v 1.7 2006-04-05 06:43:56 obarthel Exp $
 *
 * :ts=4
 *
 * Adapted from SAS/C runtime library code.
 */

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif /* EXEC_MEMORY_H */

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif /* EXEC_PORTS_H */

#ifndef EXEC_DEVICES_H
#include <exec/devices.h>
#endif /* EXEC_DEVICES_H */

#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif /* DEVICES_TIMER_H */

/****************************************************************************/

#define __NOLIBBASE__
#define __NOGLOBALIFACE__

#ifndef PROTO_EXEC_H
#include <proto/exec.h>
#endif /* PROTO_EXEC_H */

#ifndef PROTO_TIMER_H
#include <proto/timer.h>
#endif /* PROTO_TIMER_H */

/****************************************************************************/

extern struct Library * SysBase;

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

#include "macros.h"
#include "debug.h"

/****************************************************************************/

/* A quick workaround for the timeval/timerequest->TimeVal/TimeRequest
   change in the recent OS4 header files. */

#if defined(__NEW_TIMEVAL_DEFINITION_USED__)

#define timerequest	TimeRequest
#define tr_node		Request
#define tr_time		Time

#endif /* __NEW_TIMEVAL_DEFINITION_USED__ */

/****************************************************************************/

struct SPROFMSG
{
	struct Message		message;

	struct Process *	process;
	ULONG				clock_value;
	char *				id;
	ULONG				stack_pointer;
	ULONG				flags;
};

/****************************************************************************/

/* Values for the 'flags' field of SPROFMSG */
#define SPROF_INIT		0x00000001	/* Initialize connection */
#define SPROF_ENTRY		0x00000002	/* Function entry */
#define SPROF_EXIT		0x00000004	/* Function exit */
#define SPROF_TERM		0x00000008	/* Terminate connection, program continues */
#define SPROF_ABORT		0x00000010	/* Abort program */
#define SPROF_DENIED	0x00000020	/* Connection refused */

/****************************************************************************/

static struct Library * TimerBase;

/****************************************************************************/

static struct MsgPort *		profiler_port;
static struct MsgPort *		reply_port;

static struct MsgPort *		time_port;
static struct timerequest *	time_request;

static struct Process *		this_process;

static struct EClockVal		start_time;
static ULONG				eclock_frequency;
static struct EClockVal		last_time;
static ULONG				overhead;

static ULONG				num_messages_allocated;

static BOOL					initialized;

/****************************************************************************/

STATIC int send_profiler_message(ULONG clock_value, char *id, ULONG flags);
STATIC ULONG get_current_time(void);
STATIC void update_overhead(void);
STATIC void __profile_init(void);
STATIC void __profile_exit(void);

/****************************************************************************/

void ASM _PROLOG(REG(a0, char *id));
void ASM _EPILOG(REG(a0, char *id));

/****************************************************************************/

STATIC int
send_profiler_message(ULONG clock_value,char * id,ULONG flags)
{
	extern long __builtin_getreg(int);

	struct SPROFMSG * spm;
	int result = ERROR;

	spm = (struct SPROFMSG *)GetMsg(reply_port);
	if(spm != NULL)
	{
		if(flags != SPROF_TERM && spm->flags == SPROF_TERM)
		{
			PutMsg(reply_port,(struct Message *)spm);

			__profile_exit();
			goto out;
		}
	}
	else
	{
		spm = AllocMem(sizeof(*spm),MEMF_ANY|MEMF_PUBLIC|MEMF_CLEAR);
		if(spm == NULL)
		{
			__profile_exit();
			goto out;
		}

		spm->message.mn_Length		= sizeof(*spm);
		spm->message.mn_ReplyPort	= reply_port;
		spm->process				= this_process;

		num_messages_allocated++;
	}

	spm->clock_value	= clock_value;
	spm->id				= id;
	spm->stack_pointer	= __builtin_getreg(15); /* getreg(REG_A7) */
	spm->flags			= flags;

	PutMsg(profiler_port,(struct Message *)spm);
	WaitPort(reply_port);

	result = 0;

 out:

	return(result);
}

/****************************************************************************/

STATIC ULONG
get_current_time(void)
{
	ULONG result;

	ReadEClock(&last_time);

	result = (last_time.ev_lo - start_time.ev_lo - overhead) / eclock_frequency;

	return(result);
}

/****************************************************************************/

STATIC void
update_overhead(void)
{
	struct EClockVal ev;

	ReadEClock(&ev);

	overhead += (ev.ev_lo - last_time.ev_lo);
}

/****************************************************************************/

void ASM
_PROLOG(REG(a0,char * id))
{
	if(initialized)
	{
		if(send_profiler_message(get_current_time(),id,SPROF_ENTRY) == OK)
			update_overhead();
	}
}

/****************************************************************************/

void ASM
_EPILOG(REG(a0,char * id))
{
	if(initialized)
	{
		if(send_profiler_message(get_current_time(),id,SPROF_EXIT) == OK)
			update_overhead();
	}
}

/****************************************************************************/

STATIC VOID
__profile_init(void)
{
	struct SPROFMSG * spm;
	BOOL ready = FALSE;

	this_process = (struct Process *)FindTask(NULL);

	time_port = CreateMsgPort();
	if(time_port == NULL)
		goto out;

	time_request = (struct timerequest *)CreateIORequest(time_port,sizeof(*time_request));
	if(time_request == NULL)
		goto out;

	if(OpenDevice(TIMERNAME,UNIT_ECLOCK,(struct IORequest *)time_request,0) != OK)
		goto out;

	TimerBase = (struct Library *)time_request->tr_node.io_Device;

	reply_port = CreateMsgPort();
	if(reply_port == NULL)
		goto out;

	Forbid();

	profiler_port = FindPort("SPROF_Profiler");
	if(profiler_port == NULL)
	{
		Permit();

		goto out;
	}

	if(send_profiler_message(0,NULL,SPROF_INIT) != OK)
	{
		Permit();

		goto out;
	}

	initialized = TRUE;

	Permit();

	while((spm = (struct SPROFMSG *)GetMsg(reply_port)) == NULL)
		WaitPort(reply_port),

	PutMsg(reply_port,(struct Message *)spm);

	if(spm->flags == SPROF_DENIED || spm->flags == SPROF_TERM)
		goto out;

	eclock_frequency = ReadEClock(&start_time) / 1000;

	ready = TRUE;

 out:

	if(NOT ready)
		__profile_exit();
}

/****************************************************************************/

STATIC VOID
__profile_exit(void)
{
	if(initialized)
	{
		send_profiler_message(0,NULL,SPROF_TERM);

		initialized = FALSE;
	}

	if(reply_port != NULL)
	{
		struct SPROFMSG * spm;

		while(num_messages_allocated > 0)
		{
			while((spm = (struct SPROFMSG *)GetMsg(reply_port)) == NULL)
				WaitPort(reply_port),

			FreeMem(spm,sizeof(*spm));

			num_messages_allocated--;
		}

		DeleteMsgPort(reply_port);
		reply_port = NULL;
	}

	if(time_request != NULL)
	{
		if(time_request->tr_node.io_Device != NULL)
			CloseDevice((struct IORequest *)time_request);

		DeleteIORequest((struct IORequest *)time_request);
		time_request = NULL;
	}

	if(time_port != NULL)
	{
		DeleteMsgPort(time_port);
		time_port = NULL;
	}
}

/****************************************************************************/

PROFILE_CONSTRUCTOR(profile_init)
{
	ENTER();

	__profile_init();

	LEAVE();

	CONSTRUCTOR_SUCCEED();
}

/****************************************************************************/

PROFILE_DESTRUCTOR(profile_exit)
{
	ENTER();

	__profile_exit();

	LEAVE();
}
