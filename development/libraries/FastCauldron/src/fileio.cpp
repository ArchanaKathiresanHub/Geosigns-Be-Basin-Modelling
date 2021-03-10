//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include"fileio.h"

#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>  

#include "utils.h"
#include "ConstantsFastcauldron.h"

string generatePropertyFileName ( const string& outputDirectory, 
                                  const bool    isMinorSnapshot, 
                                  const double  time ) {

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
