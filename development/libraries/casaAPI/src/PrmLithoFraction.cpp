//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmLithoFraction.C
/// @brief This file keeps API implementation for lithofraction parameter handling.

// CASA APIlayerLithologiesList
#include "PrmLithoFraction.h"
#include "VarPrmLithoFraction.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{
   static const char * s_stratIoTblName = "StratIoTbl";
   static const char * s_lithoTypePercent1GridFiledName = "Percent1Grid";
   static const char * s_lithoTypePercent2GridFiledName = "Percent2Grid";

   std::vector<double> PrmLithoFraction::createLithoPercentages( const std::vector<double> & lithoFractions, const std::vector<int> & lithoFractionInds )
   {
      //the litho fractions must be 2
      if ( lithoFractions.size() != 2 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The number of litho fractions must be 2 ";
      }

      //the litho indexes must be 2
      if ( lithoFractionInds.size() != 2 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The number of litho fraction indexes must be 2 ";
      }

      //the first lithofraction is always a total percentage ranging from 0.0 to 100.0
      if ( lithoFractions[0] < 0.0 || lithoFractions[0] > 100.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The percentage of the lithology " << lithoFractionInds[0] << 
                                                                           " is out of range [0:100]: "       << lithoFractions[0];
      }

      //the second lithofraction is always a ratio ranging from 0 to 1
      if ( lithoFractions[1] < 0.0 || lithoFractions[1] > 1.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The ratio of the lithology " << lithoFractionInds[1] << 
                                                                           " is out of range [0:1]: "    << lithoFractions[1];
      }

      const int numPercentages = 3;
      std::vector<double> lithoPercentages( numPercentages );

      lithoPercentages[lithoFractionInds[0]] = lithoFractions[0];

      //the second lithofraction defines the fraction of what is left. it is varing from 0 to 1. 
      double percentage = lithoFractions[1] * ( 100.0 - lithoFractions[0] );
      lithoPercentages[lithoFractionInds[1]] = percentage;

      percentage = 100.0 - lithoFractions[0] - percentage;
      for ( int i = 0; i != numPercentages; ++i )
      {
         if ( i != lithoFractionInds[0] && i != lithoFractionInds[1] )
         {
            if ( percentage < 0.0 || percentage > 100.0 ) 
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The percentage of the lithology " << i << 
                                                                                 " is not valid : " << percentage;
            }
            lithoPercentages[i] = percentage;
         }
      }
      return lithoPercentages;
   }

   std::vector<double> PrmLithoFraction::createLithoFractions( const std::vector<double> & lithoPercentages, const std::vector<int> & lithoFractionInds )
   {

      //the litho indexes must be 2
      if ( lithoFractionInds.size() != 2 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The number of litho fraction indexes must be 2 ";
      }

      std::vector< double> lithoFractions;
      if ( lithoPercentages[lithoFractionInds[0]] < 0.0 || lithoPercentages[lithoFractionInds[0]] > 100.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The percentage of the lithology " << lithoFractionInds[0] << 
                                                                           " is out of range [0:100]: " << lithoPercentages[lithoFractionInds[0]];
      }

      lithoFractions.push_back( lithoPercentages[lithoFractionInds[0]] );

      if ( lithoPercentages[lithoFractionInds[0]] == 100.0 )
      {
         lithoFractions.push_back( 0.5 ); //here we assume that both fractions shares the same 0% left
      }
      else
      {
         lithoFractions.push_back( lithoPercentages[lithoFractionInds[1]] / ( 100.0 - lithoPercentages[lithoFractionInds[0]] ) );
      }

      return lithoFractions;
   }

   // Constructor: read from the model
   PrmLithoFraction::PrmLithoFraction( mbapi::Model & mdl, const std::string & layerName, const std::vector<int> & lithoFractionsInds )
      : m_parent( 0 )
      , m_layerName( layerName )
      , m_lithoFractionsInds( lithoFractionsInds )
   {
      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager( );
      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode( ) != ErrorHandler::NoError )
      {
         throw ErrorHandler::Exception( stMgr.errorCode( ) ) << stMgr.errorMessage( );
      }

      const std::string & percent1GridFiledName = mdl.tableValueAsString( s_stratIoTblName, lid, s_lithoTypePercent1GridFiledName );
      if ( ErrorHandler::NoError != mdl.errorCode() ) { throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage(); }

      const std::string & percent2GridFiledName = mdl.tableValueAsString( s_stratIoTblName, lid, s_lithoTypePercent2GridFiledName );
      if ( ErrorHandler::NoError != mdl.errorCode() ) { throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage(); }

      mbapi::MapsManager & mpMgr = mdl.mapsManager( );

      if ( !percent1GridFiledName.empty() )
      {
         mbapi::MapsManager::MapID mFirstID = mpMgr.findID( percent1GridFiledName );

         if ( UndefinedIDValue == mFirstID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percent1GridFiledName
               << " defined for the first lithology percentage";
         }

         m_mapNameFirstLithoPercentage = percent1GridFiledName ;
      }

      if ( !percent2GridFiledName.empty() )
      {
         mbapi::MapsManager::MapID mSecondID = mpMgr.findID( percent2GridFiledName );

         if ( UndefinedIDValue == mSecondID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percent2GridFiledName
               << " defined for the second lithology percentage";
         }

         m_mapNameSecondLithoPercentage = percent2GridFiledName ;
      }

      if ( m_mapNameFirstLithoPercentage.empty( ) && m_mapNameSecondLithoPercentage.empty( ) )
      {

         mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();
         // get the layer ID
         mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
         if ( stMgr.errorCode() != ErrorHandler::NoError )
         {
            throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
         }

         // vector to store the lithologies names 
         std::vector<string> lithoNames;
         // vector to store the percentages 
         std::vector<double> lithoPercentages;

         if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, lithoNames, lithoPercentages ) )
         {
            throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
         }

         m_lithoFractions = createLithoFractions( lithoPercentages, m_lithoFractionsInds );

         if ( m_lithoFractions[0] < 0.0 || m_lithoFractions[0] > 100.0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Lithology percentage " << m_lithoFractionsInds[0] <<
               " for layer " << m_layerName << " is out of range [0:100]: " << m_lithoFractions[0] << "\n";
         }

         if ( m_lithoFractions[1] < 0.0 || m_lithoFractions[1] > 1.0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Lithology rest fraction " << m_lithoFractionsInds[1] <<
               " for layer " << m_layerName << " is out of range [0:1]: " << m_lithoFractions[1] << "\n";
         }
      }

      m_name = "LithoFraction(" + m_layerName + ")";

   }

   // Constructor: set the values given by VarPrmLithoFraction
   PrmLithoFraction::PrmLithoFraction( const VarPrmLithoFraction * parent
                                     , const std::string         & name
                                     , const std::string         & layerName
                                     , const std::vector<int>    & lithoFractionsInds
                                     , const std::vector<double> & lithoFractions 
                                     , const std::string         & mapNameFirstLithoPercentage
                                     , const std::string         & mapNameSecondLithoPercentage )
                                     : m_parent( parent )
                                     , m_layerName( layerName )
                                     , m_lithoFractionsInds( lithoFractionsInds )
                                     , m_lithoFractions( lithoFractions )
                                     , m_name( name )
                                     , m_mapNameFirstLithoPercentage( mapNameFirstLithoPercentage )
                                     , m_mapNameSecondLithoPercentage( mapNameSecondLithoPercentage )
   {
      if ( m_name.empty( ) ) m_name = "LithoFraction(" + m_layerName + ")";

      // scalar case
      if ( m_mapNameFirstLithoPercentage.empty( ) && m_mapNameSecondLithoPercentage.empty( ) ) // check the ranges only if a single value is set
      {
         if ( m_lithoFractions[0] < 0.0 || lithoFractions[0] > 100.0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Lithology percentage " << m_lithoFractionsInds[0] <<
               "for layer " << m_layerName << " is out of range [0:100]: " << m_lithoFractions[0] << "\n";
         }

         if ( m_lithoFractions[1] < 0.0 || m_lithoFractions[1] > 1.0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Lithology rest fraction " << m_lithoFractionsInds[1] <<
               "for layer " << m_layerName << " is out of range [0:1]: " << m_lithoFractions[1] << "\n";
         }
      }
   }

   // Update given model with the parameter value
   ErrorHandler::ReturnCode PrmLithoFraction::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
   {
      mbapi::StratigraphyManager & stMgr = caldModel.stratigraphyManager();
      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError )
      {
         throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
      }

      // scalar case
      if ( m_mapNameFirstLithoPercentage.empty( ) && m_mapNameSecondLithoPercentage.empty( ) )
      {
         // vector to store the lithology percentages
         std::vector<double> lithoPercentages;
         // vector to store the lithologies names 
         std::vector<string> lithoNames;

         // get the lithology names
         if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, lithoNames, lithoPercentages ) )
         {
            throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
         }

         // create the new lithology percentages
         lithoPercentages = createLithoPercentages( m_lithoFractions, m_lithoFractionsInds );

         // set the three percentages in the model
         if ( ErrorHandler::NoError != stMgr.setLayerLithologiesList( lid, lithoNames, lithoPercentages ) )
         {
            throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
         }
      }
      else
      {
         stMgr.setLayerLithologiesPercentageMaps( lid, m_mapNameFirstLithoPercentage, m_mapNameSecondLithoPercentage );
      }

      return ErrorHandler::NoError;
   }

   // Get parameter value as an array of doubles
   std::vector<double> PrmLithoFraction::asDoubleArray() const
   {
      return m_lithoFractions;
   }

   // Are two parameters equal?
   bool PrmLithoFraction::operator == ( const Parameter & prm ) const
   {
      const PrmLithoFraction * pp = dynamic_cast<const PrmLithoFraction *>( &prm );
      if ( !pp ) return false;

      const double eps = 1.e-6;

      if ( m_layerName             != pp->m_layerName             ) return false;
      if ( m_lithoFractions.size() != pp->m_lithoFractions.size() ) return false;
      for ( size_t i = 0; i != m_lithoFractions.size(); ++i )
      {
         if ( m_lithoFractionsInds[i] != m_lithoFractionsInds[i] ) return false;
         if ( !NumericFunctions::isEqual( m_lithoFractions[i], pp->m_lithoFractions[i], eps ) ) return false;
      }

      return true;
   }

   // Validate litho fractions stored in the parameter and base case
   std::string PrmLithoFraction::validate( mbapi::Model & mdl )
   {
      std::ostringstream oss;

      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager( );
      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode( ) != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode( ) ) << stMgr.errorMessage( ); }

      // scalar case
      if ( m_mapNameFirstLithoPercentage.empty( ) && m_mapNameSecondLithoPercentage.empty( ) ) 
      {
         const std::vector<double> & prms = asDoubleArray();
         const double eps = 1.e-6;

         const char * colNames[3];
         colNames[0] = "Percent1";
         colNames[1] = "Percent2";
         colNames[2] = "Percent3";

         // vector to store the litho percentages
         std::vector<double> mdlLithoPercentages;
         // vector to store the lithologies names 
         std::vector<string> mdlLithoNames;

         if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, mdlLithoNames, mdlLithoPercentages ) )
         {
            throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
         }

         double sumPercentages = 0;
         for ( size_t i = 0; i != mdlLithoPercentages.size(); ++i )
         {
            sumPercentages += mdlLithoPercentages[i];
         }

         if ( !NumericFunctions::isEqual( sumPercentages, 100.0, eps ) )
         {
            oss << " The sum of the percentages for the layer: " << m_layerName << " do not equal to 100: " << sumPercentages << "\n";
         }

         // vector to store the litho fractions
         std::vector<double> lithoPercentages = createLithoPercentages( m_lithoFractions, m_lithoFractionsInds );

         // check that percentages are set correctly
         for ( size_t i = 0; i < lithoPercentages.size(); ++i )
         {
            if ( mdlLithoNames[i].empty() && lithoPercentages[i] > 0.0 )
            {
               oss << colNames[i] << " for the layer " << m_layerName << " is not zero:" << lithoPercentages[i] << " for the empty lithology name\n";
            }
         }

         for ( size_t i = 0; i < mdlLithoPercentages.size(); ++i )
         {
            if ( !NumericFunctions::isEqual( mdlLithoPercentages[i], lithoPercentages[i], eps ) )
            {
               oss << "Lithology " << colNames[i] << " for the layer " << m_layerName << " in model: " << mdlLithoPercentages[i] <<
                  ", is differ from the parameter value: " << lithoPercentages[i] << "\n";
            }
         }
      }
      else //map case
      {
         // get map and check that was set right
         mbapi::MapsManager & mpMgr = mdl.mapsManager();

         if ( !m_mapNameFirstLithoPercentage.empty() )
         {
            // get the name of the map in the StratIoTbl
            const std::string & percent1GridFiledName = mdl.tableValueAsString( s_stratIoTblName, lid, s_lithoTypePercent1GridFiledName );
            if ( ErrorHandler::NoError != mdl.errorCode( ) )
            {
               oss << mdl.errorMessage( ) << std::endl;
               return oss.str( );
            }

            // check the name is correct
            if ( percent1GridFiledName != m_mapNameFirstLithoPercentage )
            {
               oss << "Map name in project: " << percent1GridFiledName << ", is different from parameter value: " << m_mapNameFirstLithoPercentage;
               return oss.str( );
            }

            // get the name in the GridMapIoTbl
            mbapi::MapsManager::MapID mFirstID = mpMgr.findID( percent1GridFiledName ); // without the HDF exstension

            if ( UndefinedIDValue == mFirstID )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percent1GridFiledName
                  << " defined for the first lithology percentage";
            }

            // check the values are plausible
            double minVal, maxVal;
            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mFirstID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
            }

            if ( minVal < -0.0001 ) oss << "The minimum value in the map " << percent1GridFiledName << " is below the permitted minimum value: " << minVal;
            if ( maxVal > 100.0001 ) oss << "The maximum value in the map " << percent1GridFiledName << " is above the permitted maximum value: " << maxVal;

         }

         if ( !m_mapNameSecondLithoPercentage.empty() )
         {
            // get the name of the map in the StratIoTbl
            const std::string & percent2GridFiledName = mdl.tableValueAsString( s_stratIoTblName, lid, s_lithoTypePercent2GridFiledName );
            if ( ErrorHandler::NoError != mdl.errorCode( ) )
            {
               oss << mdl.errorMessage( ) << std::endl;
               return oss.str( );
            }

            // check the name is correct
            if ( percent2GridFiledName != m_mapNameSecondLithoPercentage )
            {
               oss << "Map name in project: " << percent2GridFiledName << ", is different from parameter value: " << m_mapNameSecondLithoPercentage;
               return oss.str( );
            }

            // get the name in the GridMapIoTbl
            mbapi::MapsManager::MapID mSecondID = mpMgr.findID( percent2GridFiledName );  // without the HDF exstension

            if ( UndefinedIDValue == mSecondID )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percent2GridFiledName
                  << " defined for the second lithology percentage";
            }

            // check the values are plausible
            double minVal, maxVal;
            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mSecondID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
            }

            if ( minVal < -0.0001 ) oss << "The minimum value in the map " << percent2GridFiledName << " is below the permitted minimum value: " << minVal;
            if ( maxVal > 100.0001 ) oss << "The maximum value in the map " << percent2GridFiledName << " is above the permitted maximum value: " << maxVal;
         }
      }

      return oss.str();
   }

   // Save all object data to the given stream, that object could be later reconstructed from saved data
   bool PrmLithoFraction::save( CasaSerializer & sz, unsigned int /* version */ ) const
   {

      bool hasParent = m_parent ? true : false;
      bool ok = sz.save( hasParent, "hasParent" );

      if ( hasParent )
      {
         CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
         ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
      }
      ok = ok ? sz.save( m_name,                     "name"               )       : ok;
      ok = ok ? sz.save( m_layerName,                "layerName"          )       : ok;
      ok = ok ? sz.save( m_lithoFractionsInds,       "lithoFractionsInds" )       : ok;
      ok = ok ? sz.save( m_lithoFractions,           "lithoFractions"     )       : ok;
      ok = ok ? sz.save( m_mapNameFirstLithoPercentage,  "mapNameFirstLithoPercentages" ) : ok;
      ok = ok ? sz.save( m_mapNameSecondLithoPercentage, "mapNameSecondLithoPercentage" ) : ok;

      return ok;
   }

   // Create a new var.parameter instance by deserializing it from the given stream
   PrmLithoFraction::PrmLithoFraction( CasaDeserializer & dz, unsigned int /* version */ )
   {

      CasaDeserializer::ObjRefID parentID;

      bool hasParent;
      bool ok = dz.load( hasParent, "hasParent" );

      if ( hasParent )
      {
         bool ok = dz.load( parentID, "VarParameterID" );
         m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
      }

      ok = ok ? dz.load( m_name,                     "name"               )       : ok;
      ok = ok ? dz.load( m_layerName,                "layerName"          )       : ok;
      ok = ok ? dz.load( m_lithoFractionsInds,       "lithoFractionsInds" )       : ok;
      ok = ok ? dz.load( m_lithoFractions,           "lithoFractions"     )       : ok;
      ok = ok ? dz.load( m_mapNameFirstLithoPercentage,  "mapNameFirstLithoPercentages" ) : ok;
      ok = ok ? dz.load( m_mapNameSecondLithoPercentage, "mapNameSecondLithoPercentage" ) : ok;

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "PrmLithoFraction deserialization unknown error";
      }

   }

} // namespace casa
