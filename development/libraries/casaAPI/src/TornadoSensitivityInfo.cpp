//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file TornadoSensitivityInfo.cpp
/// @brief This file keeps implementation of TornadoSensitivityInfo methods

// CASA API
#include "Observable.h"
#include "VarParameter.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"
#include "TornadoSensitivityInfo.h"

// CMB API
#include "UndefinedValues.h"

#include <cmath>

namespace casa
{
   //////////////////////////////////////////////////////////////
   // TornadoSensitivityInfo
   //////////////////////////////////////////////////////////////
   TornadoSensitivityInfo::TornadoSensitivityInfo( const  Observable * obs
                                                 , int    obsSubID
                                                 , double obsRefVal
                                                 , const  std::vector< std::pair<const VarParameter *, int> > & varPrms
                                                 , const  SensitivityData & sensData
                                                 , const  SensitivityData & relSensData
                                                 )
                                                 : m_obs( obs )
                                                 , m_obsSubID( obsSubID )
                                                 , m_refObsValue( obsRefVal )
   {

      for ( size_t i = 0; i < varPrms.size(); ++i )
      {
         const VarParameter * vprm = varPrms[i].first;
         int subID = varPrms[i].second;

         switch ( vprm->variationType() )
         {
            case casa::VarParameter::Continuous:
               {
                  const casa::VarPrmContinuous * cprm = dynamic_cast<const casa::VarPrmContinuous*>( vprm );
                  const std::vector<bool> & selPrms = cprm->selected();

                  if ( selPrms[subID] )  // if parameter not fixed add to tornado
                  {
                     m_vprmPtr.push_back( varPrms[i] );
                     m_sensitivities.push_back( sensData[i] );
                     m_relSensitivities.push_back( relSensData[i] );
                  }
               }
               break;

            case casa::VarParameter::Categorical:
            default:
               m_vprmPtr.push_back( varPrms[i] );
               m_sensitivities.push_back( sensData[i] );
               m_relSensitivities.push_back( relSensData[i] );
               break;
         }
      }
   }

   TornadoSensitivityInfo::TornadoSensitivityInfo( const Observable * obs, int obsSubID, double obsRefVal )
                                                 : m_obs( obs )
                                                 , m_obsSubID( obsSubID )
                                                 , m_refObsValue( obsRefVal )
   { ; }

