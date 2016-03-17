//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Parameter.C
/// @brief This file keeps loaders for all possible types of Parameter

#include "ErrorHandler.h"

#include "PrmOneCrustThinningEvent.h"
#include "PrmCrustThinning.h"
#include "PrmSourceRockTOC.h"
#include "PrmSourceRockHC.h"
#include "PrmSourceRockHI.h"
#include "PrmSourceRockType.h"
#include "PrmSourceRockPreAsphaltStartAct.h"
#include "PrmTopCrustHeatProduction.h"
#include "PrmPorosityModel.h"
#include "PrmSurfacePorosity.h"
#include "PrmPermeabilityModel.h"
#include "PrmLithoSTPThermalCond.h"
#include "PrmWindow.h"
#include "PrmLithoFraction.h"

casa::Parameter * casa::Parameter::load( CasaDeserializer & dz, const char * objName )
{
   std::string  ot; // object type name
   std::string  on; // object name
   unsigned int vr; // object version

   dz.loadObjectDescription( ot, on, vr );
   if ( on.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Parameter deserialization error, expected parameter with name: " << objName
         << ", but stream gave object with name: " << on;
   }
   if (      ot == "PrmOneCrustThinningEvent"        ) { return new PrmOneCrustThinningEvent(        dz, vr ); }
   else if ( ot == "PrmCrustThinning"                ) { return new PrmCrustThinning(                dz, vr ); }
   else if ( ot == "PrmSourceRockTOC"                ) { return new PrmSourceRockTOC(                dz, vr ); }
   else if ( ot == "PrmSourceRockHC"                 ) { return new PrmSourceRockHC(                 dz, vr ); }
   else if ( ot == "PrmSourceRockHI"                 ) { return new PrmSourceRockHI(                 dz, vr ); }
   else if ( ot == "PrmSourceRockPreAsphaltStartAct" ) { return new PrmSourceRockPreAsphaltStartAct( dz, vr ); }
   else if ( ot == "PrmSourceRockType"               ) { return new PrmSourceRockType(               dz, vr ); }
   else if ( ot == "PrmTopCrustHeatProduction"       ) { return new PrmTopCrustHeatProduction(       dz, vr ); }
   else if ( ot == "PrmPorosityModel"                ) { return new PrmPorosityModel(                dz, vr ); }
   else if ( ot == "PrmSurfacePorosity"              ) { return new PrmSurfacePorosity(              dz, vr ); }
   else if ( ot == "PrmPermeabilityModel"            ) { return new PrmPermeabilityModel(            dz, vr ); }
   else if ( ot == "PrmLithoSTPThermalCond"          ) { return new PrmLithoSTPThermalCond(          dz, vr ); }
   else if ( ot == "PrmWindow"                       ) { return new PrmWindow(                       dz, vr ); }
   else if ( ot == "PrmLithoFraction               " ) { return new PrmWindow(                       dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Parameter deserialization error: Unknown type: " << ot;
   }
   return 0;
}
