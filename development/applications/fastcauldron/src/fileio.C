#include"fileio.h"

#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;


#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>  

#include "utils.h"
#include "globaldefs.h"


// void checkFileSystem(string& str) {
 
//   struct utsname buf;
//   uname(&buf);
//   if (0 == strcmp(buf.sysname, "Linux")) {
//   } else if (0 == strcmp(buf.sysname, "IRIX64")) {
//     //cout << "This is an IRIX64 operating system" << endl;
//   } else if (0 == strcmp(buf.sysname, "SunOS")) {
//     //cout << "This is a Sun operating system" << endl;
//   } else {
//     cout << buf.sysname << " " << buf.nodename << " ";
//     cout << buf.release << " " << buf.version << " " << buf.machine << endl;
//   }

// }

string generatePropertyFileName ( const string& outputDirectory, 
                                  const bool    isMinorSnapshot, 
                                  const double  time ) {

  const int size = 150;
  char buffer[size];

  stringstream fileNameStr;//(buffer, size);

  if ( isMinorSnapshot ) {
    fileNameStr << MinorSnapshotFileName << "_" << time << ".h5" << '\0';
  } else {
    fileNameStr << MajorSnapshotFileName << "_" << time << ".h5" << '\0';
  }

  string fileName = outputDirectory;
  fileName += fileNameStr.str();
  return fileName;

}

string generatePropertyGridName(string propertyName, string surfaceName,
				string formationName, const double time) {

  const int size = 150;
  char buffer[size];

  stringstream gridNameStr; //(buffer, size);

  underscoreBlankSpace(surfaceName);
  underscoreBlankSpace(formationName);
  underscoreSlashCharacter(surfaceName);
  underscoreSlashCharacter(formationName);

  gridNameStr << propertyName << "_" << time << "_" 
	      << surfaceName <<  "_" << formationName << '\0';

  string gridName;
  gridName += gridNameStr.str();
  return gridName;

}