   void TornadoSensitivityInfo::addSensitivity( double minV, double maxV, const VarParameter * prm, size_t prmSubID )
   {
      if ( !IsValueUndefined( minV ) ) { minV -= m_refObsValue; }
      if ( !IsValueUndefined( maxV ) ) { maxV -= m_refObsValue; }

      if ( prm->variationType() == VarParameter::Continuous )
      {
         const std::vector<bool> & selPrms = dynamic_cast<const VarPrmContinuous*>( prm )->selected();
         if ( selPrms[prmSubID] )
         {
            m_vprmPtr.push_back( std::pair<const VarParameter *, int>( prm, static_cast<int>( prmSubID ) ) );
            m_sensitivities.push_back( std::vector<double>( 2, minV ) );
            m_sensitivities.back()[1] = maxV;
         }
      }
      else if ( prm->variationType() == VarParameter::Categorical )
      {
         if ( dynamic_cast<const VarPrmCategorical *>( prm )->selected()[0] )
         {
            m_vprmPtr.push_back( std::pair<const VarParameter *, int>( prm, static_cast<int>( prmSubID ) ) );
            m_sensitivities.push_back( std::vector<double>( 1, minV ) ); // for categorical only one sensitivity
         }
      }
      else { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Tornado sensitivity: unsupported influential parameter type"; }
   }

   void TornadoSensitivityInfo::addMinMaxSensitivityInRange( double minVal, double maxVal, double minPrmVal, double maxPrmVal )
   {
      if ( m_sensitivities.size() - m_maxSensitivities.size() != 1 )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Consitency broken in parameters sensitivity calculation";
      }

      const VarParameter * vprm = m_vprmPtr.back().first;

      if ( !IsValueUndefined( minVal ) ) { minVal -= m_refObsValue; }
      if ( !IsValueUndefined( maxVal ) ) { maxVal -= m_refObsValue; }

      if ( vprm->variationType() == VarParameter::Continuous )
      {
         m_maxSensitivities.push_back( std::vector<double>( 2, minVal ) );
         m_maxSensitivities.back()[1] = maxVal;

         m_prmValForMaxSensitivities.push_back( std::vector<double>( 2, minPrmVal ) );
         m_prmValForMaxSensitivities.back()[1] = maxPrmVal;
      }
      else if ( vprm->variationType() == VarParameter::Categorical )
      {
         m_maxSensitivities.push_back( std::vector<double>( 1, minVal ) );
         m_prmValForMaxSensitivities.push_back( std::vector<double>( 1, minPrmVal ) );
      }
      else { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Tornado sensitivity: unsupported influential parameter type"; }
   }

   void TornadoSensitivityInfo::calculateAndAddValidRanges( const std::vector<double> & allPrmVals, const std::vector<double> & allObsVals )
   {
      if ( m_sensitivities.size() - m_validSensitivitiesPrmVals.size() != 1 )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Consitency broken in parameters sensitivity calculation";
      }
      m_validSensitivitiesPrmVals.push_back( std::vector<double>() );
      m_validSensitivitiesObsVals.push_back( std::vector<double>() );

      if ( allPrmVals.empty() ) { return; }

      size_t rngBeg = allPrmVals.size();
      size_t rngEnd = allPrmVals.size();

      for ( size_t i = 0; i < allPrmVals.size(); ++i )
      {
         if ( !IsValueUndefined( allObsVals[i] ) )
         {
            if ( rngBeg == allPrmVals.size() ) { rngBeg = i; }
            else                               { rngEnd = i; }
         }
         else
         {
            if ( rngBeg < allPrmVals.size() )
            {
               if ( rngEnd - rngBeg > 0 ) // store valid range
               {
                  m_validSensitivitiesPrmVals.back().push_back( allPrmVals[rngBeg] );
                  m_validSensitivitiesPrmVals.back().push_back( allPrmVals[rngEnd] );
                  m_validSensitivitiesObsVals.back().push_back( allObsVals[rngBeg] );
                  m_validSensitivitiesObsVals.back().push_back( allObsVals[rngEnd] );
               }
               // reset range bounds
               rngBeg = allPrmVals.size();
               rngEnd = allPrmVals.size();
            }
         }
      }
      // last interval, up to the end
      if ( rngBeg < allPrmVals.size() )
      {
         m_validSensitivitiesPrmVals.back().push_back( allPrmVals[rngBeg] );
         m_validSensitivitiesPrmVals.back().push_back( allPrmVals[rngEnd] );
         m_validSensitivitiesObsVals.back().push_back( allObsVals[rngBeg] );
         m_validSensitivitiesObsVals.back().push_back( allObsVals[rngEnd] );
      }
   }

