//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsTrapDerivedProp.cpp

// CASA API
#include "ObsTrapDerivedProp.h"

#include "ObsValueDoubleScalar.h"
#include "ObsValueDoubleArray.h"
#include "ObsValueTransformable.h"

// CMB API
#include "cmbAPI.h"
#include "UndefinedValues.h"

// Utilities lib
#include "ConstantsMathematics.h"
using Utilities::Maths::CelciusToKelvin;
using Utilities::Maths::MegaPaToPa;
#include "ConstantsPhysics.h"
using Utilities::Physics::StockTankPressureMPa;
using Utilities::Physics::StockTankTemperatureC;

#include "LogHandler.h"

// EosPack
#include "EosPack.h"

// CBMGenerics
#include "ComponentManager.h"

using namespace CBMGenerics;

// STL/C lib
#include <cassert>
#include <sstream>

static const char * g_PropCompSuffix       = "TrappedAmount";

static const double g_ZeroMassLogThreshold = -5.0;  // if ( log10( mass ) < ZeroMassLogThreshold ) - mass value is treated as zero
static const double g_ZeroMassThreshold    = pow( 10.0, g_ZeroMassThreshold );

// Some auxillary functions
static bool performPVT( double masses[ComponentManager::NUMBER_OF_SPECIES]
                      , double temperature
                      , double pressure
                      , double phaseMasses[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES]
                      , double phaseDensities[ComponentManager::NUMBER_OF_PHASES]
                      , double phaseViscosities[ComponentManager::NUMBER_OF_PHASES]
                      );

static double ComputeVolume( double * masses, double density, int numberOfSpecies );


namespace casa
{

// Create observable for the given trap property for specified areal position
ObsTrapDerivedProp::ObsTrapDerivedProp( double              x
                                      , double              y
                                      , const char        * resName
                                      , const char        * propName
                                      , double              simTime
                                      , bool                logTrans
                                      , const std::string & myName
                                      )
                                      : ObservableSinglePoint( myName, propName, simTime, x, y )
                                      , m_resName( resName )
                                      , m_logTransf( logTrans ) {}

// Destructor
ObsTrapDerivedProp::~ObsTrapDerivedProp() {}

size_t ObsTrapDerivedProp::dimensionUntransformed() const { return ComponentManager::NUMBER_OF_SPECIES + 4; }

// Convert composition and P/T to trap property
ObsValue * ObsTrapDerivedProp::transform( const ObsValue * val ) const
{
   const ObsValueDoubleArray * arrVal = dynamic_cast<const ObsValueDoubleArray*>( val );
   if ( !arrVal )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) <<
         "Wrong observable value type to make a transformation for TrapDerivedProp observable";
   }
   std::vector<double> values     = arrVal->asDoubleArray();
   bool                allDefined = true;

   for ( auto it : values ) { if ( IsValueUndefined( it ) ) { allDefined = false; break; } }

   double ret = Utilities::Numerical::IbsNoDataValue;
   if ( allDefined && values.size() == ComponentManager::NUMBER_OF_SPECIES + 4 )
   {
      // do back transform from log10
      for ( size_t i = 0; i < ComponentManager::NUMBER_OF_SPECIES + 2; ++i )
      {
         if ( m_logTransf )
         {
            values[i] = values[i] <= g_ZeroMassLogThreshold ? 0.0 : pow( 10, values[i] );
         }
         else
         {
            values[i] = values[i] < 0.0 ? 0.0 : values[i];
         }
      }
      ret = calculateDerivedTrapProp( values );
   }
   else if ( allDefined ) { assert( false ); } // shouldn't get here

   return ObsValueDoubleScalar::createNewInstance( this, ret );
}

// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsTrapDerivedProp::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot( simulationTime() ) ) return caldModel.errorCode();

   std::vector<std::string> propList;

   for ( int i = 0; i < ComponentManager::NUMBER_OF_SPECIES; ++i )
   {
      propList.push_back( ComponentManager::getInstance().getSpeciesName( i ) + g_PropCompSuffix );
   }
   propList.push_back( "MassLiquid" );
   propList.push_back( "MassVapour" );
   propList.push_back( "Pressure" );
   propList.push_back( "Temperature" );

   // add rows to the table
   size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );

   std::vector<int> posVec;
   for ( size_t i = 0; i < propList.size(); ++i )
   {  // insert new row into the table
      if ( ErrorHandler::NoError != caldModel.addRowToTable( Observable::s_dataMinerTable ) ) return caldModel.errorCode();

      size_t pos = static_cast<int>( tblSize ) + i;
      posVec.push_back( pos );

      // fill columns for the new row with values
      if ( !setCommonTableValues( caldModel, pos, xCoord(), yCoord(), zCoord(), "ReservoirName", m_resName ) )
      { return caldModel.errorCode(); }
   }

   setPosDataMiningTbl( posVec );

   return ErrorHandler::NoError;
}


// Get this observable value from Cauldron model
ObsValue * ObsTrapDerivedProp::getFromModel( mbapi::Model & caldModel )
{
   std::vector<double> val( posDataMiningTbl().size(), Utilities::Numerical::IbsNoDataValue );
   double eps = g_ZeroMassThreshold;

   if ( !checkObservableForProject( caldModel ) )
   {
     return ObsValueTransformable::createNewInstance( this, val );
   }

   size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );

   for ( size_t i = 0; i < posDataMiningTbl().size(); ++i )
   {
      std::string propName;
      if (      i <  ComponentManager::NUMBER_OF_SPECIES     ) { propName = ComponentManager::getInstance().getSpeciesName( static_cast<int>(i) ) + g_PropCompSuffix; }
      else if ( i == ComponentManager::NUMBER_OF_SPECIES     ) { propName = "MassLiquid";  }
      else if ( i == ComponentManager::NUMBER_OF_SPECIES + 1 ) { propName = "MassVapour";  }
      else if ( i == ComponentManager::NUMBER_OF_SPECIES + 2 ) { propName = "Pressure";    }
      else if ( i == ComponentManager::NUMBER_OF_SPECIES + 3 ) { propName = "Temperature"; }

      if ( IsValueUndefined( posDataMiningTbl()[i] ) )
      {
         std::vector<int> posVec = posDataMiningTbl();
         for ( size_t j = 0; j < tblSize; ++j )
         {
           if ( !checkObsMatchesModel( caldModel, j, xCoord(), yCoord(), zCoord(), eps, "ReservoirName", m_resName ) ) { continue; }

            val[i] = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "Value" );
            posVec[i] = j; // fill the rest of the table as well data must be continuous
            for ( size_t k = i+1; k < posVec.size(); ++k )
            {
               posVec[k] = posVec[k-1] + 1;
            }
            break;
         }
         setPosDataMiningTbl( posVec );
      }
      else
      {
         val[i] = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, posDataMiningTbl()[i], "Value" );
      }
   }

   for ( size_t i = 0; i < ComponentManager::NUMBER_OF_SPECIES + 2; ++i )
   {
      if ( m_logTransf )
      { // do log10 transform for masses
         val[i] = val[i] < eps ? g_ZeroMassLogThreshold : log10( val[i] );
      }
      else // if Undefinded value, this could be because trap not found. Set mass value to 0 in this case
      {
         val[i] = val[i] < eps ? 0.0 : val[i];
      }
   }
   return ObsValueTransformable::createNewInstance( this, val );
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsTrapDerivedProp::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   std::vector<double> values( val, val + ComponentManager::NUMBER_OF_SPECIES + 4 );
   val += ComponentManager::NUMBER_OF_SPECIES + 4;

   return ObsValueTransformable::createNewInstance( this, values );
}

bool ObsTrapDerivedProp::save( CasaSerializer & sz ) const
{
   bool ok = false;
   saveCommon( this, sz, ok, "reservoirName", m_resName );

   ok = ok ? sz.save( m_logTransf, "logTransfer" ) : ok;

   return ok;
}

