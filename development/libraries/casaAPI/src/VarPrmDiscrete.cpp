//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmDiscrete.cpp
/// @brief This file keeps loaders for all possible types of VarPrmDiscrete

#include "VarPrmDiscrete.h"

namespace casa
{
   VarPrmDiscrete * VarPrmDiscrete::load( CasaDeserializer & dz, const char * objName )
   {
      std::string  ot; // object type name
      std::string  on; // object name
      unsigned int vr; // object version

      dz.loadObjectDescription( ot, on, vr );
      if ( on.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "VarPrmDiscrete deserialization error, expected VarPrmDiscrete with name: " << objName
            << ", but stream gave object with name: " << on;
      }
      /*
      // Here should be switch over possible discrete parameters
      if (      ot == typeid( casa::CatPrm1 ).name() ) { return 0; }
      else if ( ot == typeid( casa::CatPrm2 ).name() ) { return 0; }
      else
      {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
      << "VarPrmDiscrete deserialization error: Unknown type: " << ot;
      }
      */
      return 0;
   }
}