//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsGridPropertyWell.cpp

// CASA API
#include "ObsValueDoubleArray.h"
#include "ObsValueDoubleScalar.h"
#include "ObsGridPropertyWell.h"

// CMB API
#include "cmbAPI.h"
#include "NumericFunctions.h"

// utilities lib
#include "LogHandler.h"

// STL/C lib
#include <cassert>
#include <sstream>

using namespace Utilities::Numerical;

namespace casa
{

// Create observable for the given grid property for specified grid position
ObsGridPropertyWell::ObsGridPropertyWell( const std::vector<double> & x
                                        , const std::vector<double> & y
                                        , const std::vector<double> & z
                                        , const char                * propName
                                        , double                      simTime
                                        , const std::string         & myName
                                        )
                                        : Observable( propName, simTime )
                                        , m_x( x.begin(), x.end() )
                                        , m_y( y.begin(), y.end() )
                                        , m_z( z.begin(), z.end() )
{
   if ( m_x.size() != m_y.size() || m_x.size() != m_z.size() )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No property name specified for well target";
   }

   setPosDataMiningTbl( std::vector<int>( x.size(), -1 ) );

   std::vector<std::string> name;
   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      std::ostringstream oss;

      if ( myName.empty() )
      {
         oss << propertyName() << "(" << m_x[i] << "," << m_y[i] << "," << m_z[i] << "," << simulationTime() << ")";
      }
      else
      {
        oss << myName << "_" << i+1;
      }

      name.push_back( oss.str() );
   }
   setName( name );
}

// Destructor
ObsGridPropertyWell::~ObsGridPropertyWell() {}

size_t ObsGridPropertyWell::dimension() const
{
  return m_x.size();
}

// Get standard deviations for the reference value
void ObsGridPropertyWell::setReferenceValue( ObsValue * obsVal, ObsValue * devVal )
{
   assert( obsVal != nullptr );
   ObsValueDoubleArray * val = dynamic_cast<ObsValueDoubleArray*>( obsVal );
   assert( val != nullptr );

   setRefValue( obsVal );

   assert( devVal != nullptr );
   ObsValueDoubleArray * dev = dynamic_cast<ObsValueDoubleArray*>( devVal );
   assert( dev != nullptr );

   // check dev for negative/zero value
   std::vector<double> refLst = val->asDoubleArray();
   std::vector<double> devLst = dev->asDoubleArray();

   size_t numOfValid = 0;
   double validAv    = 0.0;
   double eps        = 1.e-12;

   bool isUpdated = false;
   // calculate average over defined std deviations along well
   for ( size_t i = 0; i < devLst.size(); ++i )
   {
      if ( devLst[i] > eps )
      {
         validAv += devLst[0];
         ++numOfValid;
      }
   }
   if ( numOfValid > 0 ) { validAv /= static_cast<double>( numOfValid ); }

   for ( size_t i = 0; i < devLst.size(); ++i )
   {
      if ( devLst[i] <= eps )
      {
         double newDev = numOfValid > 0 ? validAv : std::abs( refLst[i] ) * 0.1;
         if ( newDev < eps ) { newDev = 0.1; }

         LogHandler( LogHandler::WARNING_SEVERITY ) << "Invalid the standard deviation value: " << devLst[i]
                                                    << " for the target " << name()[i] << ", possible error in scenario setup? "
                                                    << "Replacing it with the value: " << newDev;
         devLst[i] = newDev;
         isUpdated = true;
      }
   }

   if ( isUpdated ) // if we changed any deviation value, recreate the ObsValue object with new std. deviation values
   {
     delete devVal;
     devVal = ObsValueDoubleArray::createNewInstance( this, devLst );
   }

   setDevValue( devVal );
}

std::vector<double> ObsGridPropertyWell::depth() const
{
  return m_z;
}

std::vector<double> ObsGridPropertyWell::xCoords() const
{
  return m_x;
}

std::vector<double> ObsGridPropertyWell::yCoords() const
{
  return m_y;
}

// Update Model to be sure that requested property will be saved at the requested time
ErrorHandler::ReturnCode ObsGridPropertyWell::requestObservableInModel( mbapi::Model & caldModel )
{
   if ( ErrorHandler::NoError != caldModel.snapshotManager().requestMajorSnapshot(       simulationTime() ) ||
        ErrorHandler::NoError != caldModel.propertyManager().requestPropertyInSnapshots( propertyName() )
      ) return caldModel.errorCode();

   size_t tblSize = caldModel.tableSize( s_dataMinerTable );

   std::vector<int> posVec = posDataMiningTbl();
   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      if ( ErrorHandler::NoError != caldModel.addRowToTable( s_dataMinerTable ) ) return caldModel.errorCode();

      posVec[i] = static_cast<int>( tblSize + i );

      if ( !setCommonTableValues( caldModel, posVec[i], m_x[i], m_y[i], m_z[i] ) )
      { return caldModel.errorCode(); }
   }
   setPosDataMiningTbl( posVec );

   return ErrorHandler::NoError;
}


