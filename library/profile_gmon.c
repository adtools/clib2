#include <exec/exec.h>
#include <dos/dos.h>
#include <libraries/elf.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/elf.h>
#include <stdio.h>

#define SCALE_1_TO_1 0x10000L

#include "profile_gmon.h"
#undef DebugPrintF
#define dprintf(format, args...)((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF("[%s] " format, __PRETTY_FUNCTION__ , ## args)

struct gmonparam _gmonparam =
{
	state: kGmonProfOn
};

static unsigned int s_scale;

void moncontrol(int);
void monstartup(uint32, uint32);
void moncleanup(void);
void mongetpcs(uint32* lowpc, uint32 *highpc);

extern int profil(uint16 *buffer, uint32 bufSize,
	uint32 offset, uint32 scale);

void
monstartup(uint32 low_pc, uint32 high_pc)
{
	uint8 *cp;
	uint32 lowpc, highpc;
	struct gmonparam *p = &_gmonparam;
	dprintf("in monstartup)\n");
	/*
	 * If we don't get proper lowpc and highpc, then
	 * we'll try to get them from the elf handle.
	 */
	if (low_pc == 0 && high_pc == 0)
	{
		mongetpcs(&lowpc, &highpc);
	}
	else
	{
		lowpc = low_pc;
		highpc = high_pc;
	}

	/*
	 * Round lowpc and highpc to multiples of the density
	 * to prevent using floating point scaling
	 */
	p->lowpc 	= ROUNDDOWN(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
	p->highpc	= ROUNDUP(highpc, HISTFRACTION * sizeof(HISTCOUNTER));

	/* Size of the text segment */
	p->textsize = p->highpc - p->lowpc;

	/*
	 * Size of the histogram. Due to the nature of PowerPC code,
	 * we can safely use a histogram fraction of at least 4, since
	 * every instruction is exactly one word wide and always aligned.
	 */
	p->kcountsize = p->textsize / HISTFRACTION;

	/*
	 * The hash table size
	 */
	p->hashfraction = HASHFRACTION;
	p->fromssize = p->textsize / p->hashfraction;

	p->tolimit = p->textsize * ARCDENSITY / 100;
	if (p->tolimit < MINARCS)
		p->tolimit = MINARCS;
	else if (p->tolimit > MAXARCS)
		p->tolimit = MAXARCS;

	p->tossize = p->tolimit * sizeof(struct tostruct);

	dprintf("lowpc = %p, highpc = %p\n", lowpc, highpc);
	dprintf("textsize = %d\n", p->textsize);
	dprintf("kcountsize = %d\n", p->kcountsize);
	dprintf("fromssize = %d\n", p->fromssize);
	dprintf("tolimit = %d, tossize = %d\n", p->tolimit, p->tossize);

	cp = (uint8*)AllocMem(p->kcountsize + p->fromssize + p->tossize,
		MEMF_CLEAR);
	if (!cp)
	{
		p->state = kGmonProfError;
		return;
	}

	p->memory = cp;
	p->tos = (struct tostruct *)cp;
	cp += p->tossize;

	p->kcount = (uint16 *)cp;
	cp += p->kcountsize;

	p->froms = (uint16 *)cp;

	p->tos[0].link = 0;

	/* Verify granularity for sampling */
	if (p->kcountsize < p->textsize)
		/* FIXME Avoid floating point */
		s_scale = ((float)p->kcountsize / p->textsize) * SCALE_1_TO_1;
	else
		s_scale = SCALE_1_TO_1;

	s_scale >>= 1;
	dprintf("Enabling monitor\n");
	moncontrol(1);
}

void
moncontrol(int mode)
{
	struct gmonparam *p = &_gmonparam;

	if (mode)
	{
		/* Start profiling. */
		profil((uint16 *)p->kcount, (size_t)p->kcountsize,
		   p->lowpc, s_scale);
		p->state = kGmonProfOn;
	}
	else
	{
		/* Stop Profiling. */
		profil(NULL, 0, (uint32)0, 0);
		p->state = kGmonProfOff;
	}
}

void
moncleanup(void)
{
	BPTR fd;
	int fromindex;
	int endfrom;
	uint32 frompc;
	int toindex;
	struct rawarc rawarc;
	struct gmonparam *p = &_gmonparam;
	struct gmonhdr gmonhdr, *hdr;
#ifdef DEBUG
	FILE *log;
#endif

	moncontrol(0);

	if (p->state == kGmonProfError)
	{
		fprintf(stderr, "WARNING: Overflow during profiling\n");
	}

	fd = Open("gmon.out", MODE_NEWFILE);
	if (!fd)
	{
		fprintf(stderr, "ERROR: could not open gmon.out\n");
		return;
	}

	hdr = (struct gmonhdr *)&gmonhdr;

	hdr->lpc = 0; //p->lowpc;
	hdr->hpc = p->highpc - p->lowpc;
	hdr->ncnt = (int)p->kcountsize + sizeof(gmonhdr);
	hdr->version = GMONVERSION;
	hdr->profrate = 100; //FIXME:!!

	Write(fd, hdr, sizeof(*hdr));
	Write(fd, p->kcount, p->kcountsize);

	endfrom = p->fromssize / sizeof(*p->froms);

#ifdef DEBUG
	log = fopen("gmon.log", "w");
#endif

	for (fromindex = 0; fromindex < endfrom; fromindex++)
	{
		if (p->froms[fromindex] == 0) continue;

		frompc = 0; /* FIXME: was p->lowpc; needs to be 0 and assumes
		               -Ttext=0 on compile. Better idea? */
		frompc += fromindex * p->hashfraction * sizeof (*p->froms);
		for (toindex = p->froms[fromindex]; toindex != 0;
			toindex = p->tos[toindex].link)
		{
#ifdef DEBUG
			if (log) fprintf(log, "%p called from %p: %d times\n", frompc,
				p->tos[toindex].selfpc,
				p->tos[toindex].count);
#endif
			rawarc.raw_frompc = frompc;
			rawarc.raw_selfpc = p->tos[toindex].selfpc;
			rawarc.raw_count  = p->tos[toindex].count;
			Write(fd, &rawarc, sizeof(rawarc));
		}
	}

#ifdef DEBUG
	if (log) fclose(log);
#endif
	Close(fd);

}

void
mongetpcs(uint32* lowpc, uint32 *highpc)
{
	struct Library *ElfBase = NULL;
	struct ElfIFace *IElf = NULL;
	struct Process *self;
	BPTR seglist;
	Elf32_Handle elfHandle;
	uint32 i;
	Elf32_Shdr *shdr;
	uint32 numSections;

	*lowpc = 0;
	*highpc = 0;

	ElfBase = OpenLibrary("elf.library", 0L);
	if (!ElfBase) goto out;

	IElf = (struct ElfIFace *)GetInterface(ElfBase, "main", 1, NULL);
	if (!IElf) goto out;

	self = (struct Process *)FindTask(0);
	seglist = GetProcSegList(self, GPSLF_CLI | GPSLF_SEG);

	GetSegListInfoTags(seglist,
		GSLI_ElfHandle,		&elfHandle,
	TAG_DONE);

	elfHandle = OpenElfTags(
		OET_ElfHandle,		elfHandle,
	TAG_DONE);

	if (!elfHandle) goto out;

	GetElfAttrsTags(elfHandle, EAT_NumSections, &numSections, TAG_DONE);

	for (i = 0; i < numSections; i++)
	{
		shdr = GetSectionHeaderTags(elfHandle,
			GST_SectionIndex, i,
		TAG_DONE);
		if (shdr && (shdr->sh_flags & SWF_EXECINSTR))
		{
			uint32 base = (uint32)GetSectionTags(elfHandle,
				GST_SectionIndex, i,
			TAG_DONE);
			*lowpc = base;
			*highpc = base + shdr->sh_size;
			break;
		}
	}

	CloseElfTags(elfHandle, CET_ReClose, TRUE, TAG_DONE);

out:
	if (IElf) DropInterface((struct Interface *)IElf);
	if (ElfBase) CloseLibrary(ElfBase);
}


#include "macros.h"

int __profiler_init(void) __attribute__((constructor));
void __profiler_exit(void) __attribute__((destructor));

int __profiler_init(void)
{
	monstartup(0,0);
	return OK;
}

void __profiler_exit(void)
{
	moncleanup();
}
