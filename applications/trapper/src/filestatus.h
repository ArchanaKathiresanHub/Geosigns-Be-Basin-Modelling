//
// Wrapper Class for UNIX command stat
// stat return status information on a file
// 

#ifndef __filestatus__
#define __filestatus__

#include <sys/stat.h>
#include <sys/types.h>
#include <string>
using namespace std;

class FileStatus
{
public:
   typedef struct stat StatusStruct; 

   // ctor/dtor
   FileStatus (const string &filename);
   FileStatus (const char *filename);
   
   ~FileStatus () {}

   // return info functions
   const char* filename    (void) const { return fFilename.c_str(); }
   bool        error       (void) const { return fError != 0; }
   bool        exists      (void) const { return fError == 0; }
   off_t       fileSize    (void) const { return fStat.st_size; }
   mode_t      mode        (void) const { return fStat.st_mode; }
   bool        authorWrite (void) const { return fStat.st_mode & S_IWRITE; }
   bool        groupWrite  (void) const { return fStat.st_mode & S_IWGRP; }
   bool        authorRead  (void) const { return fStat.st_mode & S_IREAD; }
   bool        groupRead   (void) const { return fStat.st_mode & S_IRGRP; }
   bool        authorExec  (void) const { return fStat.st_mode & S_IEXEC; }
   bool        groupExec   (void) const { return fStat.st_mode & S_IXGRP; }
   bool        isDirectory (void) const { return S_ISDIR (fStat.st_mode); }
   char*      dateMod (void) const;

private:
   bool getStatInfo (const char *filename);
   void setError    (void);

   StatusStruct fStat;
   string       fFilename;
   int          fError;
};

#endif
