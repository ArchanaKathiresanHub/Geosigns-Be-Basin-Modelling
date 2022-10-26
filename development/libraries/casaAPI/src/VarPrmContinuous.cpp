//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarPrmContinuous.cpp
/// @brief This file keeps API definition for handling continuous parameters.

#include "VarPrmContinuous.h"

#include "VarPrmCrustThinning.h"
#include "VarPrmOneCrustThinningEvent.h"
#include "VarPrmSourceRockTOC.h"
#include "VarPrmSourceRockHC.h"
#include "VarPrmSourceRockHI.h"
#include "VarPrmSourceRockPreAsphaltStartAct.h"
#include "VarPrmPorosityModel.h"
#include "VarPrmSurfacePorosity.h"
#include "VarPrmPermeabilityModel.h"
#include "VarPrmLithoFraction.h"
#include "VarPrmCompactionCoefficient.h"

#include "PrmEquilibriumOceanicLithosphereThickness.h"
#include "PrmInitialLithosphericMantleThickness.h"
#include "PrmLithoSTPThermalCond.h"
#include "PrmSurfaceTemperature.h"
#include "PrmTopCrustHeatProduction.h"
#include "PrmTopCrustHeatProductionGrid.h"
#include "PrmTopCrustHeatProductionGridScaling.h"
#include "VarPrmContinuousTemplate.h"
#include "UndefinedValues.h"

#include "VarPrmCategorical.h"

#include "MapInterpolator.h"

#include <cmath>

