//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmCategorical.C
/// @brief This file keeps loaders for all possible types of VarPrmCategorical

#include "VarPrmCategorical.h"

namespace casa
{
   VarPrmCategorical * VarPrmCategorical::load( CasaDeserializer & dz, const char * objName )
   {
      std::string  ot; // object type name
      std::string  on; // object name
      unsigned int vr; // object version

      dz.loadObjectDescription( ot, on, vr );
      if ( on.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "VarPrmCategorical deserialization error, expected VarPrmCategorical with name: " << objName
            << ", but stream gave object with name: " << on;
      }
      // Here should be switch over possible categorical parameters
/*      if ( ot == typeid( casa::CatPrmType1).name() ) { return 0; }
      else if ( ot == typeid( casa::CatPrmType1).name() ) { return 0; }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "VarPrmCategorical deserialization error: Unknown type: " << ot;
      }
      */
      return 0;
   }
}