   void TornadoSensitivityInfo::calculateRelativeSensitivities()
   {
      double sumOfAbsSensitivities    = 0.0;
      double sumOfMaxAbsSensitivities = 0.0;

      // first accumulate sensitivities
      for ( size_t i = 0; i < m_sensitivities.size(); ++i ) // min/max on boundaries of IP range
      {
         double smin = IsValueUndefined( m_sensitivities[i].front() ) ? 0.0 : m_sensitivities[i].front();
         double smax = IsValueUndefined( m_sensitivities[i].back()  ) ? 0.0 : m_sensitivities[i].back();
         switch ( m_vprmPtr[i].first->variationType() )
         {
            case VarParameter::Continuous:
               sumOfAbsSensitivities += smin * smax < 0.0 ? ( std::fabs( smin ) + std::fabs( smax ) ) : ( std::max( std::fabs( smin ), std::fabs( smax ) ) );
               break;

            case VarParameter::Categorical: sumOfAbsSensitivities += std::fabs( smin ); break;

            default: throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Unsupported parameter type for tornado calculation";
         }
      }

      for ( size_t i = 0; i < m_maxSensitivities.size(); ++i ) // min/max inside IP range
      {
         double smin = IsValueUndefined( m_maxSensitivities[i].front() ) ? 0.0 : m_maxSensitivities[i].front();
         double smax = IsValueUndefined( m_maxSensitivities[i].back()  ) ? 0.0 : m_maxSensitivities[i].back();
         switch ( m_vprmPtr[i].first->variationType() )
         {
            case VarParameter::Continuous:
               sumOfMaxAbsSensitivities += smin * smax < 0.0 ? ( std::fabs( smin ) + std::fabs( smax ) ) : ( std::max( std::fabs( smin ), std::fabs( smax ) ) );
               break;

            case VarParameter::Categorical: sumOfMaxAbsSensitivities += std::fabs( smin ); break;

            default: throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Unsupported parameter type for tornado calculation";
         }
      }


      // Calculate the relative sensitivities (in %)
      m_relSensitivities.clear();

      if ( sumOfAbsSensitivities > 1e-12 ) // can't calculate relative sensitivities
      {
         for ( size_t i = 0; i < m_sensitivities.size(); ++i )
         {
            double smin = IsValueUndefined( m_sensitivities[i].front() ) ? 0.0 : m_sensitivities[i].front();
            double smax = IsValueUndefined( m_sensitivities[i].back()  ) ? 0.0 : m_sensitivities[i].back();

            m_relSensitivities.push_back( std::vector<double>() );

            switch ( m_vprmPtr[i].first->variationType() )
            {
               case VarParameter::Continuous:
                  m_relSensitivities[i].push_back( 100.0 * smin / sumOfAbsSensitivities );
                  m_relSensitivities[i].push_back( 100.0 * smax / sumOfAbsSensitivities );
                  break;

               case VarParameter::Categorical:
                  m_relSensitivities[i].push_back( 100.0 * smin / sumOfAbsSensitivities );
                  break;

               default: throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Unsupported parameter type for tornado calculation";
            }
         }
      }

      if ( sumOfMaxAbsSensitivities > 1e-12 ) // can't calculate max relative sensitivities
      {
         for ( size_t i = 0; i < m_maxSensitivities.size(); ++i )
         {
            double smin = IsValueUndefined( m_maxSensitivities[i].front() ) ? 0.0 : m_maxSensitivities[i].front();
            double smax = IsValueUndefined( m_maxSensitivities[i].back()  ) ? 0.0 : m_maxSensitivities[i].back();

            m_maxRelSensitivities.push_back( std::vector<double>() );

            switch ( m_vprmPtr[i].first->variationType() )
            {
               case VarParameter::Continuous:
                  m_maxRelSensitivities[i].push_back( 100.0 * smin / sumOfMaxAbsSensitivities );
                  m_maxRelSensitivities[i].push_back( 100.0 * smax / sumOfMaxAbsSensitivities );
                  break;

               case VarParameter::Categorical:
                  m_maxRelSensitivities[i].push_back( 100.0 * smin / sumOfMaxAbsSensitivities );
                  break;

               default: throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Unsupported parameter type for tornado calculation";
            }
         }
      }
   }

   TornadoSensitivityInfo::TornadoSensitivityInfo( const TornadoSensitivityInfo & tsi )
                                                 : m_obs( tsi.observable() )
                                                 , m_obsSubID( tsi.observableSubID() )
                                                 , m_refObsValue( tsi.refObsValue() )
                                                 , m_vprmPtr( tsi.varPrmList() )
                                                 , m_sensitivities( tsi.sensitivities() )
                                                 , m_relSensitivities( tsi.relSensitivities() )
                                                 , m_maxSensitivities( tsi.maxSensitivities() )
                                                 , m_maxRelSensitivities( tsi.maxRelSensitivities() )
                                                 , m_prmValForMaxSensitivities( tsi.prmValsForMaxSensitivities() )
                                                 , m_validSensitivitiesObsVals( tsi.validRangesObsVals() )
                                                 , m_validSensitivitiesPrmVals( tsi.validRangesPrmVals() )
   { ; }

