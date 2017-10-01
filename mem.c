/*********************************************************************
The GWLIBL library, its source code, and header files, are all
copyright 1987, 1988, 1989, 1990 by Graham Wheeler. All rights reserved.
**********************************************************************/

#include <stdio.h>
#include <alloc.h>
#include "miditool.h"

extern int inDflat;

/*
************************* MEMORY MANAGEMENT *****************************

 The memory management system uses two conditional compilation macros:

	USEOWNMEMBLOCK - tells the system to allocate a block of memory
			of size MEMSPACE bytes, and perform allocations
			from this block. Mem_Free then does nothing.
			This is useful if all allocated chunks can be freed
			in one go at the end, rather than individually.
	MEM_DEBUG - tells the system to include code to keep track of up
			to MEMTABLESIZE pointers, and check all attempts
			to free blocks for legality. Mem_Check can be
			called to report the location and size of all
			currently allocated memory chunks.

 The functions are:
	Mem_Init : Allocate and initialise the memory system. If neither
		of the conditional compiles are being used, this does
		nothing.
	Mem_Check : If MEM_DEBUG is defined, print details of all currently
		allocated chunks. If the Boolean parameter is true, the
		check table is freed up as well.
	Mem_Calloc : A `calloc' substitute. An extra parameter is used to
		identify the caller. This means that if MEM_DEBUG is set
		and an error occursin Mem_Free, the original allocation
		call can be identified.
	Mem_Malloc : As above, but a `malloc' workalike.
	Mem_Free: A `free' workalike, including error checking if MEM_DEBUG
		is defined.
*/

#define MEMTABLESIZE	2500

static struct MemTableEntry
{
	long p;
	int size, who;
} *MemTable=NULL;

void Mem_Init(void)
{
	int i;
	/* Allocate pointer tracking table and initialise it */
	if (MemTable==NULL)
		MemTable = (struct MemTableEntry *)calloc(MEMTABLESIZE,sizeof(struct MemTableEntry));
	/* else print warning */
	if (MemTable==NULL) ;
	else for (i=0;i<MEMTABLESIZE;i++)
		{
		MemTable[i].p = 0l;
		MemTable[i].size = 0;
		}
}

void Mem_Check(int free_table)
{
	int i;
	if (MemTable)
		{
		for (i=0;i<MEMTABLESIZE;i++)
			if (MemTable[i].p != 0l)
				fprintf(stderr,"Memory block of %d bytes currently allocated at %08lX by %d\n",
					MemTable[i].size,MemTable[i].p, MemTable[i].who);
		if (free_table) free(MemTable);
		}
}

void *_Mem_Note(void *tmp, int nelts, int size, int who)
{
	int rtn, nbytes = nelts * size, i;
	if (tmp==NULL)
		if (inDflat)
			ErrorMessage("Calloc fails");
		else fputs("Calloc fails\n",stderr);
	if (MemTable)
		{
		for (i=0;i<MEMTABLESIZE;i++)
			{
		     	if (MemTable[i].p==0l)
				{
				MemTable[i].p = PTR_TO_LONG(tmp);
				MemTable[i].size = nbytes;
				MemTable[i].who = who;
				break;
				}
			}
		if (i>=MEMTABLESIZE)
			if (inDflat)
				ErrorMessage("Mem table full");
			else fputs("Mem table full\n",stderr);
		}
	return tmp;
}

int _ClearNote(void *p) {
	int i;
	long pv;
	if (p==NULL) return 0;
	if (p && MemTable)
		{
		pv = PTR_TO_LONG(p);
		for (i=0;i<MEMTABLESIZE;i++)
			{
			if (MemTable[i].p == pv)
				{
				MemTable[i].p = 0l;
				MemTable[i].size = 0;
				break;
				}
			}
		if (i>=MEMTABLESIZE) {
			if (inDflat)
				ErrorMessage("Illegal attempt to free memory!");
			else fputs("Illegal attempt to free memory!\n",stderr);
			return 0;
			}
		}
	return 1;
}

/*******************************************************************/

void *Mem_Malloc(int nbytes, int who)
{
	return _Mem_Note(malloc(nbytes), nbytes, 1, who);
}

void *Mem_Calloc(int nelts, int size, int who)
{
	return _Mem_Note(calloc(nelts,size), nelts, size, who);
}

void *Mem_Realloc(void *old, int nbytes, int who)
{
	_ClearNote(old);
	return _Mem_Note(realloc(old,nbytes), nbytes, 1, who);
}

void Mem_Free(void *p)
{
	if (_ClearNote(p)) free(p);
}

