//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file Parameter.cpp
/// @brief This file keeps loaders for all possible types of Parameter

#include "ErrorHandler.h"

#include "PrmCompactionCoefficient.h"
#include "PrmCrustThinning.h"
#include "PrmEquilibriumOceanicLithosphereThickness.h"
#include "PrmInitialLithosphericMantleThickness.h"
#include "PrmLithoFraction.h"
#include "PrmLithoSTPThermalCond.h"
#include "PrmOneCrustThinningEvent.h"
#include "PrmPermeabilityModel.h"
#include "PrmPorosityModel.h"
#include "PrmSourceRockHC.h"
#include "PrmSourceRockHI.h"
#include "PrmSourceRockPreAsphaltStartAct.h"
#include "PrmSourceRockTOC.h"
#include "PrmSourceRockType.h"
#include "PrmSurfacePorosity.h"
#include "PrmSurfaceTemperature.h"
#include "PrmTopCrustHeatProduction.h"
#include "PrmTopCrustHeatProductionGrid.h"
#include "PrmTopCrustHeatProductionGridScaling.h"
#include "PrmWindow.h"

using namespace std;

namespace casa
{

const VarParameter* casa::Parameter::parent() const
{
  return m_parent;
}

void Parameter::setParent(const casa::VarParameter* varPrm)
{
  m_parent = varPrm;
}

std::vector<string> Parameter::parameters() const
{
  return std::vector<std::string>();
}

Parameter * casa::Parameter::load( CasaDeserializer & dz, const char * objName )
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
   else if ( ot == "PrmTopCrustHeatProductionGrid"   ) { return new PrmTopCrustHeatProductionGrid(   dz, vr ); }
   else if ( ot == "PrmPorosityModel"                ) { return new PrmPorosityModel(                dz, vr ); }
   else if ( ot == "PrmSurfacePorosity"              ) { return new PrmSurfacePorosity(              dz, vr ); }
   else if ( ot == "PrmPermeabilityModel"            ) { return new PrmPermeabilityModel(            dz, vr ); }
   else if ( ot == "PrmLithoSTPThermalCond"          ) { return new PrmLithoSTPThermalCond(          dz, vr ); }
   else if ( ot == "PrmWindow"                       ) { return new PrmWindow(                       dz, vr ); }
   else if ( ot == "PrmLithoFraction"                ) { return new PrmLithoFraction(                dz, vr ); }
   else if ( ot == "PrmCompactionCoefficient"        ) { return new PrmCompactionCoefficient(        dz, vr ); }
   else if ( ot == "PrmSurfaceTemperature"           ) { return new PrmSurfaceTemperature(           dz, vr ); }
   else if ( ot == "PrmInitialLithosphericMantleThickness") { return new PrmInitialLithosphericMantleThickness(dz, vr); }
   else if ( ot == "PrmTopCrustHeatProductionGridScaling" ) { return new PrmTopCrustHeatProductionGridScaling( dz, vr); }
   else if ( ot == "PrmEquilibriumOceanicLithosphereThickness" ) { return new PrmEquilibriumOceanicLithosphereThickness( dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Parameter deserialization error: Unknown type: " << ot;
   }
   return 0;
}

bool Parameter::saveCommonPart(casa::CasaSerializer& sz) const
{
  const bool hasParent = m_parent;
  bool ok = sz.save( hasParent, "hasParent" );

  if ( hasParent )
  {
     CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
     ok = ok && sz.save( parentID, "VarParameterID" );
  }

  return ok;
}

Parameter::Parameter(const casa::VarParameter * parent) :
  m_parent(parent)
{
}

Parameter::Parameter(casa::CasaDeserializer& dz, unsigned int /*version*/)
{
  CasaDeserializer::ObjRefID parentID;

  bool hasParent;
  bool ok = dz.load( hasParent, "hasParent" );

  if ( hasParent )
  {
     bool ok = dz.load( parentID, "VarParameterID" );
     m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
  }

  if ( !ok )
  {
     throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
        << "Parameter deserialization unknown error";
  }
}

}
