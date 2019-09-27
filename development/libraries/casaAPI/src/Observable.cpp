//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Observable.C
/// @brief This file keeps loaders for all possible types of Observable

#include "ObsGridPropertyXYZ.h"
#include "ObsGridPropertyWell.h"
#include "ObsSourceRockMapProp.h"
#include "ObsTrapProp.h"
#include "ObsTrapDerivedProp.h"
#include "ObsGridXYLayerTopSurfaceProp.h"

#include "cmbAPI.h"
#include "UndefinedValues.h"

#include "NumericFunctions.h"

namespace casa
{

Observable::Observable()
{
}

Observable::Observable( const std::string & propName, const double simTime )
                      : m_name{}
                      , m_propName{ propName }
                      , m_simTime{ simTime }
                      , m_saWeight{ 1.0 }
                      , m_uaWeight{ 1.0 }
                      , m_refValue{ nullptr }
                      , m_devValue{ nullptr }
                      , m_posDataMiningTbl{}
{
  if ( m_propName.empty() )
  {
    throw ErrorHandler::Exception( ErrorHandler::UndefinedValue )
        << "No property name specified for observable";
  }
}

Observable::~Observable()
{
}

double Observable::simulationTime() const
{
  return m_simTime;
}

void Observable::setSimTime( double simTime )
{
  m_simTime = simTime;
}

void Observable::setPropertyName( const std::string & propName )
{
  m_propName = propName;
}

void Observable::setName( std::vector<std::string> & name )
{
  m_name = name;
}

void Observable::setRefValue( const ObsValue * refValue )
{
  m_refValue.reset( refValue );
}

void Observable::createRefValues( const casa::ObsValue * refValue, const ObsValue * devValue )
{
  m_refValue.reset( refValue );
  m_devValue.reset( devValue );
}

const ObsValue * Observable::devValue() const
{
  return m_devValue.get();
}

void Observable::setDevValue( const ObsValue * devValue )
{
  m_devValue.reset( devValue );
}

bool Observable::checkObsMatchesModel( mbapi::Model      & caldModel
                                     , const int           iTable
                                     , const double        xCoord
                                     , const double        yCoord
                                     , const double        zCoord
                                     , const double        epsilon
                                     , const std::string & variableTypeName
                                     , const std::string & variableName ) const
{
  double obTimeModel = caldModel.tableValueAsDouble( Observable::s_dataMinerTable, iTable, "Time" );
  if ( caldModel.errorCode() != ErrorHandler::NoError || !NumericFunctions::isEqual( obTimeModel, simulationTime(), epsilon ) ) { return false; }

  double xCoordModel = caldModel.tableValueAsDouble( s_dataMinerTable, iTable, "XCoord" );
  if ( caldModel.errorCode() != ErrorHandler::NoError || !NumericFunctions::isEqual( xCoordModel, xCoord, epsilon ) ) { return false; }

  double yCoordModel = caldModel.tableValueAsDouble( s_dataMinerTable, iTable, "YCoord" );
  if ( caldModel.errorCode() != ErrorHandler::NoError || !NumericFunctions::isEqual( yCoordModel, yCoord, epsilon ) ) { return false; }

  if ( !IsValueUndefined( zCoord ) )
  {
    double zCoordModel = caldModel.tableValueAsDouble( s_dataMinerTable, iTable, "ZCoord" );
    if ( caldModel.errorCode() != ErrorHandler::NoError || !NumericFunctions::isEqual( zCoordModel, zCoord, epsilon ) ) { return false; }
  }

  if (!variableTypeName.empty() )
  {
    const std::string & variableNameModel = caldModel.tableValueAsString( Observable::s_dataMinerTable, iTable, variableTypeName );
    if ( caldModel.errorCode() != ErrorHandler::NoError || variableName != variableNameModel ) { return false; }
  }

  const std::string & propNameModel = caldModel.tableValueAsString( s_dataMinerTable, iTable, "PropertyName" );
  if ( caldModel.errorCode() != ErrorHandler::NoError || propertyName() != propNameModel ) { return false; }

  return true;
}

bool Observable::setCommonTableValues( mbapi::Model       & caldModel
                                     , const int            posData
                                     , const double         xCoord
                                     , const double         yCoord
                                     , const double         zCoord
                                     , const std::string  & variableTypeName
                                     , const std::string  & variableName  ) const
{
  if ( ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, posData, "Time",           m_simTime  ) ||
       ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, posData, "XCoord",         xCoord     ) ||
       ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, posData, "YCoord",         yCoord     ) ||
       ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, posData, "ZCoord",         zCoord     ) ||
       ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, posData, "PropertyName",   m_propName ) ||
       ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, posData, "Value",
                                                         Utilities::Numerical::IbsNoDataValue                                ) ||
       ( !variableTypeName.empty()
         ? ErrorHandler::NoError != caldModel.setTableValue( Observable::s_dataMinerTable, posData, variableTypeName, variableName )
         : false                                                                                                             )
     ) { return false; }

  return true;
}

