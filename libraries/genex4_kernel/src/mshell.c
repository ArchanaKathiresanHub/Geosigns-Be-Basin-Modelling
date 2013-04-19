/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/mshell.c,v 25.0 2005/07/05 08:03:52 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: mshell.c,v $
 * Revision 25.0  2005/07/05 08:03:52  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:04  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:38  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:16  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:47  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:43  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:33  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:53  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:12  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:28  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:36  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:29  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:08  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:10  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:40  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:30  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:52  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:38  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 MShell.c - memory allocation debugging tool
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie)

 Date:        25-NOV-1991

 Description: MShell is a tool to help debug memory allocation. It replaces
              the usual memory allocation functions (malloc, calloc, realloc,
              strdup and free) by special functions. These functions check
              that memory allocations succeed and that only valid memory
              blocks are deallocated.
              In addition, MShell provides two functions to inquire the
              memory usage:
               - size_t MShellUsed (void)
                 MShellUsed returns the number of bytes of memory
                 currently consumed by memory allocated by the program.
               - void MShellDisplay (FILE *)
                 MShellDisplay writes memory allocation information to the
                 file passed in the argument list. The information consists
                 of the allocated blocks of memory with their size and the
                 lines in the program where they were allocated.

 Usage notes: In order to use MShell the file mshell.h has to be included
              in the program.

 Example:     #include  <stdio.h>
                                  #include  <stdlib.h>
                                  #include  "mshell.h"
                                  main ()
                                  {
                                         void  *p;
                                         p = malloc (500);
                                         if (MShellUsed() != 0) MShellDisplay (stdout);
                                  }

 History
 -------
 25-NOV-1991  P. Alphenaar  initial version

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

/* Macro definitions */
/* ----------------- */
/* -- constants -- */
#define ALIGNSIZE          sizeof (double)
#define HEADERSIZE         sizeof (MemoryHeader)
#define HEADERTOMEMORY(h)  ((void *) (((char *) (h)) + RESERVESIZE))
#define MEMORYTAG          0xA55A
#define MEMORYTOHEADER(m)  ((MemoryHeader *) (((char *) (m)) - RESERVESIZE))
#define RESERVESIZE        (((HEADERSIZE+(ALIGNSIZE-1))/ALIGNSIZE)*ALIGNSIZE)

/* Type definitions */
/* ---------------- */
/* -- structure definitions -- */
typedef struct MemoryHeader  MemoryHeader;
struct  MemoryHeader
{
   unsigned int  tag;
   size_t        size;
   MemoryHeader  *next, *prev;
   const char    *file;
   int           line;
};

/* Function prototypes */
/* ------------------- */
/* -- exported functions -- */
void         *MShellCalloc (size_t, size_t, const char *, int);
void         MShellDisplay (FILE *);
void         MShellFree (const void *, const char *, int);
void         *MShellMalloc (size_t, const char *, int);
void         *MShellRealloc (const void *, size_t, const char *, int);
char         *MShellStrdup (const char *, const char *, int);
size_t       MShellUsed (void);
/* -- local functions -- */
static void  MShellAddHeader (MemoryHeader *, size_t, const char *, int);
static void  MShellError (const char *, const char *, int);
static void  MShellRemoveHeader (MemoryHeader *);

/* Global variables */
/* ---------------- */
/* -- file global variables -- */
static MemoryHeader  *MemoryBlockList=NULL;



/*
--------------------------------------------------------------------------------
 MShellAddHeader - fills and adds an allocated block of memory to the list
--------------------------------------------------------------------------------

 MShellAddHeader fills the header of the block of memory that MShell has
 allocated with information and adds it to the double linked list of memory
 blocks.

 Arguments
 ---------
 header - header of the memory block to be filled and added to the list
 size   - the size of the memory requested by the application
 file   - file in which memory allocation is requested
 line   - line in file where memory allocation is requested

--------------------------------------------------------------------------------
*/

static void MShellAddHeader (MemoryHeader *header, size_t size,
                             const char *file, int line)
{
   assert (header);
   assert (size>0);
   assert (file);
   assert (line>0);

/* Fill in header information */
   header->tag  = MEMORYTAG;
   header->size = size;
   header->file = file;
   header->line = line;

/* Add header to the list */
   header->next = MemoryBlockList;
   header->prev = NULL;
   if (MemoryBlockList) MemoryBlockList->prev = header;
   MemoryBlockList = header;
}



/*
--------------------------------------------------------------------------------
 MShellCalloc - allocate a cleared block of memory and add it to the list
--------------------------------------------------------------------------------

 MShellCalloc allocates the memory for a number of <num> elements, each with
 a size of <size> bytes. The memory allocated is initialized with 0. A number
 of extra bytes are allocated to be used as a header. This header is filled
 with information concerning the memory block and added to the list of
 allocated memory blocks. If memory allocation fails an error message is
 written to stderr and a NULL pointer is returned.

 Arguments
 ---------
 num  - the number of elements to be allocated
 size - the size of the memory required for each of the elements
 file - file in which MShellCalloc is called
 line - line in file where MShellCalloc is called

 Return value
 ------------
 pointer to block of memory of requested size or NULL in case of error

--------------------------------------------------------------------------------
*/

