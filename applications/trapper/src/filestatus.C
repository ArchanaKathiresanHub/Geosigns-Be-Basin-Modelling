//
// Wrapper Class for UNIX command stat
// stat return status information on a file
//

#include "filestatus.h"
#include "time.h"

//
// constructors
//
FileStatus::FileStatus (const string &filename)
{
   if ( getStatInfo (filename.c_str()) )
   {
      fFilename = filename; 
   }
} 

FileStatus::FileStatus (const char *filename)
{
   if ( filename )
   {
      if ( getStatInfo (filename) )
      {
         fFilename = filename;
      }
      else
      {
   setError();
      }
   }
   else
   {
      setError (); 
   }
}

char* FileStatus::dateMod (void) const
{
    time_t mod_time (fStat.st_mtime);
    return ctime (&mod_time);
}

//
// Get Status Info
//
bool FileStatus::getStatInfo (const char *filename)
{
   fError = stat (filename, &fStat);
   
   return fError == 0;
}

void FileStatus::setError (void)
{
   // stat sets this to -1 if a file does not exist
   fError = -1;
}

/* TEST */
/*
int main (int argc, char **argv)
{
   FileStatus s (argv[1]);

   printf ("\nName        = %s"
           "\nFile Exists = %d"
    "\nError       = %d"
    "\nSize        = %d"
           "\nMode        = %d"
           "\nauthorWrite = %d"
           "\ngroup Write = %d"
           "\nauthorRead  = %d"
           "\ngroup read  = %d"
           "\nauthor Exec = %d"
           "\ngroup exec  = %d"
    "\nIs Dir      = %d\n",
    s.filename (),
    s.exists (),
    s.error (),
    s.fileSize (),
           s.mode (),
           s.authorWrite (),  
           s.groupWrite (),
           s.authorRead (),
           s.groupRead (),
           s.authorExec (),
           s.groupExec (),
    s.isDirectory ());
   
   return 0;
}
*/