std::vector<int> Observable::posDataMiningTbl() const
{
  return m_posDataMiningTbl;
}

void Observable::setPosDataMiningTbl( const std::vector<int>& posDataVec )
{
  m_posDataMiningTbl = posDataVec;
}

ObsValue * Observable::transform( const ObsValue * /* val */ ) const
{
  return 0;
}

size_t Observable::dimensionUntransformed() const
{
  return dimension();
}

std::vector<std::string> Observable::name() const
{
   return m_name;
}

std::string Observable::propertyName() const
{
  return m_propName;
}

bool Observable::hasReferenceValue() const
{
  return m_refValue.get();
}

const ObsValue * Observable::referenceValue() const
{
  return m_refValue.get();
}

const ObsValue * Observable::stdDeviationForRefValue() const
{
  return m_devValue.get();
}

double Observable::saWeight() const
{
  return m_saWeight;
}

void Observable::setSAWeight( const double w )
{
  m_saWeight = w;
}

void Observable::setUAWeight( const double w )
{
  m_uaWeight = w;
}

double Observable::uaWeight() const
{
  return m_uaWeight;
}

ObsValue * Observable::newObsValueFromDoubles( const std::vector<double> & vals, int & off )
{
   std::vector<double>::const_iterator it = vals.begin() + off;
   std::vector<double>::const_iterator sit = it;

   ObsValue * ret = createNewObsValueFromDouble( it );
   off += static_cast<unsigned int>( it - sit );
   return ret;
}

std::vector<bool> Observable::isValid( const ObsValue * obv ) const
{
   std::vector<bool> ret;

   if ( obv == nullptr ) { return ret; }

   const std::vector<double> & vals = obv->asDoubleArray();
   for ( auto v : vals ) { ret.push_back( !IsValueUndefined( v ) ); }
 
   return ret;
}

Observable * Observable::load( CasaDeserializer & dz, const char * objName )
{
   std::string  ot; // object type name
   std::string  on; // object name
   unsigned int vr; // object version

   dz.loadObjectDescription( ot, on, vr );
   if ( on.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Observable deserialization error, expected observable with name: " << objName
         << ", but stream gave object with name: " << on;
   }
   if (      ot == "ObsGridPropertyXYZ"           ) { return new ObsGridPropertyXYZ(           dz, vr ); }
   else if ( ot == "ObsGridPropertyWell"          ) { return new ObsGridPropertyWell(          dz, vr ); }
   else if ( ot == "ObsSourceRockMapProp"         ) { return new ObsSourceRockMapProp(         dz, vr ); }
   else if ( ot == "ObsTrapProp"                  ) { return new ObsTrapProp(                  dz, vr ); }
   else if ( ot == "ObsTrapDerivedProp"           ) { return new ObsTrapDerivedProp(           dz, vr ); }
   else if ( ot == "ObsGridXYLayerTopSurfaceProp" ) { return new ObsGridXYLayerTopSurfaceProp( dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Observable deserialization error: Unknown type: " << ot;
   }

   return 0;
}

} // namespace casa