void *MShellCalloc (size_t num, size_t size, const char *file, int line)
{
   const void    *memory=NULL;
   MemoryHeader  *header;

   assert (file);
   assert (line>0);

/* Check size of memory request */
   if ((num <= 0) || (size <= 0) || (num*size+RESERVESIZE < RESERVESIZE)) {
      MShellError ("incorrect memory size requested.", file, line);
   } else {

/* -- Allocate memory by calling standard C library malloc */
      header = ( MemoryHeader * ) calloc ((size_t) 1, num*size+RESERVESIZE);
      if (!header) {
         MShellError ("calloc returned NULL pointer.", file, line);
      } else {

/* ----- Add memory block to list */
         MShellAddHeader (header, num*size, file, line);
         memory = HEADERTOMEMORY (header);
      }
   }
   return ((void *) memory);
}



/*
--------------------------------------------------------------------------------
 MShellDisplay - displays the blocks of memory in use by the application
--------------------------------------------------------------------------------

 MShellDisplay displays information concerning the blocks of memory allocated
 by the application and not (yet) freed. The information is written to the
 file that is passed as an argument. This file should be open for writing.

 Arguments
 ---------
 file - file to which the information should be written

--------------------------------------------------------------------------------
*/

void MShellDisplay (FILE *file)
{
   MemoryHeader  *p, *q;
   size_t        used;

/* Check whether default for file argument is used */
   if (!file) file = stdout;

   fprintf (file, "\nSize      Allocated in line\n");
   fprintf (file, "--------------------------------------");
   fprintf (file, "--------------------------------------\n");

/* Loop through list of memory blocks, printing information */
   used = 0;
   for (p=MemoryBlockList; p; p=q) {
      q = p->next;
      fprintf (file, "%8u  %s (%d)", p->size, p->file, p->line);
      if (p->tag != MEMORYTAG) fprintf (file, " (INVALID)");
      fprintf (file, "\n");
      used += p->size;
   }

   fprintf (file, "--------\n");
   fprintf (file, "%8u  bytes total\n", used);
}



/*
--------------------------------------------------------------------------------
 MShellError - write a MShell error message
--------------------------------------------------------------------------------

 MShellError writes a MShell error message to stderr. The message consist of
 the text "MShell error: " and the text, file name and line number passed in
 the argument list.

 Arguments
 ---------
 text - the text of the error message
 file - file in which MShellMalloc is called
 line - line in file where MShellMalloc is called

--------------------------------------------------------------------------------
*/

static void MShellError (const char *text, const char *file, int line)
{
   assert (text);
   assert (file);
   assert (line>0);

   fprintf (stderr, "\nMShell: %s\n", text);
   fprintf (stderr, "        File %s -- line %d.\n", file, line);
}



/*
--------------------------------------------------------------------------------
 MShellFree - free a block of memory and remove it from the list
--------------------------------------------------------------------------------

 MShellFree frees a block of memory allocated by MShell. It checks whether
 the pointer passed points to a valid memory block. If so, the memory is
 freed and the memory block removed from the list.

 Arguments
 ---------
 memory - pointer to the memory to be freed
 file   - file in which MShellFree is called
 line   - line in file where MShellFree is called

--------------------------------------------------------------------------------
*/

void MShellFree (const void *memory, const char *file, int line)
{
   MemoryHeader  *header;

   assert (file);
   assert (line>0);

/* Check that memory pointer is present and correct */
   if (!memory) {
      MShellError ("NULL pointer freed.", file, line);
   } else {
      header = MEMORYTOHEADER (memory);
      if (header->tag != MEMORYTAG) {
         MShellError ("incorrect memory pointer.", file, line);
      } else {

/* ----- Remove block from list, invalidate tag and free memory */
         MShellRemoveHeader (header);
         header->tag = ~MEMORYTAG;
         free (header);
      }
   }
}



/*
--------------------------------------------------------------------------------
 MShellMalloc - allocate a block of memory and add it to the list
--------------------------------------------------------------------------------

 MShellMalloc allocates the specified amount of memory plus a few bytes extra
 to be used as a header. This header is filled with information concerning
 the memory block and added to the list of allocated memory blocks. If memory
 allocation fails an error message is written to stderr and a NULL pointer is
 returned.

 Arguments
 ---------
 size - the size of the memory requested by the application
 file - file in which MShellMalloc is called
 line - line in file where MShellMalloc is called

 Return value
 ------------
 pointer to block of memory of requested size or NULL in case of error

--------------------------------------------------------------------------------
*/

