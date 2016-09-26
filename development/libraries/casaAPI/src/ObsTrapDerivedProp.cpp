//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsTrapDerivedProp.C

// CASA API
#include "ObsValueDoubleScalar.h"
#include "ObsValueDoubleArray.h"
#include "ObsValueTransformable.h"
#include "ObsTrapDerivedProp.h"

// CMB API
#include "cmbAPI.h"
#include "UndefinedValues.h"

// Utilities lib
#include "NumericFunctions.h"

// EosPack
#include "EosPack.h"

// CBMGenerics
#include "ComponentManager.h"
#include "consts.h"
using namespace CBMGenerics;

// STL/C lib
#include <cassert>
#include <sstream>

static const char * g_PropCompSuffix       = "TrappedAmount";

static const double g_ZeroMassLogThreshold = -5.0;  // if ( log10( mass ) < ZeroMassLogThreshold ) - mass value is treated as zero 
static const double g_ZeroMassThreshold    = pow( 10.0, g_ZeroMassThreshold );

// Some auxillary functions
static bool performPVT( double masses[ComponentManager::NumberOfOutputSpecies]
                      , double temperature
                      , double pressure
                      , double phaseMasses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]
                      , double phaseDensities[ComponentManager::NumberOfPhases]
                      , double phaseViscosities[ComponentManager::NumberOfPhases]
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
                                      , const std::string & name
                                      )
                                      : m_x( x )
                                      , m_y( y )
                                      , m_resName( resName )
                                      , m_propName( propName )
                                      , m_simTime( simTime )
                                      , m_saWeight( 1.0 )
                                      , m_uaWeight( 1.0 )
                                      , m_logTransf( logTrans )
{
   // check input values
   if ( m_propName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No property name specified for trap target";

   if ( name.empty() )
   {
      std::ostringstream oss;
      oss << m_propName << "(" << m_x << "," << m_y << "," << m_resName << "," << m_simTime << ")";
      m_name.push_back( oss.str() );
   }
   else { m_name.push_back( name ); }
}

// Destructor
ObsTrapDerivedProp::~ObsTrapDerivedProp() {;}

// Get name of the observable
std::vector<std::string> ObsTrapDerivedProp::name() const { return m_name; }

size_t ObsTrapDerivedProp::dimensionUntransformed() const { return ComponentManager::NumberOfOutputSpecies + 4; }

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

   for ( auto it : values ) { if ( it == UndefinedDoubleValue ) { allDefined = false; break; } }

   double ret = UndefinedDoubleValue;
   if ( allDefined && values.size() == ComponentManager::NumberOfOutputSpecies + 4 )
   {
      // do back transform from log10
      for ( size_t i = 0; i < ComponentManager::NumberOfOutputSpecies + 2; ++i )
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
   else if ( allDefined ) { assert( 0 ); } // shouldn't get here

   return ObsValueDoubleScalar::createNewInstance( this, ret );
}


// Get standard deviations for the reference value
void ObsTrapDerivedProp::setReferenceValue( ObsValue * obsVal, ObsValue * devVal )
{
   assert( obsVal != NULL );
   assert( devVal != NULL );

   m_refValue.reset( obsVal );
   m_devValue.reset( devVal );
}

// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsTrapDerivedProp::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot( m_simTime ) ) return caldModel.errorCode();

   std::vector<std::string> propList;

   for ( int i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i )
   {
      propList.push_back( ComponentManager::GetSpeciesName( i ) + g_PropCompSuffix );
   }
   propList.push_back( "MassLiquid" );
   propList.push_back( "MassVapour" );
   propList.push_back( "Pressure" );
   propList.push_back( "Temperature" );

   // add rows to the table
   size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );
   m_posDataMiningTbl.clear();

   for ( size_t i = 0; i < propList.size(); ++i )
   {  // insert new row into the table
      if ( ErrorHandler::NoError != caldModel.addRowToTable( Observable::s_dataMinerTable ) ) return caldModel.errorCode();

      int pos = static_cast<int>( tblSize ) + i;
      m_posDataMiningTbl.push_back( pos );

      // fill columns for the new row with values
      if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, pos, "Time",          m_simTime            ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, pos, "XCoord",        m_x                  ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, pos, "YCoord",        m_y                  ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, pos, "ZCoord",        UndefinedDoubleValue ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, pos, "ReservoirName", m_resName            ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, pos, "PropertyName",  propList[i]          ) ||
           ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, pos, "Value",         UndefinedDoubleValue ) 
      ) return caldModel.errorCode();
   }
   return ErrorHandler::NoError;
}

  
// Get this observable value from Cauldron model
ObsValue * ObsTrapDerivedProp::getFromModel( mbapi::Model & caldModel )
{
   std::vector<double> val( m_posDataMiningTbl.size(), UndefinedDoubleValue );
   double eps = g_ZeroMassThreshold;
 
   const std::string & msg = checkObservableForProject( caldModel );
   if ( !msg.empty() ) { return ObsValueTransformable::createNewInstance( this, val ); }
  
   size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );

   for ( size_t i = 0; i < m_posDataMiningTbl.size(); ++i )
   {
      std::string propName;
      if (      i <  ComponentManager::NumberOfOutputSpecies     ) { propName = ComponentManager::GetSpeciesName( i ) + g_PropCompSuffix; }
      else if ( i == ComponentManager::NumberOfOutputSpecies     ) { propName = "MassLiquid";  }
      else if ( i == ComponentManager::NumberOfOutputSpecies + 1 ) { propName = "MassVapour";  }
      else if ( i == ComponentManager::NumberOfOutputSpecies + 2 ) { propName = "Pressure";    }
      else if ( i == ComponentManager::NumberOfOutputSpecies + 3 ) { propName = "Temperature"; }

      if ( m_posDataMiningTbl[i] < 0 )
      {
         bool found = false;
         for ( size_t j = 0; j < tblSize && !found; ++j )
         {
            double obTime = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "Time"   );
            if ( caldModel.errorCode() != ErrorHandler::NoError || ! NumericFunctions::isEqual( obTime, m_simTime,            eps ) ) { continue; }

            double xCrd   = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "XCoord" );
            if ( caldModel.errorCode() != ErrorHandler::NoError || ! NumericFunctions::isEqual( xCrd,   m_x,                  eps ) ) { continue; }

            double yCrd   = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "YCoord" );
            if ( caldModel.errorCode() != ErrorHandler::NoError || ! NumericFunctions::isEqual( yCrd,   m_y,                  eps ) ) { continue; }

            double zCrd   = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "ZCoord" );
            if ( caldModel.errorCode() != ErrorHandler::NoError || ! NumericFunctions::isEqual( zCrd,   UndefinedDoubleValue, eps ) ) { continue; }
                       
            const std::string & resName = caldModel.tableValueAsString( Observable::s_dataMinerTable, j, "ReservoirName" );
            if ( caldModel.errorCode() != ErrorHandler::NoError || m_resName != resName ) { continue; }
                           
            const std::string & pName   = caldModel.tableValueAsString( Observable::s_dataMinerTable, j, "PropertyName"  );
            if ( caldModel.errorCode() != ErrorHandler::NoError || propName != pName    ) { continue; }
                              
            found = true;
            val[i] = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, j, "Value" );

            m_posDataMiningTbl[i] = static_cast<int>( j ); // fill the rest of the table as well data must be continuous
            for ( size_t k = i+1; k < m_posDataMiningTbl.size(); ++k )
            {
               m_posDataMiningTbl[k] = m_posDataMiningTbl[k-1] + 1;
            }
         }
      }
      else
      {
         val[i] = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, m_posDataMiningTbl[i], "Value" );
      }
   }

   for ( size_t i = 0; i < ComponentManager::NumberOfOutputSpecies + 2; ++i )
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


