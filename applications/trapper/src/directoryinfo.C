//
// Wrapper class for UNIX commands
// opendir, closedir, readdir
//

#include "directoryinfo.h"
#include <dirent.h>
#include <fstream.h>
#include <iostream.h>

const char* DirectoryInfo::TempFileName = "DirectoryInfo_TempFile";

//
// PUBLIC METHODS
//

void DirectoryInfo::getAllExtension (const char *dirName, const char *ext, 
                                     DirectoryInfo::DirVector& l) 
{
   DIR *dir;

   // create path name
   string path = createPathName (dirName);
   const char *dirString = (strcmp (dirName, "") == 0) ? "./" : dirName;
   
   // look in directory
   if ( (dir = opendir (dirString)) != NULL )
   {
      for ( struct dirent *dirPtr = readdir (dir); dirPtr != NULL; dirPtr = readdir (dir) )
      {
         // if filename has correct extension or ending, save to vector
         if ( endsWith (dirPtr->d_name, ext) )
         {
            l.push_back (path + dirPtr->d_name);
         }
      }
   }
}

bool DirectoryInfo::endsWith (const char *name, const char *end) 
{
   if ( ! name || ! end ) return false;
   
   // issolate end of name
   int endLength = strlen (name) - strlen (end);
   const char *endsWith = (endLength > 0) ? name + endLength : ""; 
   
   // compare end of name to end
   return strcmp (endsWith, end) == 0;
}

//
// PRIVATE METHODS
//

string DirectoryInfo::createPathName (const char *dirName)
{
   string path (dirName);
   
   if ( path.size () > 0 )
      return (path[path.size()-1] == '/') ? path : path + "/";
   else
      return path;
}
  
void DirectoryInfo::getAllThatContains (const char *searchCommand, 
                                        DirVector& fileList)
{
   const int filenameLen = 1000;
   string createFileList = searchCommand;
   createFileList += " > ";
   createFileList += TempFileName;
   system (createFileList.c_str());

   ifstream inFile;
   inFile.open (TempFileName, ios::in);

   if ( inFile.rdbuf()->is_open () )
   {
      // read filename
      char filename[filenameLen ];

      while ( inFile.getline (filename, filenameLen , '\n') )
      {   
         // terminate string
         if ( inFile.gcount () < filenameLen  )
         { 
            filename[inFile.gcount()] = '\0';
         }

         // add to list
         fileList.push_back (filename);   
      }

      inFile.close ();
      char *rem_file = new char[strlen (TempFileName) + 5];
      sprintf(rem_file, "rm %s", TempFileName); 
      system (rem_file);
      delete [] rem_file;
   }
   else
   {
     cout << endl << "DirectoryInfo::getAllThatContains"
          << endl << "Error opening file " << TempFileName << endl;
   }
}



