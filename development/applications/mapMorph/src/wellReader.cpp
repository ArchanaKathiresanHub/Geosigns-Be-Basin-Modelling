//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellReader.h"

#include "FilePath.h"

#include <fstream>
#include <sstream>
#include <string>

namespace WellReader
{

bool readWells( const char* wellFile, std::vector<double>& xWells, std::vector<double>& yWells )
{
  if ( !wellFile || !ibs::FilePath( std::string(wellFile) ).exists() )
  {
    return false;
  }

  std::ifstream infile;
  infile.open( wellFile, std::ios::in );
  if ( infile.fail() )
  {
    return false;
  }

  std::string line;
  while ( true )
  {
    std::getline( infile, line, '\n' );
    if ( infile.eof() ) { break; }

    std::stringstream stream(line);
    double x, y;
    stream >> x >> y;
    xWells.push_back(x);
    yWells.push_back(y);
  }
  infile.close();

  return true;
}

}
