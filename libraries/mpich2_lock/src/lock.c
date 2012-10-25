/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* 
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */


extern int gethostname(char *name, int len);
extern int system (const char * command);

#ifdef linux
extern int IBS_Use_ADIOI_Locking;
#endif

#include "adio.h"

#include <time.h>

static long SecondsAtStart = 0;
static long NanoSecondsAtStart = 0;

static void StartTime (void)
{
   struct timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);
   SecondsAtStart = tp.tv_sec;
   NanoSecondsAtStart = tp.tv_nsec;
}

static long GetElapsedTime (void)
{
   struct timespec tp;
   clock_gettime(CLOCK_REALTIME, &tp);
   return (tp.tv_sec - SecondsAtStart) * 1e9 + tp.tv_nsec - NanoSecondsAtStart;
}

#ifdef ROMIO_NTFS
/* This assumes that lock will always remain in the common directory and 
 * that the ntfs directory will always be called ad_ntfs. */
#include "..\ad_ntfs\ad_ntfs.h"
int ADIOI_Set_lock(FDTYPE fd, int cmd, int type, ADIO_Offset offset, int whence,
	     ADIO_Offset len) 
{
    static char myname[] = "ADIOI_Set_lock";
    int ret_val, error_code = MPI_SUCCESS;
    OVERLAPPED Overlapped;
    DWORD dwFlags;


    ADIOI_UNREFERENCED_ARG(whence);

    if (len == 0) return MPI_SUCCESS;

    dwFlags = type;

    Overlapped.hEvent = /*0;*/CreateEvent(NULL, TRUE, FALSE, NULL);
#ifdef HAVE_INT64
    Overlapped.Offset = ( (DWORD) ( offset & (__int64) 0xFFFFFFFF ) );
    Overlapped.OffsetHigh = ( (DWORD) ( (offset >> 32) & (__int64) 0xFFFFFFFF ) );

    if (cmd == ADIOI_LOCK_CMD)
    {
	/*printf("locking %d\n", (int)fd);fflush(stdout);*/
	ret_val = LockFileEx(fd, dwFlags, 0, 
	( (DWORD) ( len & (__int64) 0xFFFFFFFF ) ), 
	( (DWORD) ( (len >> 32) & (__int64) 0xFFFFFFFF ) ), 
	&Overlapped);
    }
    else
    {
	/*printf("unlocking %d\n", (int)fd);fflush(stdout);*/
	ret_val = UnlockFileEx(fd, 0, 
	( (DWORD) ( len & (__int64) 0xFFFFFFFF ) ), 
	( (DWORD) ( (len >> 32) & (__int64) 0xFFFFFFFF ) ), 
	&Overlapped);
    }
#else
    Overlapped.Offset = offset;
    Overlapped.OffsetHigh = 0;

    if (cmd == ADIOI_LOCK_CMD)
    {
	/*printf("locking %d\n", (int)fd);fflush(stdout);*/
	ret_val = LockFileEx(fd, dwFlags, 0, len, 0, &Overlapped);
    }
    else
    {
	/*printf("unlocking %d\n", (int)fd);fflush(stdout);*/
	ret_val = UnlockFileEx(fd, 0, len, 0, &Overlapped);
    }
#endif

    if (!ret_val)
    {
    char errMsg[ADIOI_NTFS_ERR_MSG_MAX];
	/*
	FPRINTF(stderr, "File locking failed in ADIOI_Set_lock.\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
	*/
	ret_val = GetLastError();
	if (ret_val == ERROR_IO_PENDING)
	{
	    DWORD dummy;
	    ret_val = GetOverlappedResult(fd, &Overlapped, &dummy, TRUE);
	    if (ret_val)
	    {
		CloseHandle(Overlapped.hEvent);
		return MPI_SUCCESS;
	    }
	    ret_val = GetLastError();
	}
    ADIOI_NTFS_Strerror(ret_val, errMsg, ADIOI_NTFS_ERR_MSG_MAX);
	error_code = MPIO_Err_create_code(MPI_SUCCESS, MPIR_ERR_RECOVERABLE, myname, __LINE__,
	    MPI_ERR_IO, "**io", "**io %s", errMsg);
    }
    CloseHandle(Overlapped.hEvent);

    return error_code;
}
#else
int ADIOI_Set_lock(FDTYPE fd, int cmd, int type, ADIO_Offset offset, int whence,
	     ADIO_Offset len) 
{
   int err = 0, error_code, err_count = 0, sav_errno;
   int totalAttempts = 0;
   struct flock lock;

   static bool FirstCall = true;
   static bool AlwaysUseFileLocking = true;

   static int MyRank = -1;

   if (MyRank == -1)
      MPI_Comm_rank (MPI_COMM_WORLD, &MyRank);
   static int MySize = -1;

   if (MySize < 0)
      MPI_Comm_size (MPI_COMM_WORLD, &MySize);

   static bool ReportAboutFileLocking = false;

   if (FirstCall)
   {
      ReportAboutFileLocking = (getenv ("REPORT_FILE_LOCKING") != NULL);

      if (ReportAboutFileLocking)
      {
         if (MyRank == 0)
            FPRINTF (stderr, "** Using Modified MPICH2 ADIOI_Set_lock function instead of Intel MPI version\n");
      }
      if (ReportAboutFileLocking || AlwaysUseFileLocking)
      {
         AlwaysUseFileLocking = true;
         if (getenv ("REPORT_FILE_LOCKING") != NULL)
         {
            if (MyRank == 0)
               FPRINTF (stderr, "** File Locking is always on for multi-core jobs\n");
         }
      }

      FirstCall = false;
   }

   static bool IgnoreAllNextRequests = false; // true, if this node seems to be unable to perform fcntl locking
   static bool IgnoreNextRequest = false; // if true, this request is the unlock counterpart of a previously ignored lock request

   if (MySize == 1)
      return MPI_SUCCESS;       // only one process, no need for locking


   if (IgnoreNextRequest == true)
   {
      IgnoreNextRequest = false;
      return MPI_SUCCESS;
   }

   if (IgnoreAllNextRequests == true)
   {
      return MPI_SUCCESS;
   }

#if 1
   if (type == F_RDLCK)         // let's not do read locks
   {
      IgnoreNextRequest = true;
      return MPI_SUCCESS;
   }
#endif

   if (len == 0)
      return MPI_SUCCESS;

#ifndef IBS_USESETLKW
   if (cmd == F_SETLKW)
      cmd = F_SETLK;
#endif

#if 0
   /* if (type != F_UNLCK) */
   {
      fprintf (stderr, "%d: ADIOI_Set_lock(fd %d, cmd %s/%d, type %s/%d, whence %d, offset %d, len %d) %s\n",
               MyRank, fd,
               ((cmd == F_GETLK) ? "F_GETLK" :
                ((cmd == F_SETLK) ? "F_SETLK" :
                 ((cmd == F_SETLKW) ? "F_SETLKW" : "UNEXPECTED"))),
               cmd,
               ((type == F_RDLCK) ? "F_RDLCK" :
                ((type == F_WRLCK) ? "F_WRLCK" :
                 ((type == F_UNLCK) ? "F_UNLOCK" : "UNEXPECTED"))), type, whence, (int) offset, (int) len, IBS_Use_ADIOI_Locking ? "required" : "obsolete");
   }
#endif

#ifdef linux
   if (!AlwaysUseFileLocking && !IBS_Use_ADIOI_Locking)
      return MPI_SUCCESS;
#endif

   StartTime ();

   /* Depending on the compiler flags and options, struct flock 
      may not be defined with types that are the same size as
      ADIO_Offsets.  */

/* FIXME: This is a temporary hack until we use flock64 where
   available. It also doesn't fix the broken Solaris header sys/types.h
   header file, which declars off_t as a UNION ! Configure tests to
   see if the off64_t is a union if large file support is requested; 
   if so, it does not select large file support.
*/
#ifdef NEEDS_INT_CAST_WITH_FLOCK
   lock.l_type = type;
   lock.l_start = (int) offset;
   lock.l_whence = whence;
   lock.l_len = (int) len;
#else
   lock.l_type = type;
   lock.l_whence = whence;
   lock.l_start = offset;
   lock.l_len = len;
#endif

   sav_errno = errno;           /* save previous errno in case we recover from retryable errors */
   errno = 0;
   int attempt = 0;
   static int maxAttempts = 5;

   for (attempt = 0; attempt < maxAttempts; ++attempt)
   {
      int maxTotalAccumulatedAttempts = (attempt + 1) * 1000;

      do
      {
         errno = 0;
         err = fcntl (fd, cmd, &lock);
         ++totalAttempts;
      }
      while (err /* && (errno == EACCES || errno == EAGAIN) */  && totalAttempts <= maxTotalAccumulatedAttempts);

      if (err && (errno != EBADF))
      {
         /* FIXME: This should use the error message system, 
            especially for MPICH2 */
         char hostname[256];

         gethostname (hostname, 255);
         FPRINTF (stderr,
                  "Rank %d (%s): File locking attempt %d failed in ADIOI_Set_lock(fd %X,cmd %s/%X,type %s/%X,whence %X) with return value %X and errno %X. %d more attempts remaining\n",
                  MyRank, hostname, attempt + 1, fd,
                  ((cmd == F_GETLK) ? "F_GETLK" : ((cmd == F_SETLK) ? "F_SETLK" : ((cmd == F_SETLKW) ? "F_SETLKW" : "UNEXPECTED"))), cmd,
                  ((type == F_RDLCK) ? "F_RDLCK" : ((type == F_WRLCK) ? "F_WRLCK" : ((type == F_UNLCK) ? "F_UNLOCK" : "UNEXPECTED"))), type, whence, err, errno,
                  maxAttempts - (attempt + 1));
         perror ("ADIOI_Set_lock:");
      }
      else
      {
         break;
      }
   }
   if (attempt >= maxAttempts)
   {
      char hostname[256];
      char command[1024];

      gethostname (hostname, 255);

      FPRINTF (stderr, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
      FPRINTF (stderr, "Rank %d (%s): File locking failed on host %s\n", MyRank, hostname, hostname);
      FPRINTF (stderr, "Rank %d (%s): Please report this host (%s) to BPA Support (bpasupport@shell.com)\n", MyRank, hostname, hostname);
      FPRINTF (stderr, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
      FPRINTF (stderr, "Rank %d (%s): Attempting to continue without locking ....\n", MyRank, hostname);

      sprintf (command, "echo \"ADIOI_Set_lock: (NFS) file locking failed on cluster node %s, please report to HPC Support\" | "
               "mail -s \"Error on cluster node %s\" -c alfred.vanderhoeven@shell.com,fain.street@shell.com bpasupport@shell.com", hostname, hostname);

      system (command);

      IgnoreAllNextRequests = true;
      return MPI_SUCCESS;

      FPRINTF (stderr, "Rank %d: File locking failed in ADIOI_Set_lock(fd %X,cmd %s/%X,type %s/%X,whence %X) with return value %X and errno %X.\n"
               "- If the file system is NFS, you need to use NFS version 3, ensure that the lockd daemon is running on all the machines, and mount the directory with the 'noac' option (no attribute caching).\n"
               "- If the file system is LUSTRE, ensure that the directory is mounted with the 'flock' option.\n",
               MyRank,
               fd,
               ((cmd == F_GETLK) ? "F_GETLK" :
                ((cmd == F_SETLK) ? "F_SETLK" :
                 ((cmd == F_SETLKW) ? "F_SETLKW" : "UNEXPECTED"))),
               cmd,
               ((type == F_RDLCK) ? "F_RDLCK" : ((type == F_WRLCK) ? "F_WRLCK" : ((type == F_UNLCK) ? "F_UNLOCK" : "UNEXPECTED"))), type, whence, err, errno);
      perror ("ADIOI_Set_lock:");
      FPRINTF (stderr, "ADIOI_Set_lock:offset %llu, length %llu\n", (unsigned long long) offset, (unsigned long long) len);
      MPI_Abort (MPI_COMM_WORLD, 1);
   }



#if 0
   if (!err)                    /* report fcntl failure errno's (EBADF), otherwise */
      errno = sav_errno;        /* restore previous errno in case we recovered from retryable errors */
#endif

   error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;

   long microseconds = GetElapsedTime () * 1e-3;

   if (ReportAboutFileLocking && (microseconds > 1e6 || totalAttempts > 1))
   {
      fprintf (stderr,
               "%d: ADIOI_Set_lock(fd %d, cmd %s/%d, type %s/%d, whence %d, offset %d, len %d), attempts: %d, errno: %d,  experiencing delay: %ld microseconds\n",
               MyRank, fd, ((cmd == F_GETLK) ? "F_GETLK" : ((cmd == F_SETLK) ? "F_SETLK" : ((cmd == F_SETLKW) ? "F_SETLKW" : "UNEXPECTED"))), cmd,
               ((type == F_RDLCK) ? "F_RDLCK" : ((type == F_WRLCK) ? "F_WRLCK" : ((type == F_UNLCK) ? "F_UNLOCK" : "UNEXPECTED"))), type, whence, (int) offset,
               (int) len, totalAttempts, errno, microseconds);
   }
   return error_code;
}
#endif

#if (defined(ROMIO_HFS) || defined(ROMIO_XFS))
int ADIOI_Set_lock64(FDTYPE fd, int cmd, int type, ADIO_Offset offset,
                     int whence,
	             ADIO_Offset len) 
{
    int err, error_code;
    struct flock64 lock;

    if (len == 0) return MPI_SUCCESS;

    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    do {
	err = fcntl(fd, cmd, &lock);
    } while (err && (errno == EINTR));

    if (err && (errno != EBADF)) {
	FPRINTF(stderr, "File locking failed in ADIOI_Set_lock64(fd %X,cmd %s/%X,type %s/%X,whence %X) with return value %X and errno %X.\n"
                  "If the file system is NFS, you need to use NFS version 3, ensure that the lockd daemon is running on all the machines, and mount the directory with the 'noac' option (no attribute caching).\n",
          fd,
          ((cmd == F_GETLK   )? "F_GETLK" :
          ((cmd == F_SETLK   )? "F_SETLK" :
          ((cmd == F_SETLKW  )? "F_SETLKW" :
          ((cmd == F_GETLK64 )? "F_GETLK64" :
          ((cmd == F_SETLK64 )? "F_SETLK64" :
          ((cmd == F_SETLKW64)? "F_SETLKW64" : "UNEXPECTED")))))),
          cmd, 
          ((type == F_RDLCK   )? "F_RDLCK" :
          ((type == F_WRLCK   )? "F_WRLCK" :
          ((type == F_UNLCK   )? "F_UNLOCK" : "UNEXPECTED"))),
          type, 
          whence, err, errno);
  perror("ADIOI_Set_lock64:");
  FPRINTF(stderr,"ADIOI_Set_lock:offset %llu, length %llu\n",(unsigned long long)offset, (unsigned long long)len);
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    error_code = (err == 0) ? MPI_SUCCESS : MPI_ERR_UNKNOWN;
    return error_code;
}
#endif