ObsTrapDerivedProp::ObsTrapDerivedProp( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
         "Version of ObsTrapDerivedProp in file is newer. No forward compatibility!";
   }

   CasaDeserializer::ObjRefID obID;

   // load data necessary to create an object
   bool ok = dz.load( obID, "ID" );

   // register observable with deserializer under read ID to allow ObsValue objects keep reference after deserializtion
   dz.registerObjPtrUnderID( this, obID );

   double x = 0.0;
   double y = 0.0;
   ok = ok ? dz.load( x, "X" ) : ok;
   ok = ok ? dz.load( y, "Y" ) : ok;
   setCoords(x, y);

   ok = ok ? dz.load( m_resName,  "reservoirName" ) : ok;

   std::string propName;
   double simTime;
   std::vector<std::string> myName;
   ok = ok ? dz.load( propName, "propName" ) : ok;
   ok = ok ? dz.load( simTime,  "simTime"  ) : ok;
   ok = ok ? dz.load( myName,   "name"     ) : ok;
   setPropertyName( propName );
   setSimTime( simTime );
   setName( myName );

   std::vector<int> posVec;
   if ( objVer < 2 )
   {
      std::vector<int> pos;
      ok = ok ? dz.load( pos, "posDataMiningTbl" ) : ok;

      posVec.resize( pos.size() );
      for ( size_t i = 0; i < pos.size(); ++i )
      {
         posVec[i] = static_cast<size_t>( pos[i] < 0 ? Utilities::Numerical::NoDataIDValue : pos[i] );
      }
   }
   else
   {
      ok = ok ? dz.load( posVec, "posDataMiningTbl" ) : ok;
   }
   setPosDataMiningTbl( posVec );

   bool hasVal;
   ok = ok ? dz.load( hasVal, "HasRefValue" ) : ok;
   if ( hasVal ) { setRefValue( ObsValue::load( dz, "refValue" ) ); }

   ok = ok ? dz.load( hasVal, "HasDevVal" ) : ok;
   if ( hasVal ) { setDevValue( ObsValue::load( dz, "devValue" ) ); }

   double mySAWeight;
   double myUAWeight;
   ok = ok ? dz.load( mySAWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( myUAWeight, "uaWeight" ) : ok;
   setSAWeight( mySAWeight );
   setUAWeight( myUAWeight );

   if ( objVer > 0 )
   {
      ok = ok ? dz.load( m_logTransf, "logTransfer" ) : ok;
   }
   else { m_logTransf = true; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsTrapDerivedProp deserialization unknown error";
   }
}

