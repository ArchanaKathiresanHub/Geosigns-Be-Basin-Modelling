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

   std::vector<double> PrmLithoFraction::createLithoPercentages( const std::vector<double> & lithoFractions, const std::vector<int> & lithoFractionInds )
   {
      //the litho fractions must be 2
      if ( lithoFractions.size() != 2 ) { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The number of litho fractions must be 2 "; }

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

      const int           numPercentages = 3;
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
      // the litho indexes must be 2
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
      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();
      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );

      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();  }

      // vector to store the lithologies
      std::vector<string> lithoNames;
      std::vector<double> lithoPercentages;
      std::vector<string> percMaps;

      if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, lithoNames, lithoPercentages, percMaps ) )
      {
         throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
      }

      mbapi::MapsManager & mpMgr = mdl.mapsManager();

      if ( !percMaps[0].empty() )
      {
         mbapi::MapsManager::MapID mFirstID = mpMgr.findID( percMaps[0] );

         if ( UndefinedIDValue == mFirstID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percMaps[0]
               << " defined for the first lithology percentage";
         }
         m_mapNameFirstLithoPercentage = percMaps[0];
      }

      if ( !percMaps[1].empty() )
      {
         mbapi::MapsManager::MapID mSecondID = mpMgr.findID( percMaps[1] );

         if ( UndefinedIDValue == mSecondID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percMaps[1]
               << " defined for the second lithology percentage";
         }
         m_mapNameSecondLithoPercentage = percMaps[1];
      }

      if ( m_mapNameFirstLithoPercentage.empty() && m_mapNameSecondLithoPercentage.empty() )
      {
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


   // Constructor: read from the model with coordinates values
   PrmLithoFraction::PrmLithoFraction( mbapi::Model & mdl, const std::string & layerName, const std::vector<int> & lithoFractionsInds, const std::vector<double> & coordinates )
      : m_parent( 0 )
      , m_layerName( layerName )
      , m_lithoFractionsInds( lithoFractionsInds )
   {
      if ( coordinates.size() != 2 )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "The size of the coordinate vector cannot be different than 2";
      }
      
      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();
      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );

      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // vector to store the lithologies
      std::vector<string> lithoNames;
      std::vector<double> lithoPercentages;
      std::vector<string> percMaps;

      if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, lithoNames, lithoPercentages, percMaps ) )
      {
         throw ErrorHandler::Exception( stMgr.errorCode( ) ) << stMgr.errorMessage( );
      }

      mbapi::MapsManager & mpMgr = mdl.mapsManager();
      
      // Here we assume to have always 2 maps!
      if ( !percMaps[0].empty( ) && !percMaps[1].empty( ) )
      {
         // first map
         mbapi::MapsManager::MapID mFirstID = mpMgr.findID( percMaps[0] );

         if ( UndefinedIDValue == mFirstID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percMaps[0]
               << " defined for the first lithology percentage";
         }

         // get the value of the map
         double value = mpMgr.mapGetValue( mFirstID, coordinates[0], coordinates[1] );
         if ( UndefinedIDValue == value )
         {
            throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
         }

         // first lithoPercentage
         lithoPercentages[0] = value ;

         // second map
         mbapi::MapsManager::MapID mSecondID = mpMgr.findID( percMaps[1] );

         if ( UndefinedIDValue == mSecondID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percMaps[1]
               << " defined for the second lithology percentage";
         }

         // get the value of the map
         value = mpMgr.mapGetValue( mSecondID, coordinates[0], coordinates[1] );
         if ( UndefinedIDValue == value )
         {
            throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
         }

         // second lithoPercentage
         lithoPercentages[1] = value;
         // third lithoPercentage calculated by difference
         lithoPercentages[2] = 100.0 - lithoPercentages[0] - lithoPercentages[1];

         // since we got the punctual value, delete the maps (needed for later)
         m_mapNameFirstLithoPercentage.clear();
         m_mapNameSecondLithoPercentage.clear();
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
      if ( m_name.empty() ) m_name = "LithoFraction(" + m_layerName + ")";

      // scalar case
      if ( m_mapNameFirstLithoPercentage.empty() && m_mapNameSecondLithoPercentage.empty() ) // check the ranges only if a single value is set
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
      if ( m_mapNameFirstLithoPercentage.empty() && m_mapNameSecondLithoPercentage.empty() )
      {
         // vector to store the lithology
         std::vector<string> lithoNames;
         std::vector<double> lithoPercentages;
         std::vector<string> percMaps;

         // get the lithology names
         if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, lithoNames, lithoPercentages, percMaps ) )
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

      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();
      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // vector to store the litho percentages
      std::vector<string> mdlLithoNames;
      std::vector<double> mdlLithoPercentages;
      std::vector<string> percMaps;

      if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, mdlLithoNames, mdlLithoPercentages, percMaps ) )
      {
         throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
      }
      
      // scalar case
      if ( m_mapNameFirstLithoPercentage.empty() && m_mapNameSecondLithoPercentage.empty() ) 
      {
         const std::vector<double> & prms = asDoubleArray();
         const double eps = 1.e-6;

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
               oss << "The percent " << i << " for the layer " << m_layerName << " is not zero:" << 
                      lithoPercentages[i] << " for the empty lithology name\n";
            }
         }

         for ( size_t i = 0; i < mdlLithoPercentages.size(); ++i )
         {
            if ( !NumericFunctions::isEqual( mdlLithoPercentages[i], lithoPercentages[i], eps ) )
            {
               oss << "Lithology percent " << i << " for the layer " << m_layerName << " in model: " << mdlLithoPercentages[i] <<
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

            // check the name is correct
            if ( percMaps[0] != m_mapNameFirstLithoPercentage )
            {
               oss << "Map name in project: " << percMaps[0] << ", is different from parameter value: " << m_mapNameFirstLithoPercentage;
               return oss.str();
            }

            // get the name in the GridMapIoTbl
            mbapi::MapsManager::MapID mFirstID = mpMgr.findID( percMaps[0] ); // without the HDF exstension

            if ( UndefinedIDValue == mFirstID )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percMaps[0]
                  << " defined for the first lithology percentage";
            }

            // check the values are plausible
            double minVal, maxVal;
            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mFirstID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
            }

            if ( minVal < -0.0001 ) oss << "The minimum value in the map " << percMaps[0] << " is below the permitted minimum value: " << minVal;
            if ( maxVal > 100.0001 ) oss << "The maximum value in the map " << percMaps[0] << " is above the permitted maximum value: " << maxVal;

         }

         if ( !m_mapNameSecondLithoPercentage.empty() )
         {

            // check the name is correct
            if ( percMaps[1] != m_mapNameSecondLithoPercentage )
            {
               oss << "Map name in project: " << percMaps[1] << ", is different from parameter value: " << m_mapNameSecondLithoPercentage;
               return oss.str();
            }

            // get the name in the GridMapIoTbl
            mbapi::MapsManager::MapID mSecondID = mpMgr.findID( percMaps[1] );  // without the HDF exstension

            if ( UndefinedIDValue == mSecondID )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << percMaps[1]
                  << " defined for the second lithology percentage";
            }

            // check the values are plausible
            double minVal, maxVal;
            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mSecondID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
            }

            if ( minVal < -0.0001 ) oss << "The minimum value in the map " << percMaps[1] << " is below the permitted minimum value: " << minVal;
            if ( maxVal > 100.0001 ) oss << "The maximum value in the map " << percMaps[1] << " is above the permitted maximum value: " << maxVal;
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

      ok = ok ? sz.save( m_name,                         "name"                         ) : ok;
      ok = ok ? sz.save( m_layerName,                    "layerName"                    ) : ok;
      ok = ok ? sz.save( m_lithoFractionsInds,           "lithoFractionsInds"           ) : ok;
      ok = ok ? sz.save( m_lithoFractions,               "lithoFractions"               ) : ok;
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

      ok = ok ? dz.load( m_name,               "name"               ) : ok;
      ok = ok ? dz.load( m_layerName,          "layerName"          ) : ok;
      ok = ok ? dz.load( m_lithoFractionsInds, "lithoFractionsInds" ) : ok;
      ok = ok ? dz.load( m_lithoFractions,     "lithoFractions"     ) : ok;
      
      if ( version() > 0 ) 
      {
         ok = ok ? dz.load( m_mapNameFirstLithoPercentage,  "mapNameFirstLithoPercentages" ) : ok;
         ok = ok ? dz.load( m_mapNameSecondLithoPercentage, "mapNameSecondLithoPercentage" ) : ok;
      }

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "PrmLithoFraction deserialization unknown error";
      }
   }
} // namespace casa
