//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// CASA API
#include "SimpleTxtDeserializer.h"
#include "SimpleBinDeserializer.h"

// CMB API
#include "ErrorHandler.h"

// STL/C lib
#include <cstdio>
#include <cstring>

namespace casa
{

   CasaDeserializer * CasaDeserializer::createDeserializer( std::istream & fid, const std::string & fileFormat, unsigned int ver )
   {
      // if no format is given - try to find file type by reading signature
      if ( fileFormat.empty() )
      {
         if (      SimpleTxtDeserializer::checkSignature( fid ) ) { return new SimpleTxtDeserializer( fid, ver ); }
         else if ( SimpleBinDeserializer::checkSignature( fid ) ) { return new SimpleBinDeserializer( fid, ver ); }
      }
      else if (    fileFormat == "txt"                          ) { return new SimpleTxtDeserializer( fid, ver ); }
      else if (    fileFormat == "bin"                          ) { return new SimpleBinDeserializer( fid, ver ); }
         
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown type of input file for loading ScenarioAnalysis object: " << 
                                                                      fileFormat;
   }
}