void *MShellMalloc (size_t size, const char *file, int line)
{
   const void    *memory=NULL;
   MemoryHeader  *header;

   assert (file);
   assert (line>0);

/* Check size of memory request */
   if ((size <= 0) || (size+RESERVESIZE < RESERVESIZE)) {
      MShellError ("incorrect memory size requested.", file, line);
   } else {

/* -- Allocate memory by calling standard C library malloc */
      header = ( MemoryHeader * ) malloc (size+RESERVESIZE);
      if (!header) {
         MShellError ("malloc returned NULL pointer.", file, line);
      } else {

/* ----- Add memory block to list */
         MShellAddHeader (header, size, file, line);
         memory = HEADERTOMEMORY (header);
      }
   }
   return ((void *) memory);
}



/*
--------------------------------------------------------------------------------
 MShellRealloc - re-allocate a block of memory and update it in the list
--------------------------------------------------------------------------------

 MShellRealloc re-allocates the specified amount of memory plus a few bytes
 extra to be used as a header. This header is filled with information
 concerning the memory block and added to the list of allocated memory blocks.
 If memory allocation fails an error message is written to stderr and a NULL
 pointer is returned.

 Arguments
 ---------
 oldmem - pointer to the memory to be reallocated
 size   - the size of the memory requested by the application
 file   - file in which MShellRealloc is called
 line   - line in file where MShellRealloc is called

 Return value
 ------------
 pointer to block of memory of requested size or NULL in case of error

--------------------------------------------------------------------------------
*/

void *MShellRealloc (const void *oldmem, size_t size, const char *file,
                     int line)
{
   const void    *memory=NULL;
   MemoryHeader  *header;

   assert (file);
   assert (line>0);

/* Check that memory pointer is present and correct */
   if (!oldmem) {
      MShellError ("NULL pointer reallocated.", file, line);
   } else {

/* -- Check size of memory request */
      if ((size <= 0) || (size+RESERVESIZE < RESERVESIZE)) {
         MShellError ("incorrect memory size requested.", file, line);
      } else {
         header = MEMORYTOHEADER (oldmem);
         if (header->tag != MEMORYTAG) {
            MShellError ("incorrect memory pointer.", file, line);
         } else {

/* -------- Remove block from list and invalidate tag */
            MShellRemoveHeader (header);
            header->tag = ~MEMORYTAG;

/* -------- Allocate memory by calling standard C library realloc */
            header = ( MemoryHeader * ) realloc (header, size+RESERVESIZE);
            if (!header) {
               MShellError ("realloc returned NULL pointer.", file, line);
            } else {

/* ----------- Add memory block to list */
               MShellAddHeader (header, size, file, line);
               memory = HEADERTOMEMORY (header);
            }
         }
      }
   }
   return ((void *) memory);
}



/*
--------------------------------------------------------------------------------
 MShellRemoveHeader - removes an allocated block of memory from the list
--------------------------------------------------------------------------------

 MShellRemoveHeader removes the block of memory that MShell has allocated
 from the double linked list of memory blocks.

 Arguments
 ---------
 header - header of the memory block to be removed to the list

--------------------------------------------------------------------------------
*/

static void MShellRemoveHeader (MemoryHeader *header)
{
   assert (header);
   assert (MemoryBlockList);

   if (header->next) header->next->prev = header->prev;
   if (header->prev) header->prev->next = header->next;
   if (header == MemoryBlockList) MemoryBlockList = header->next;
}



/*
--------------------------------------------------------------------------------
 MShellStrdup - duplicate a string by allocating it and add it to the list
--------------------------------------------------------------------------------

 MShellStrdup duplicates a string by allocating a block of memory for it. The
 string is copied to the new memory and the block of memory is added to the
 list of memory blocks. The allocating and adding to the list is done by
 calling the function MShellMalloc.

 Arguments
 ---------
 string - the string to be duplicated
 file   - file in which MShellStrdup is called
 line   - line in file where MShellStrdup is called

 Return value
 ------------
 pointer to new string or NULL in case of error

--------------------------------------------------------------------------------
*/

char *MShellStrdup (const char *string, const char *file, int line)
{
        char  *newstring=NULL;

        assert (file);
        assert (line>0);

/* Check that string pointer is present */
        if (!string) {
                MShellError ("string pointer is NULL.", file, line);
        } else {

/* -- Allocate memory by calling MShellMalloc */
                newstring = MShellMalloc (strlen (string) + 1, file, line);

/* -- Copy the string */
                if (newstring && !strcpy (newstring, string))
                        MShellError ("strcpy returned NULL pointer.", file, line);
        }
        return (newstring);
}



/*
--------------------------------------------------------------------------------
 MShellUsed - returns the amount of memory in use by the application
--------------------------------------------------------------------------------

 MShellUsed returns the number of bytes of memory that have been allocated
 by the application and not (yet) freed. This amount of memory is calculated
 by running through the list of memory blocks and adding their sizes.

 Return value
 ------------
 number of bytes allocated by the application

--------------------------------------------------------------------------------
*/

size_t MShellUsed (void)
{
   MemoryHeader  *p, *q;
   size_t        used;

   used = 0;
   for (p=MemoryBlockList; p; p=q) {
      q = p->next;
      used += p->size;
   }
   return (used);
}
