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
   static const char * s_projectIoTblName = "StratIoTbl";
   static const char * s_percent1 = "Percent1";
   static const char * s_percent2 = "Percent2";
   static const char * s_percent3 = "Percent3";

   std::vector<double> PrmLithoFraction::createLithoPercentages( const std::vector<double> & lithoFractions, const std::vector<int> & lithoFractionInds )
   {
      const int numPercentages = 3;
      std::vector<double> lithoPercentages( numPercentages );

      //the first lithofraction is always a total percentage ranging from 0.0 to 100.0
      lithoPercentages[lithoFractionInds[0]] = lithoFractions[0];
      if ( lithoFractions[0] < 0.0 || lithoFractions[0] > 100.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The percentage of the lithology " << lithoFractionInds[0] << " is not valid : " << lithoFractions[0];
      }

      double percentage;
      if ( lithoFractions.size() == 1 )
      {
         //only one lithofraction is defined, therfore devide the remaining part equally among the remaining lithologies
         percentage = ( 100.0 - lithoFractions[0] ) / 2.0;
         for ( int i = 0; i != numPercentages; ++i )
         {
            if ( i != lithoFractionInds[0] ) lithoPercentages[i] = percentage;
         }
      }

      if ( lithoFractions.size() == 2 )
      {
         if ( lithoFractions[1] < 0.0 || lithoFractions[1] > 1.0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The ratio of the lithology " << lithoFractionInds[1] << " is not valid : " << lithoFractions[1];
         }

         //the second lithofraction defines the fraction of what is left. it is varing from 0 to 1. 
         percentage = lithoFractions[1] * ( 100.0 - lithoFractions[0] );
         lithoPercentages[lithoFractionInds[1]] = percentage;
         percentage = 100.0 - lithoFractions[0] - percentage;
         for ( int i = 0; i != numPercentages; ++i )
         {
            if ( i != lithoFractionInds[0] && i != lithoFractionInds[1] )
            {
               if ( percentage < 0.0 || percentage >100.0 ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The percentage of the lithology " << i << " is not valid : " << percentage;
               lithoPercentages[i] = percentage;
            }
         }
      }

      return lithoPercentages;
   }

   std::vector<double> PrmLithoFraction::createLithoFractions( const std::vector<double> & lithoPercentages, const std::vector<int> & lithoFractionInds )
   {
      std::vector< double> lithoFractions;
      if ( lithoPercentages[lithoFractionInds[0]] < 0.0 || lithoPercentages[lithoFractionInds[0]] >100.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "The percentage of the lithology " << lithoFractionInds[0] << " is not valid : " << lithoPercentages[lithoFractionInds[0]];
      }

      lithoFractions.push_back( lithoPercentages[lithoFractionInds[0]] );
      if ( lithoFractionInds.size() == 2 )
      {
         if ( lithoPercentages[lithoFractionInds[0]] == 100.0 )
         {
            lithoFractions.push_back( 0 );
         }
         else
         {
            lithoFractions.push_back( lithoPercentages[lithoFractionInds[1]] / ( 100.0 - lithoPercentages[lithoFractionInds[0]] ) );
         }
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

      std::ostringstream oss;
      oss << "LithoFraction(" << m_layerName << ")";
      m_name = oss.str();
   }

   // Constructor: set the values given by VarPrmLithoFraction
   PrmLithoFraction::PrmLithoFraction( const VarPrmLithoFraction * parent, const std::string & name, const std::string & layerName, const std::vector<int> & lithoFractionsInds, const std::vector<double> & lithoFractions )
      : m_parent( parent )
      , m_layerName( layerName )
      , m_lithoFractionsInds( lithoFractionsInds )
      , m_lithoFractions( lithoFractions )
      , m_name( name )
   {
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

      // vector to store the lithology percentages
      std::vector<double> lithoPercentages;
      // vector to store the lithologies names 
      std::vector<string> lithoNames;

      // get the lithology names
      if ( ErrorHandler::NoError != stMgr.layerLithologiesList( lid, lithoNames, lithoPercentages ) )
      {
         stMgr.layerLithologiesList( lid, lithoNames, lithoPercentages );
      }

      // create the new lithology percentages
      lithoPercentages = createLithoPercentages( m_lithoFractions, m_lithoFractionsInds );

      // set the three percentages in the model
      if ( ErrorHandler::NoError != stMgr.setLayerLithologiesList( lid, lithoNames, lithoPercentages ) )
      {
         throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
      }

      return ErrorHandler::NoError;
   }

   // Get parameter value as an array of doubles
   std::vector<double> PrmLithoFraction::asDoubleArray() const
   {
      std::vector<double> vals;
      for ( size_t i = 0; i != m_lithoFractions.size(); ++i )
      {
         vals.push_back( m_lithoFractions[i] );
      }
      return vals;
   }

   // Are two parameters equal?
   bool PrmLithoFraction::operator == ( const Parameter & prm ) const
   {
      const PrmLithoFraction * pp = dynamic_cast<const PrmLithoFraction *>( &prm );
      if ( !pp ) return false;

      const double eps = 1.e-6;

      if ( m_name != pp->m_name ) return false;
      if ( m_layerName != pp->m_layerName ) return false;
      if ( m_lithoFractions.size() != pp->m_lithoFractions.size() ) return false;
      for ( size_t i = 0; i != m_lithoFractions.size(); ++i )
      {
         if ( !NumericFunctions::isEqual( m_lithoFractions[i], pp->m_lithoFractions[i], eps ) ) return false;
      }

      return true;
   }

   // Validate litho fractions stored in the parameter and base case
   std::string PrmLithoFraction::validate( mbapi::Model & caldModel )
   {
      std::ostringstream oss;
      const std::vector<double> & prms = asDoubleArray();
      std::vector<const char *>          colNames( 3 );
      const double eps = 1.e-6;

      colNames[0] = s_percent1;
      colNames[1] = s_percent2;
      colNames[2] = s_percent3;

      mbapi::StratigraphyManager & stMgr = caldModel.stratigraphyManager();

      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError )
      {
         throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
      }
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
         sumPercentages += prms[i];
      }
      if ( !NumericFunctions::isEqual( sumPercentages, 100.0, eps ) ) { oss << " The sum of the percentages do not sum up to 100 : " << sumPercentages << "\n"; }


      // vector to store the litho fractions
      std::vector<double> lithoFractions = createLithoFractions( mdlLithoPercentages, m_lithoFractionsInds );

      for ( size_t i = 0; i < m_lithoFractionsInds.size(); ++i )
      {
         if ( i == 1 && ( lithoFractions[i] < 0.0 || lithoFractions[i] > 100.0 ) ) { oss << colNames[m_lithoFractionsInds[i]] << " is not acceptable: " << lithoFractions[i] << "\n"; }
         if ( i == 2 && ( lithoFractions[i] < 0.0 || lithoFractions[i] > 1.0   ) ) { oss << colNames[m_lithoFractionsInds[i]] << " is not acceptable: " << lithoFractions[i] << "\n"; }
         if ( !NumericFunctions::isEqual( lithoFractions[i], m_lithoFractions[i], eps ) ) { oss << "The lithofraction in the model (" << lithoFractions[i] << ") is differ from a parameter value (" << m_lithoFractions[i] << ")\n"; }
      }

      return oss.str();
   }

   // Save all object data to the given stream, that object could be later reconstructed from saved data
   bool PrmLithoFraction::save( CasaSerializer & sz, unsigned int /* version */ ) const
   {

      bool hasParent = m_parent ? true : false;
      bool ok = sz.save( hasParent, "hasParent" );

      std::vector<int>     m_lithoFractionsInds;
      std::vector<double>  m_lithoFractions;

      if ( hasParent )
      {
         CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
         ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
      }
      ok = ok ? sz.save( m_name, "name" ) : ok;
      ok = ok ? sz.save( m_layerName, "layerName" ) : ok;
      ok = ok ? sz.save( m_lithoFractionsInds, "lithoFractionsInds" ) : ok;
      ok = ok ? sz.save( m_lithoFractions, "lithoFractions" ) : ok;

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

      ok = ok ? dz.load( m_name, "name" ) : ok;
      ok = ok ? dz.load( m_layerName, "layerName" ) : ok;
      ok = ok ? dz.load( m_lithoFractionsInds, "lithoFractionsInds" ) : ok;
      ok = ok ? dz.load( m_lithoFractions, "lithoFractions" ) : ok;

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "PrmLithoFraction deserialization unknown error";
      }

   }

} // namespace casa
