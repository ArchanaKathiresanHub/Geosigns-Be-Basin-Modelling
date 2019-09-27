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
#include "SimpleTxtSerializer.h"
#include "SimpleBinSerializer.h"

// CMB API
#include "ErrorHandler.h"

// C lib
#include <stdint.h>
#include <string.h>

namespace casa
{

   CasaSerializer * CasaSerializer::createSerializer( const std::string & fileName, const std::string & fileFormat, int ver )
   {
      if (      fileFormat == "txt" ) { return new SimpleTxtSerializer( fileName, ver ); }
      else if ( fileFormat == "bin" ) { return new SimpleBinSerializer( fileName, ver ); }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown type of output file for saving ScenarioAnalysis object: " << fileFormat;
      }
   }
}