namespace casa
{
std::vector<double> VarPrmContinuous::stdDevs() const
{
   const std::vector<double> minVals  = m_minValue->asDoubleArray();
   const std::vector<double> maxVals  = m_maxValue->asDoubleArray();
   const std::vector<double> baseVals = m_baseValue->asDoubleArray();

   std::vector<double> devs( minVals.size(), 0 );
   for ( size_t i = 0; i < devs.size(); ++i )
   {
      double mi = minVals[i];
      double ma = maxVals[i];
      double to = baseVals[i];

      switch ( m_pdf )
      {
      case Block:    devs[i] = 0.5 * (ma - mi) / sqrt( 3.0 );                                              break;
      case Triangle: devs[i] = sqrt( (mi * mi + ma * ma + to * to - mi * ma - mi * to - ma * to) / 18.0 ); break;
      case Normal:   devs[i] = 0.5 * (ma - mi) / 5.0;                                                      break;
      default:       assert( false );                                                                          break;
      }
   } return devs;
}

SharedParameterPtr VarPrmContinuous::makeThreeDFromOneD( mbapi::Model& mdl,
                                                         const std::vector<double>& xin,
                                                         const std::vector<double>& yin,
                                                         const std::vector<SharedParameterPtr>& prmVec,
                                                         const InterpolationParams& interpolationParams,
                                                         const MapInterpolator& interpolator) const
{
   const TableInfo tableInfo = prmVec[0]->tableInfo();

   std::vector<double> prmValueVec;
   for ( unsigned int i = 0; i != prmVec.size(); ++i )
   {
      const std::vector<double> parameters = prmVec[i]->asDoubleArray();
      if ( parameters.size() != 1 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "3D from 1D only supported for scalar continuous parameters.";
      }
      prmValueVec.push_back( parameters[0] );
   }

   std::vector<double> xOut;
   std::vector<double> yOut;
   std::vector<double> vOut;

   DomainData domainData;
   mdl.highResOrigin(domainData.xmin, domainData.ymin);
   mdl.highResCellSize(domainData.deltaX, domainData.deltaY);
   mdl.highResGridArealSize(domainData.numI, domainData.numJ);

   interpolator.generateInterpolatedMap(domainData, xin, yin, prmValueVec, xOut, yOut, vOut);

   if ( ErrorHandler::NoError != mdl.smoothenVector( vOut, interpolationParams.smoothingMethod, interpolationParams.radius, interpolationParams.nrOfThreads ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "The smoothing of " << prmVec[0]->typeName() << " failed.";
   }

   // get the maps manager
   mbapi::MapsManager & mapsMgr = mdl.mapsManager();

   // Correct for the region with valid data if there are grid maps
   std::vector<double> depths;
   mdl.getGridMapDepthValues(0, depths);
   if (depths.size() == vOut.size())
   {
      for ( unsigned int i = 0; i < depths.size(); ++i )
      {
         if ( Utilities::isValueUndefined(depths[i]) )
         {
            vOut[i] = Utilities::Numerical::CauldronNoDataValue;
         }
      }
   }

   // generate the maps
   const std::string mapName = "Interpolated_" + tableInfo.variableGridName + "_Map";
   size_t mapSeqNbr = Utilities::Numerical::NoDataIDValue;

   mbapi::MapsManager::MapID id = mapsMgr.generateMap( tableInfo.tableName, mapName, vOut, mapSeqNbr,"" ); // use the default filename for the file storing the maps
   if ( Utilities::isValueUndefined( id ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Generation of the " << mapName
                                                                     << " map failed";
   }

   // Remove the map that is going to be replaced
   const std::string replacedMap = mdl.tableValueAsString(tableInfo.tableName, tableInfo.tableRow, tableInfo.variableGridName);
   if (replacedMap != "")
   {
      mdl.mapsManager().removeMapReferenceFromGridMapIOTbl(replacedMap, tableInfo.tableName);
   }

   // Set the new map into the table
   mdl.setTableValue(tableInfo.tableName, tableInfo.tableRow, tableInfo.variableGridName, mapName);

   // Since we create a 3d version of the parameter (makeThreeDFromOneD) we produce the grid version of the parameter
   return prmVec[0]->createNewGridVersionOfParameterFromModel(mdl);
}

std::vector<bool> VarPrmContinuous::selected() const
{
   std::vector<bool> mask;

   const std::vector<double> & minVals = m_minValue->asDoubleArray();
   const std::vector<double> & maxVals = m_maxValue->asDoubleArray();

   assert( minVals.size() == maxVals.size() );

   for ( size_t i = 0; i < minVals.size(); ++i )
   {
      // check relative difference
      mask.push_back( ( std::fabs(maxVals[i] - minVals[i]) <= 1.e-6 * std::fabs(maxVals[i] + minVals[i]) ) ? false : true );
   }

   return mask;
}

bool VarPrmContinuous::save( CasaSerializer & sz ) const
{
   // register var. parameter with serializer to allow all Parameters objects keep reference after deserializtion
   CasaSerializer::ObjRefID obID = sz.ptr2id( this );
   bool ok = sz.save( obID, "ID" );

   ok = ok ? sz.save( m_pdf, "prmPDF" ) : ok;

   ok = ok ? sz.save( *(m_baseValue.get()), "baseValue" ) : ok;
   ok = ok ? sz.save( *(m_minValue.get()),  "minValue" )  : ok;
   ok = ok ? sz.save( *(m_maxValue.get()),  "maxValue" )  : ok;

   ok = ok ? sz.save( m_name, "userGivenName" ) : ok;

   std::vector<CasaSerializer::ObjRefID> vecToSave( m_dependsOn.begin(), m_dependsOn.end() );
   ok = ok ? sz.save( vecToSave, "connectedTo" ) : ok;

   return ok;
}

VarPrmContinuous * VarPrmContinuous::load( CasaDeserializer & dz, const char * objName )
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
   // create new variabale parameter object depending on object type name from file
   if (      ot == "VarPrmCrustThinning"                ) { return new VarPrmCrustThinning(                dz, vr ); }
   else if ( ot == "VarPrmOneCrustThinningEvent"        ) { return new VarPrmOneCrustThinningEvent(        dz, vr ); }
   else if ( ot == "VarPrmSourceRockTOC"                ) { return new VarPrmSourceRockTOC(                dz, vr ); }
   else if ( ot == "VarPrmSourceRockHC"                 ) { return new VarPrmSourceRockHC(                 dz, vr ); }
   else if ( ot == "VarPrmSourceRockHI"                 ) { return new VarPrmSourceRockHI(                 dz, vr ); }
   else if ( ot == "VarPrmSourceRockPreAsphaltStartAct" ) { return new VarPrmSourceRockPreAsphaltStartAct( dz, vr ); }
   else if ( ot == "VarPrmPorosityModel"                ) { return new VarPrmPorosityModel(                dz, vr ); }
   else if ( ot == "VarPrmSurfacePorosity"              ) { return new VarPrmSurfacePorosity(              dz, vr ); }
   else if ( ot == "VarPrmPermeabilityModel"            ) { return new VarPrmPermeabilityModel(            dz, vr ); }
   else if ( ot == "VarPrmLithoFraction"                ) { return new VarPrmLithoFraction(                dz, vr ); }
   else if ( ot == "VarPrmCompactionCoefficient"        ) { return new VarPrmCompactionCoefficient(        dz, vr ); }

   else if ( ot == "VarPrmEquilibriumOceanicLithosphereThickness") { return new VarPrmContinuousTemplate<PrmEquilibriumOceanicLithosphereThickness>( dz, vr ); }
   else if ( ot == "VarPrmInitialLithosphericMantleThickness" ) { return new VarPrmContinuousTemplate<PrmInitialLithosphericMantleThickness>( dz, vr ); }
   else if ( ot == "VarPrmLithoSTPThermalCond"                ) { return new VarPrmContinuousTemplate<PrmLithoSTPThermalCond>(                dz, vr ); }
   else if ( ot == "VarPrmSurfaceTemperature"                 ) { return new VarPrmContinuousTemplate<PrmSurfaceTemperature>(                 dz, vr ); }
   else if ( ot == "VarPrmTopCrustHeatProduction"             ) { return new VarPrmContinuousTemplate<PrmTopCrustHeatProduction>(             dz, vr ); }
   else if ( ot == "VarPrmTopCrustHeatProductionGrid"         ) { return new VarPrmContinuousTemplate<PrmTopCrustHeatProductionGrid>(         dz, vr ); }
   else if ( ot == "VarPrmTopCrustHeatProductionGridScaling"  ) { return new VarPrmContinuousTemplate<PrmTopCrustHeatProductionGridScaling>(  dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "VarPrmDiscrete deserialization error: Unknown type: " << ot;
   }

   return 0;
}

// Constructor from input stream, implements common part of deserialization for continuous influential parameters
bool VarPrmContinuous::deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
                                                                             "Version of VarPrmContinuous in file is newer. No forward compatibility!";
   }

   CasaDeserializer::ObjRefID obID;

   // load data necessary to create an object
   bool ok = dz.load( obID, "ID" );

   // register observable with deserializer under read ID to allow Parameters objects keep reference after deserialization
   if ( ok ) { dz.registerObjPtrUnderID( this, obID ); }

   int pdf;
   ok = ok ? dz.load( pdf, "prmPDF" ) : ok;
   m_pdf = static_cast<VarPrmContinuous::PDF>(pdf);

   if ( ok )
   {
      m_baseValue.reset( Parameter::load( dz, "baseValue" ) );
      m_minValue.reset(  Parameter::load( dz, "minValue"  ) );
      m_maxValue.reset(  Parameter::load( dz, "maxValue"  ) );
   }

   ok = ok ? dz.load( m_name, "userGivenName" ) : ok;

   // after restoring connections we do not need to keep connected ojbects ID any more, on
   // another serialization call m_dependsOn set will be updated
   std::vector<CasaDeserializer::ObjRefID> vecToLoad( m_dependsOn.begin(), m_dependsOn.end() );
   ok = ok ? dz.load( vecToLoad, "connectedTo" ) : ok;
   for ( size_t i = 0; ok && i < vecToLoad.size(); ++i )
   {
      const VarPrmCategorical * prm = dz.id2ptr<VarPrmCategorical>( vecToLoad[i] );
      if ( prm )
      {
         (const_cast<VarPrmCategorical*>(prm))->addDependent( this );
      }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "Can't restore connection for " <<
                                                                                " parameter: " << name()[0];
      }
   }
   return ok;
}
}
