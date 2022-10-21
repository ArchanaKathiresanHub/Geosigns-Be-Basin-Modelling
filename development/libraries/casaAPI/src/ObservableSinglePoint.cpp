//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file  ObservableSinglePoint.cpp
/// @brief This file contains implementation of the class that contains the common functionalities of
///        all Observables with single point

#include "ObservableSinglePoint.h"
#include "ObsValueDoubleScalar.h"

#include "cmbAPI.h"

#include "LogHandler.h"
#include "Utilities.h"

#include <cmath>

namespace casa
{

ObservableSinglePoint::ObservableSinglePoint() {}

ObservableSinglePoint::ObservableSinglePoint( const std::string & myName
                                            , const std::string & propName
                                            , const double        simTime
                                            , const double        x
                                            , const double        y
                                            , const double        z
                                            )
                                            : Observable( propName, simTime )
                                            , m_x{ x }
                                            , m_y{ y }
                                            , m_z{ z }
{
  std::vector<std::string> name;
  if ( myName.empty() )
  {
    std::ostringstream oss;
    oss << propertyName() << "(" << m_x << "," << m_y << "," << m_z << "," << simulationTime() << ")";
    name.push_back( oss.str() );
  }
  else
  {
    name.push_back( myName );
  }

  setName( name );
}

ObservableSinglePoint::~ObservableSinglePoint() {}

size_t ObservableSinglePoint::dimension() const
{
  return 1;
}

// Check well against project coordinates
bool ObservableSinglePoint::checkObservableForProject( mbapi::Model & caldModel ) const
{
  double x0, y0;
  if ( caldModel.origin( x0, y0 ) != ErrorHandler::NoError )
   {
     return false;
   }

   double dimX, dimY;
   if ( caldModel.arealSize( dimX, dimY ) != ErrorHandler::NoError )
   {
     return false;
   }

   if ( m_x < x0 || m_x > x0 + dimX ||
        m_y < y0 || m_y > y0 + dimY )
   {
      return false;
   }

   return true;
}

// Check well against project coordinates and coordinates of observable attached to project
bool ObservableSinglePoint::checkObservableOriginForProject( mbapi::Model & caldModel ) const
{
   double x0, y0;
   if ( caldModel.origin( x0, y0 ) != ErrorHandler::NoError )
   {
     return false;
   }

   double dimX, dimY;
   if ( caldModel.arealSize( dimX, dimY ) != ErrorHandler::NoError )
   {
     return false;
   }

   if ( m_x < x0 || m_x > x0 + dimX ||
        m_y < y0 || m_y > y0 + dimY )
   {
     return false;
   }
   else
   {
     double xCoordWindowObs, yCoordWindowObs;
     if ( caldModel.windowObservableOrigin( xCoordWindowObs, yCoordWindowObs ) != ErrorHandler::NoError )
     {
       return false;
     }

     // If the observable coordinate distance from the window observable coordinate is larger than 1 [m], the observable does not belong to this well.
     // Although, it falls within the boundaries of the current window.

     if ( std::fabs( m_x - xCoordWindowObs ) > 1.0 ||
          std::fabs( m_y - yCoordWindowObs ) > 1.0 )
     {
       return false;
     }
   }

   return true;
}

void ObservableSinglePoint::setReferenceValue( ObsValue * obsVal, ObsValue * devVal )
{
   assert( obsVal != nullptr );
   ObsValueDoubleScalar * val = dynamic_cast<ObsValueDoubleScalar*>( obsVal );
   assert( val != nullptr );

   assert( devVal != nullptr );
   ObsValueDoubleScalar * dev = dynamic_cast<ObsValueDoubleScalar*>( devVal );
   assert( dev != nullptr );

   // check dev for negative/zero value
   if ( dev->value() <= 0.0 )
   {
      double newDev = std::abs( val->value() ) * 0.1;
      if ( newDev == 0.0 ) { newDev = 0.1; }

      LogHandler( LogHandler::WARNING_SEVERITY ) << "Invalid the standard deviation value: " << dev->value()
                                                 << " for the target " << name()[0] << ", possible error in scenario setup. "
                                                 << "Replacing it with the default value (0.1*refVal): " << newDev;
      delete devVal;
      devVal = ObsValueDoubleScalar::createNewInstance( this, newDev );
   }

   createRefValues( obsVal, devVal );
}

std::vector<double> ObservableSinglePoint::depth() const
{
  return std::vector<double>(1, m_z);
}

std::vector<double> ObservableSinglePoint::xCoords() const
{
  return std::vector<double>(1, m_x);
}

std::vector<double> ObservableSinglePoint::yCoords() const
{
  return std::vector<double>(1, m_y);
}

double ObservableSinglePoint::xCoord() const
{
  return m_x;
}

double ObservableSinglePoint::yCoord() const
{
  return m_y;
}

double ObservableSinglePoint::zCoord() const
{
  return m_z;
}

void ObservableSinglePoint::setCoords(const double x, const double y, const double z)
{
  m_x = x;
  m_y = y;
  m_z = z;
}

void ObservableSinglePoint::saveCommon( const Observable * observableClass
                                      , CasaSerializer & sz
                                      , bool & ok
                                      , const std::string & variableTypeName
                                      , const std::string & variableName ) const
{
  // register observable with serializer to allow ObsValue objects keep reference after deserializtion
  CasaSerializer::ObjRefID obID = sz.ptr2id( observableClass );

  ok = sz.save( obID, "ID" );
  ok = ok ? sz.save( m_x, "X" ) : ok;
  ok = ok ? sz.save( m_y, "Y" ) : ok;

  if ( !Utilities::isValueUndefined( m_z ) )
  {
    ok = ok ? sz.save( m_z, "Z" ) : ok;
  }

  if ( !variableTypeName.empty() )
  {
    ok = ok ? sz.save( variableName, variableTypeName ) : ok;
  }

  ok = ok ? sz.save( propertyName(),  "propName"  ) : ok;
  ok = ok ? sz.save( simulationTime(),   "simTime"   ) : ok;
  ok = ok ? sz.save( name(), "name" ) : ok;
  ok = ok ? sz.save( posDataMiningTbl(), "posDataMiningTbl" ) : ok;

  bool hasRefVal = referenceValue();
  ok = ok ? sz.save( hasRefVal, "HasRefValue" ) : ok;
  if ( hasRefVal ) { ok = ok ? sz.save( *( referenceValue() ), "refValue" ) : ok; }

  bool hasDevVal = devValue();
  ok = ok ? sz.save( hasDevVal, "HasDevVal" ) : ok;
  if ( hasDevVal ) { ok = ok ? sz.save( *( devValue() ), "devValue" ) : ok; }

  ok = ok ? sz.save( saWeight(), "saWeight" ) : ok;
  ok = ok ? sz.save( uaWeight(), "uaWeight" ) : ok;
}

} // namespace casa
