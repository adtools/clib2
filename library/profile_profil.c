/*
 * $Id$
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
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

#include <proto/exec.h>
#include <exec/interrupts.h>
#include <interfaces/performancemonitor.h>
#include <resources/performancemonitor.h>
#include <unistd.h>

static struct Interrupt CounterInt;
static struct PerformanceMonitorIFace *IPM;

static struct IntData
{
	struct PerformanceMonitorIFace *IPM;
	uint16 *Buffer;
	uint32 BufferSize;
	uint32 Offset;
	uint32 Scale;
	uint32 CounterStart;
} ProfileData;

uint32 GetCounterStart(void);
uint32 CounterIntFn(struct ExceptionContext *, struct ExecBase *, struct IntData *);


uint32
GetCounterStart(void)
{
	uint64 fsb;
	double bit0time;
	uint32 count;

	GetCPUInfoTags(
			GCIT_FrontsideSpeed,	&fsb,
			TAG_DONE);

	/* Timebase ticks at 1/4 of FSB */
	bit0time = 8.0 / (double)fsb;
	count = (uint32)(0.01 / bit0time);

	return 0x80000000 - count;
}

uint32
CounterIntFn(struct ExceptionContext *ctx, struct ExecBase *ExecBase,
	struct IntData *ProfileData)
{
	uint32 sia = (uint32)ProfileData->IPM->GetSampledAddress();

	/* Silence compiler */
	(void)ExecBase;
	(void)ctx;

	sia = ((sia - ProfileData->Offset) * ProfileData->Scale) >> 16;

	if (sia <= (ProfileData->BufferSize>>1))
	{
		//if (ProfileData->Buffer[sia] != 0xffff)
			ProfileData->Buffer[sia]++;
	}

	IPM->CounterControl(1, ProfileData->CounterStart, PMCI_Transition);

	return 1;
}


int
profil(unsigned short *buffer, size_t bufSize, size_t offset, unsigned int scale)
{
	APTR Stack;

	if (buffer == 0)
	{
		if (!IPM)
		{
			return 0;
		}

		Stack = SuperState();
		IPM->EventControlTags(
			PMECT_Disable, 			PMEC_MasterInterrupt,
			TAG_DONE);

		IPM->SetInterruptVector(1, 0);

		IPM->Unmark(0);
		IPM->Release();
		if (Stack) UserState(Stack);

		return 0;
	}

	IPM = (struct PerformanceMonitorIFace *)
			OpenResource("performancemonitor.resource");

	if (!IPM || IPM->Obtain() != 1)
	{
		return 0;
	}

	Stack = SuperState();

	/* Init IntData */
	ProfileData.IPM = IPM;
	ProfileData.Buffer = buffer;
	ProfileData.BufferSize = bufSize;
	ProfileData.Offset = offset;
	ProfileData.Scale = scale;
	ProfileData.CounterStart = GetCounterStart();

	/* Set interrupt vector */
	CounterInt.is_Code = (void (*)(void))CounterIntFn;
	CounterInt.is_Data = &ProfileData;
	IPM->SetInterruptVector(1, &CounterInt);

	/* Prepare Performance Monitor */
	IPM->MonitorControlTags(
			PMMCT_FreezeCounters,			PMMC_Unmarked,
			PMMCT_RTCBitSelect,				PMMC_BIT0,
			TAG_DONE);
	IPM->CounterControl(1, ProfileData.CounterStart, PMCI_Transition);

	IPM->EventControlTags(
		PMECT_Enable, 			1,
		PMECT_Enable, 			PMEC_MasterInterrupt,
		TAG_DONE);

	IPM->Mark(0);

	if (Stack) UserState(Stack);

	return 0;
}