double ObsTrapDerivedProp::calculateDerivedTrapProp( const std::vector<double> & vals ) const
{
   double ret = Utilities::Numerical::IbsNoDataValue;

   if ( vals.size() < ComponentManager::NUMBER_OF_SPECIES + 4 ) return ret;

   double phaseMasses[2];
   phaseMasses[ComponentManager::LIQUID] = vals[ComponentManager::NUMBER_OF_SPECIES    ];
   phaseMasses[ComponentManager::VAPOUR] = vals[ComponentManager::NUMBER_OF_SPECIES + 1];

   double P       = vals[ComponentManager::NUMBER_OF_SPECIES + 2];
   double T       = vals[ComponentManager::NUMBER_OF_SPECIES + 3];

   // check if it is possible to calculate for zero mass of HC
   if ( phaseMasses[ComponentManager::LIQUID] + phaseMasses[ComponentManager::VAPOUR]  < 1.0e-3 ) return ret;

   double masses[ComponentManager::NUMBER_OF_SPECIES];
   for ( int comp = 0; comp < ComponentManager::NUMBER_OF_SPECIES; ++comp ) { masses[comp] = vals[comp]; }

   // reservoir condition phases
   double massesRC[     ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES];
   double densitiesRC[  ComponentManager::NUMBER_OF_PHASES];
   double viscositiesRC[ComponentManager::NUMBER_OF_PHASES];

   // stock tank phases of reservoir condition phases
   double massesST[     ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES];
   double densitiesST[  ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES];
   double viscositiesST[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_PHASES];


   // perform PVT under reservoir conditions
   bool pvtRC = performPVT( masses, T, P, massesRC, densitiesRC, viscositiesRC );

   // perform PVT's of reservoir condition phases under stock tank conditions
   bool pvtRCVapour = performPVT( massesRC[ComponentManager::VAPOUR], StockTankTemperatureC, StockTankPressureMPa,
                                  massesST[ComponentManager::VAPOUR], densitiesST[ComponentManager::VAPOUR], viscositiesST[ComponentManager::VAPOUR]
                                );

   bool pvtRCLiquid = performPVT( massesRC[ComponentManager::LIQUID], StockTankTemperatureC, StockTankPressureMPa,
                                  massesST[ComponentManager::LIQUID], densitiesST[ComponentManager::LIQUID], viscositiesST[ComponentManager::LIQUID]
                                );

   // check for undefined values for Densities
   for ( int phase = 0; phase < ComponentManager::NUMBER_OF_PHASES; ++phase )
   {
      if ( densitiesRC[phase] == 1000 ) // some magical value from EoSPack if we do not have this phase
      {
         densitiesRC[phase] = 0.0;
         densitiesST[phase][ComponentManager::LIQUID] = 0.0;
         densitiesST[phase][ComponentManager::VAPOUR] = 0.0;

         viscositiesRC[phase] = Utilities::Numerical::IbsNoDataValue;
         viscositiesST[phase][ComponentManager::LIQUID] = Utilities::Numerical::IbsNoDataValue;
         viscositiesST[phase][ComponentManager::VAPOUR] = Utilities::Numerical::IbsNoDataValue;
      }
      else
      {
         for ( int stPhase = 0; stPhase < ComponentManager::NUMBER_OF_PHASES; ++stPhase )
         {
            if ( densitiesST[phase][stPhase] == 1000 )
            {
               densitiesST[phase][stPhase] = 0.0;
               viscositiesST[phase][stPhase] = Utilities::Numerical::IbsNoDataValue;
            }
         }
      }
   }

   bool stPhaseFound = false;

   ComponentManager::PhaseId rcPhase;
   ComponentManager::PhaseId stPhase;

   if ( propertyName().find( "FGIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::VAPOUR;
      stPhase = ComponentManager::VAPOUR;
   }
   else if (propertyName().find( "CIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::VAPOUR;
      stPhase = ComponentManager::LIQUID;
   }
   else if (propertyName().find( "SGIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::LIQUID;
      stPhase = ComponentManager::VAPOUR;
   }
   else if ( propertyName().find( "STOIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::LIQUID;
      stPhase = ComponentManager::LIQUID;
   }
   else if ( propertyName().find( "Vapour" ) != string::npos )
   {
      rcPhase = ComponentManager::VAPOUR;
   }
   else if ( propertyName().find( "Liquid" ) != string::npos )
   {
      rcPhase = ComponentManager::LIQUID;
   }

   // Volume, Density, Viscosity, and Mass properties for stock tank conditions
   if ( propertyName().find( "Density" ) != string::npos )
   {
      if ( stPhaseFound )
      {
         if ( (rcPhase == ComponentManager::VAPOUR && pvtRCVapour) || (rcPhase == ComponentManager::LIQUID && pvtRCLiquid) )
         {
            ret = densitiesST[rcPhase][stPhase];
         }
      }
      else if ( pvtRC && phaseMasses[rcPhase] > 0.0 ) { ret = densitiesRC[rcPhase]; }
   }
   else if ( propertyName().find( "Viscosity" ) != string::npos )
   {
      if ( stPhaseFound )
      {
         if ( (rcPhase == ComponentManager::VAPOUR && pvtRCVapour) || (rcPhase == ComponentManager::LIQUID && pvtRCLiquid) )
         {
            ret = viscositiesST[rcPhase][stPhase];
         }
      }
      else if ( pvtRC && phaseMasses[rcPhase] > 0.0 ) { ret = viscositiesRC[rcPhase]; }
   }
   else if ( propertyName() == "GOR" )
   {
      if ( pvtRCLiquid )
      {
        rcPhase = ComponentManager::LIQUID;
         stPhase = ComponentManager::LIQUID;

         double volumeSTOIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

         rcPhase = ComponentManager::LIQUID;
         stPhase = ComponentManager::VAPOUR;

         double volumeSGIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

         if ( volumeSTOIIP != 0 ) { ret = volumeSGIIP / volumeSTOIIP; }
      }
   }
   else if ( propertyName() == "CGR" )
   {
      if ( pvtRCVapour )
      {
         rcPhase = ComponentManager::VAPOUR;
         stPhase = ComponentManager::LIQUID;

         double volumeCIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

         rcPhase = ComponentManager::VAPOUR;
         stPhase = ComponentManager::VAPOUR;

         double volumeFGIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NUMBER_OF_SPECIES );

         if ( volumeFGIIP != 0.0 ) { ret = volumeCIIP / volumeFGIIP; }
      }
   }
   else if ( propertyName() == "OilAPI" )
   {
      if ( pvtRCLiquid )
      {
         rcPhase = ComponentManager::LIQUID;
         stPhase = ComponentManager::LIQUID;

         if ( densitiesST[rcPhase][stPhase] != 0.0 )
         {
            ret = 141.5 / ( 0.001 * densitiesST[rcPhase][stPhase] ) - 131.5;
         }
      }
   }
   else if ( propertyName() == "CondensateAPI" )
   {
      if ( pvtRCVapour )
      {
         rcPhase = ComponentManager::VAPOUR;
         stPhase = ComponentManager::LIQUID;

         if ( densitiesST[rcPhase][stPhase] != 0.0 )
         {
            ret = 141.5 / ( 0.001 * densitiesST[rcPhase][stPhase] ) - 131.5;
         }
      }
   }
   else { throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Trap property " << propertyName() << " is not implemented yet"; }

   return ret;
}

} // namespace casa


bool performPVT( double masses[ComponentManager::NUMBER_OF_SPECIES]
               , double temperature
               , double pressure
               , double phaseMasses[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES]
               , double phaseDensities[ComponentManager::NUMBER_OF_PHASES]
               , double phaseViscosities[ComponentManager::NUMBER_OF_PHASES]
               )
{
   double massTotal = 0.0;

   for ( int comp = 0; comp < ComponentManager::NUMBER_OF_SPECIES; ++comp )
   {
      massTotal += masses[comp];

      for ( int phase = 0; phase < ComponentManager::NUMBER_OF_PHASES; ++phase )
      {
         phaseMasses[phase][comp] = 0;
      }
   }

   for ( int phase = 0; phase < ComponentManager::NUMBER_OF_PHASES; ++phase )
   {
      phaseDensities[phase] = 0;
      phaseViscosities[phase] = 0;
   }

   if ( massTotal > g_ZeroMassThreshold )
   {
      return pvtFlash::EosPack::getInstance().computeWithLumping( temperature + CelciusToKelvin
                                                                , pressure * MegaPaToPa
                                                                , masses
                                                                , phaseMasses
                                                                , phaseDensities
                                                                , phaseViscosities
                                                                );
   }
   return false;
}

double ComputeVolume( double * masses, double density, int numberOfSpecies )
{
   double value = Utilities::Numerical::IbsNoDataValue;

   double mass = 0.0;
   for ( int i = 0; i < numberOfSpecies; ++i ) { mass += masses[i]; }

   if ( mass < g_ZeroMassThreshold ) // in kg
   {
      value = 0;
   }
   else
   {
      if ( density == 0.0 ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Zero density computed with non-zero mass";
      value = mass / density;
   }
   return value;
}


