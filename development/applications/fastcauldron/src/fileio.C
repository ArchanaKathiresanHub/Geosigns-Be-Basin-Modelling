#include"fileio.h"

#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>  

#include "utils.h"
#include "globaldefs.h"

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