// Get this observable value from Cauldron model
ObsValue * ObsGridPropertyWell::getFromModel( mbapi::Model & caldModel )
{
   std::vector<double> vals( posDataMiningTbl().size(), IbsNoDataValue );

   if ( !checkObservableForProject( caldModel ) )
   {
     return new ObsValueDoubleArray( this, vals );
   }

   const double eps = 1.e-5;

   std::vector<int> posVec = posDataMiningTbl();
   for ( size_t i = 0; i < posVec.size(); ++i )
   {
      if ( posVec[i] < 0 )
      {
         size_t tblSize = caldModel.tableSize( Observable::s_dataMinerTable );
         for ( size_t j = 0; j < tblSize; ++j )
         {
           if ( !checkObsMatchesModel( caldModel, j, m_x[i], m_y[i], m_z[i], eps ) ) { continue; }

            vals[i] = caldModel.tableValueAsDouble( s_dataMinerTable, j, "Value" );
            posVec[i] = static_cast<int>( j ); // fill the rest of the table as well data must be continuous
            for ( size_t k = i + 1; k < posVec.size(); ++k )
            {
               posVec[k] = posVec[k-1]+1;
            }
            break;
         }
      }
      else { vals[i] = caldModel.tableValueAsDouble( s_dataMinerTable, posVec[i], "Value" ); }

      if ( caldModel.errorCode() != ErrorHandler::NoError ) return nullptr;
   }
   setPosDataMiningTbl( posVec );

   return new ObsValueDoubleArray( this, vals );
}

// Check well against project coordinates
bool ObsGridPropertyWell::checkObservableForProject( mbapi::Model & caldModel ) const
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

   for ( size_t i = 0; i < m_x.size(); ++i )
   {
     if ( m_x[i] < x0 || m_x[i] > x0 + dimX ||
          m_y[i] < y0 || m_y[i] > y0 + dimY )
     {
       return false;
     }
   }

   return true;
}

// Check well against project coordinates and coordinates of observable attached to project
bool ObsGridPropertyWell::checkObservableOriginForProject( mbapi::Model & caldModel ) const
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

   for ( size_t i = 0; i < m_x.size(); ++i )
   {
     if ( m_x[i] < x0 || m_x[i] > x0 + dimX ||
          m_y[i] < y0 || m_y[i] > y0 + dimY )
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

       if ( std::fabs( m_x[i] - xCoordWindowObs ) > 1.0 ||
            std::fabs( m_y[i] - yCoordWindowObs ) > 1.0 )
       {
         return false;
       }
     }
   }

   return true;
}

// Create this observable value from double array (converting data from SUMlib for response surface evaluation
ObsValue * ObsGridPropertyWell::createNewObsValueFromDouble( std::vector<double>::const_iterator & val ) const
{
   std::vector<double> obsVal( m_x.size(), IbsNoDataValue );

   for ( size_t i = 0; i < m_x.size(); ++i )
   {
      obsVal[i] = *val++;
   }

   return new ObsValueDoubleArray( this, obsVal );
}

void ObsGridPropertyWell::saveCommon( const Observable * observableClass
                                    , CasaSerializer & sz
                                    , bool & ok
                                    , const std::string& variableTypeName
                                    , const std::string& variableName ) const {}

bool ObsGridPropertyWell::save( CasaSerializer & sz ) const
{
   // register observable with serializer to allow ObsValue objects keep reference after deserializtion
   CasaSerializer::ObjRefID obID = sz.ptr2id( this );

   bool ok = sz.save( obID, "ID" );
   ok = ok ? sz.save( m_x, "X" ) : ok;
   ok = ok ? sz.save( m_y, "Y" ) : ok;
   ok = ok ? sz.save( m_z, "Z" ) : ok;
   ok = ok ? sz.save( propertyName(), "propName" ) : ok;
   ok = ok ? sz.save( simulationTime(), "simTime" ) : ok;
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

   return ok;
}

ObsGridPropertyWell::ObsGridPropertyWell( CasaDeserializer & dz, unsigned int objVer )
{
   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
         "Version of ObsGridPropertyWell in file is newer. No forward compatibility!";
   }

   CasaDeserializer::ObjRefID obID;

   // load data necessary to create an object
   bool ok = dz.load( obID, "ID" );

   // register observable with deserializer under read ID to allow ObsValue objects keep reference after deserializtion
   dz.registerObjPtrUnderID( this, obID );

   ok = ok ? dz.load( m_x, "X" ) : ok;
   ok = ok ? dz.load( m_y, "Y" ) : ok;
   ok = ok ? dz.load( m_z, "Z" ) : ok;

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
   ok = ok ? dz.load( posVec, "posDataMiningTbl" ) : ok;
   setPosDataMiningTbl( posVec );

   bool hasRefVal;
   ok = ok ? dz.load( hasRefVal, "HasRefValue" ) : ok;
   if ( hasRefVal ) { setRefValue( ObsValue::load( dz, "refValue" ) ); }

   if ( objVer == 0 )
   {
      double val;
      ok = ok ? dz.load( val, "devValue" ) : ok;
      if ( ok ) { setDevValue( new  ObsValueDoubleScalar( this, val ) ); }
   }
   else
   {
      bool hasDevVal;
      ok = ok ? dz.load( hasDevVal, "HasDevVal" ) : ok;
      if ( hasDevVal ) { setDevValue( ObsValue::load( dz, "devValue" ) ); }
   }

   double mySAWeight;
   double myUAWeight;
   ok = ok ? dz.load( mySAWeight, "saWeight" ) : ok;
   ok = ok ? dz.load( myUAWeight, "uaWeight" ) : ok;
   setSAWeight( mySAWeight );
   setUAWeight( myUAWeight );

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsGridPropertyWell deserialization unknown error";
   }
}

} // namespace casa