// Check well against project coordinates
std::string ObsTrapDerivedProp::checkObservableForProject( mbapi::Model & caldModel ) const
{
   std::ostringstream oss;

   double x0, y0;
   caldModel.origin( x0, y0 );
   
   double dimX, dimY;
   caldModel.arealSize( dimX, dimY );

   if ( m_x < x0 || m_x > x0 + dimX ||
        m_y < y0 || m_y > y0 + dimY )
   {
      oss << "Trap is outside of the project boundaries"; 
   }

   return oss.str();
}


// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsTrapDerivedProp::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{  
   std::vector<double> values( val, val + ComponentManager::NumberOfOutputSpecies + 4 );
   val += ComponentManager::NumberOfOutputSpecies + 4;

   return ObsValueTransformable::createNewInstance( this, values );
}

bool ObsTrapDerivedProp::save( CasaSerializer & sz, unsigned int /* version */) const
{
   // register observable with serializer to allow ObsValue objects keep reference after deserializtion
   CasaSerializer::ObjRefID obID = sz.ptr2id( this ); 

   bool ok = sz.save( obID, "ID" );
   ok = ok ? sz.save( m_x, "X" ) : ok;
   ok = ok ? sz.save( m_y, "Y" ) : ok;

   ok = ok ? sz.save( m_resName,  "reservoirName" ) : ok;
   ok = ok ? sz.save( m_propName, "propName"      ) : ok;
   ok = ok ? sz.save( m_simTime,  "simTime"       ) : ok;

   ok = ok ? sz.save( m_name, "name" ) : ok;

   ok = ok ? sz.save( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;

   bool hasRefVal = m_refValue.get() ? true : false;
   ok = ok ? sz.save( hasRefVal, "HasRefValue" ) : ok;
   if ( hasRefVal ) { ok = ok ? sz.save( *(m_refValue.get()), "refValue" ) : ok; }
   
   bool hasDevVal = m_devValue.get() ? true : false;
   ok = ok ? sz.save( hasDevVal, "HasDevVal" ) : ok;
   if ( hasDevVal ) { ok = ok ? sz.save( *(m_devValue.get()), "devValue" ) : ok; }

   ok = ok ? sz.save( m_saWeight, "saWeight" ) : ok;
   ok = ok ? sz.save( m_uaWeight, "uaWeight" ) : ok;

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

   ok = ok ? dz.load( m_x, "X" ) : ok;
   ok = ok ? dz.load( m_y, "Y" ) : ok;

   ok = ok ? dz.load( m_resName,  "reservoirName" ) : ok;
   ok = ok ? dz.load( m_propName, "propName"      ) : ok;
   ok = ok ? dz.load( m_simTime,  "simTime"       ) : ok;
   ok = ok ? dz.load( m_name,     "name"          ) : ok;
   ok = ok ? dz.load( m_posDataMiningTbl, "posDataMiningTbl" ) : ok;

   bool hasVal;
   
   ok = ok ? dz.load( hasVal, "HasRefValue" ) : ok;
   if ( hasVal ) { m_refValue.reset( ObsValue::load( dz, "refValue" ) ); }

   ok = ok ? dz.load( hasVal, "HasDevVal" ) : ok;
   if ( hasVal ) { m_devValue.reset( ObsValue::load( dz, "devValue" ) ); }

   ok = ok ? dz.load( m_saWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( m_uaWeight, "uaWeight" ) : ok;
   
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
   double ret = UndefinedDoubleValue;

   if ( vals.size() < ComponentManager::NumberOfOutputSpecies + 4 ) return ret;

   double massLiq = vals[ComponentManager::NumberOfOutputSpecies    ];
   double massVap = vals[ComponentManager::NumberOfOutputSpecies + 1];
   double P       = vals[ComponentManager::NumberOfOutputSpecies + 2];
   double T       = vals[ComponentManager::NumberOfOutputSpecies + 3];

   if ( massLiq + massVap < 1.0e-3 ) return ret; // not possible to calculate for zero mass of HC

   double masses[ComponentManager::NumberOfOutputSpecies];
   for ( int comp = 0; comp < ComponentManager::NumberOfOutputSpecies; ++comp ) { masses[comp] = vals[comp]; }

   // reservoir condition phases
   double massesRC[     ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies];
   double densitiesRC[  ComponentManager::NumberOfPhases];
   double viscositiesRC[ComponentManager::NumberOfPhases];

   // stock tank phases of reservoir condition phases
   double massesST[     ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies];
   double densitiesST[  ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases];
   double viscositiesST[ComponentManager::NumberOfPhases][ComponentManager::NumberOfPhases];

   // perform PVT under reservoir conditions
   if ( !performPVT( masses, T, P, massesRC, densitiesRC, viscositiesRC ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Can not perform PVT calculation for reservoir conditions";
   }

   // perform PVT's of reservoir condition phases under stock tank conditions
   if ( !performPVT( massesRC[ComponentManager::Vapour], StockTankTemperatureC, StockTankPressureMPa,
                     massesST[ComponentManager::Vapour], densitiesST[ComponentManager::Vapour], viscositiesST[ComponentManager::Vapour] 
                   )
      )
   { 
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Can not perform PVT calculation for vapour phase for surface conditions";
   }
      
   if ( !performPVT( massesRC[ComponentManager::Liquid], StockTankTemperatureC, StockTankPressureMPa,
                     massesST[ComponentManager::Liquid], densitiesST[ComponentManager::Liquid], viscositiesST[ComponentManager::Liquid]
                   )
      )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Can not perform PVT calculation for liquid phase for surface conditions";
   }

   bool stPhaseFound = false;
   bool rcPhaseFound = false;

   ComponentManager::PhaseId rcPhase;
   ComponentManager::PhaseId stPhase;

   if ( m_propName.find( "FGIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Vapour;
   }
   else if (m_propName.find( "CIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Liquid;
   }
   else if (m_propName.find( "SGIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Vapour;
   }
   else if ( m_propName.find( "STOIIP" ) != string::npos )
   {
      stPhaseFound = true;
      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Liquid;
   }
   else if ( m_propName.find( "Vapour" ) != string::npos )
   {
      rcPhaseFound = true;
      rcPhase = ComponentManager::Vapour;
   }
   else if ( m_propName.find( "Liquid" ) != string::npos )
   {
      rcPhaseFound = true;
      rcPhase = ComponentManager::Liquid;
   }

   // Volume, Density, Viscosity, and Mass properties for stock tank conditions
   if ( m_propName.find( "Density" ) != string::npos )
   {
      ret = stPhaseFound ? densitiesST[rcPhase][stPhase]   : densitiesRC[rcPhase];
   }
   else if ( m_propName.find( "Viscosity" ) != string::npos )
   {
      ret = stPhaseFound ? viscositiesST[rcPhase][stPhase] : viscositiesRC[rcPhase];
   }
   else if ( m_propName == "GOR" )
   {
      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Liquid;

      double volumeSTOIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies );

      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Vapour;

      double volumeSGIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies );

      if ( volumeSTOIIP != 0 ) { ret = volumeSGIIP / volumeSTOIIP; }
   }
   else if ( m_propName == "CGR" )
   {
      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Liquid;

      double volumeCIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies );

      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Vapour;

      double volumeFGIIP = ComputeVolume( massesST[rcPhase][stPhase], densitiesST[rcPhase][stPhase], ComponentManager::NumberOfOutputSpecies );

      if ( volumeFGIIP != 0.0 ) { ret = volumeCIIP / volumeFGIIP; }
   }
   else if ( m_propName == "OilAPI" )
   {
      rcPhase = ComponentManager::Liquid;
      stPhase = ComponentManager::Liquid;

      if ( densitiesST[rcPhase][stPhase] != 0.0 )
      {
         ret = 141.5 / ( 0.001 * densitiesST[rcPhase][stPhase] ) - 131.5;
      }
   }
   else if ( m_propName == "CondensateAPI" )
   {
      rcPhase = ComponentManager::Vapour;
      stPhase = ComponentManager::Liquid;

      if ( densitiesST[rcPhase][stPhase] != 0.0 )
      {
         ret = 141.5 / ( 0.001 * densitiesST[rcPhase][stPhase] ) - 131.5;
      }
   }
   else { throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Trap property " << m_propName << " is not implemented yet"; }

   return ret;
}

}


bool performPVT( double masses[ComponentManager::NumberOfOutputSpecies]
               , double temperature
               , double pressure
               , double phaseMasses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies]
               , double phaseDensities[ComponentManager::NumberOfPhases]
               , double phaseViscosities[ComponentManager::NumberOfPhases]
               )
{
   double massTotal = 0.0;

   for ( int comp = 0; comp < ComponentManager::NumberOfOutputSpecies; ++comp )
   {
      massTotal += masses[comp];

      for ( int phase = 0; phase < ComponentManager::NumberOfPhases; ++phase )
      {
         phaseMasses[phase][comp] = 0;
      }
   }

   for ( int phase = 0; phase < ComponentManager::NumberOfPhases; ++phase )
   {
      phaseDensities[phase] = 0;
      phaseViscosities[phase] = 0;
   }

   return pvtFlash::EosPack::getInstance().computeWithLumping( temperature + C2K
                                                             , pressure * MPa2Pa
                                                             , masses
                                                             , phaseMasses
                                                             , phaseDensities 
                                                             , phaseViscosities
                                                             );
}

double ComputeVolume( double * masses, double density, int numberOfSpecies )
{
   double value = UndefinedDoubleValue;

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


