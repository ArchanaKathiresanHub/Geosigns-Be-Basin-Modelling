//
// Wrapper class for UNIX commands
// opendir, closedir, readdir
//

#ifndef __directoryinfo__
#define __directoryinfo__

#include <vector>
#include <string>
using namespace std;

class DirectoryInfo
{
public:
   typedef vector<string> DirVector;
   typedef DirVector::iterator DirVectorIt;
   
   // public functions
   static void getAllExtension (const char *dirName, const char *ext, DirVector& l); 
   static bool endsWith        (const char *filename, const char *ext);

   static void getAllThatContains (const char *searchCommand, 
                                   DirVector& fileList);

private:
   static string createPathName (const char *dirName);

   static const char *TempFileName;
};

#endif