   TornadoSensitivityInfo & TornadoSensitivityInfo::operator = ( const TornadoSensitivityInfo & tsi )
   {
      m_obs                       = tsi.observable();
      m_obsSubID                  = tsi.observableSubID();
      m_refObsValue               = tsi.refObsValue();
      m_vprmPtr                   = tsi.varPrmList();
      m_sensitivities             = tsi.sensitivities();
      m_relSensitivities          = tsi.relSensitivities();
      m_maxSensitivities          = tsi.maxSensitivities();
      m_maxRelSensitivities       = tsi.maxRelSensitivities();
      m_prmValForMaxSensitivities = tsi.prmValsForMaxSensitivities();
      m_validSensitivitiesObsVals = tsi.validRangesObsVals();
      m_validSensitivitiesPrmVals = tsi.validRangesPrmVals();
      return *this;
   }


   double TornadoSensitivityInfo::minAbsSensitivityValue( size_t prmNum ) const
   {
      return m_sensitivities[prmNum].size() == 1 ? refObsValue() :                             // categ. prm
                                                   refObsValue() + m_sensitivities[prmNum][0]; // contin. prm
   }

   double TornadoSensitivityInfo::maxAbsSensitivityValue( size_t prmNum ) const
   {
      return m_sensitivities[prmNum].size() == 1 ? refObsValue() + m_sensitivities[prmNum][0] : // categ. prm
                                                   refObsValue() + m_sensitivities[prmNum][1];  // contin. prm
   }

   double TornadoSensitivityInfo::minRelSensitivityValue( size_t prmNum ) const
   {
      return m_relSensitivities.size()      == 0 ? 0.0 : (
             m_sensitivities[prmNum].size() == 1 ? 101 : // no rel. sensitivity for categ. prm
                                                   m_relSensitivities[prmNum][0] );
   }

   double TornadoSensitivityInfo::maxRelSensitivityValue( size_t prmNum ) const
   {
      return m_relSensitivities.size()      == 0 ? 0.0 : (
             m_sensitivities[prmNum].size() == 1 ? 101 : // no rel. sensitivity for categ. prm
                                                   m_relSensitivities[prmNum][1] );
   }

   std::vector<std::string> TornadoSensitivityInfo::varParametersNameList()
   {
      std::vector<std::string> names;
      for ( size_t i = 0; i < m_vprmPtr.size(); ++i )
      {
         const std::vector<std::string> & vprmNames = m_vprmPtr[i].first->name();
         names.push_back( vprmNames[m_vprmPtr[i].second] );
      }
      return names;
   }

   double TornadoSensitivityInfo::minVarParameterRangeValue( size_t vPrmNum ) const
   {
      const VarParameter * vPrm = m_vprmPtr[vPrmNum].first;
      size_t               vPrmSubID = m_vprmPtr[vPrmNum].second;

      double ret = Utilities::Numerical::IbsNoDataValue;

      if ( vPrm->variationType() == VarParameter::Continuous )
      {
         ret = vPrm->minValue()->asDoubleArray()[vPrmSubID];
      }
      else if ( vPrm->variationType() == VarParameter::Categorical )
      {
         ret = static_cast<double> ( vPrm->minValue()->asInteger() );
      }
      else { throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Tornado: Unsupported IP type"; }

      return ret;
   }

   double TornadoSensitivityInfo::maxVarParameterRangeValue( size_t vPrmNum ) const
   {
      const VarParameter * vPrm = m_vprmPtr[vPrmNum].first;
      size_t               vPrmSubID = m_vprmPtr[vPrmNum].second;

      double ret = Utilities::Numerical::IbsNoDataValue;

      if ( vPrm->variationType() == VarParameter::Continuous )
      {
         ret = vPrm->maxValue()->asDoubleArray()[vPrmSubID];
      }
      else if ( vPrm->variationType() == VarParameter::Categorical )
      {
         ret = static_cast<double> ( vPrm->maxValue()->asInteger() );
      }
      else { throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Tornado: Unsupported IP type"; }

      return ret;
   }
}

