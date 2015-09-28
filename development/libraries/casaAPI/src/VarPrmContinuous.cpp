//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmContinuous.C
/// @brief This file keeps API definition for handling continuous parameters.

#include "VarPrmContinuous.h"

#include "VarPrmCrustThinning.h"
#include "VarPrmOneCrustThinningEvent.h"
#include "VarPrmTopCrustHeatProduction.h"
#include "VarPrmSourceRockTOC.h"
#include "VarPrmSourceRockHC.h"
#include "VarPrmSourceRockHI.h"
#include "VarPrmSourceRockPreAsphaltStartAct.h"
#include "VarPrmPorosityModel.h"
#include "VarPrmPermeabilityModel.h"
#include "VarPrmLithoSTPThermalCond.h"


#include "VarPrmCategorical.h"

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
         default:       assert( 0 );                                                                          break;
         }
      }
      return devs;
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

   bool VarPrmContinuous::save( CasaSerializer & sz, unsigned int version ) const
   {
      // register var. parameter with serializer to allow all Parameters objects keep reference after deserializtion
      CasaSerializer::ObjRefID obID = sz.ptr2id( this );
      bool ok = sz.save( obID, "ID" );
      
      ok = ok ? sz.save( m_pdf, "prmPDF" ) : ok;

      ok = ok ? sz.save( *(m_baseValue.get()), "baseValue" ) : ok;
      ok = ok ? sz.save( *(m_minValue.get()),  "minValue" )  : ok;
      ok = ok ? sz.save( *(m_maxValue.get()),  "maxValue" )  : ok;
      
      if ( version >= 6 ) // version of ScenarioAnalysis object
      {
         ok = ok ? sz.save( m_name, "userGivenName" ) : ok;
      }

      // save connection
      if ( version >= 8 ) // version of ScenarioAnalysis object
      {
         std::vector<CasaSerializer::ObjRefID> vecToSave( m_dependsOn.begin(), m_dependsOn.end() );
         ok = ok ? sz.save( vecToSave, "connectedTo" ) : ok;
      }
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
      else if ( ot == "VarPrmTopCrustHeatProduction"       ) { return new VarPrmTopCrustHeatProduction(       dz, vr ); }
      else if ( ot == "VarPrmSourceRockTOC"                ) { return new VarPrmSourceRockTOC(                dz, vr ); }
      else if ( ot == "VarPrmSourceRockHC"                 ) { return new VarPrmSourceRockHC(                 dz, vr ); }
      else if ( ot == "VarPrmSourceRockHI"                 ) { return new VarPrmSourceRockHI(                 dz, vr ); }
      else if ( ot == "VarPrmSourceRockPreAsphaltStartAct" ) { return new VarPrmSourceRockPreAsphaltStartAct( dz, vr ); }
      else if ( ot == "VarPrmPorosityModel"                ) { return new VarPrmPorosityModel(                dz, vr ); }
      else if ( ot == "VarPrmPermeabilityModel"            ) { return new VarPrmPermeabilityModel(            dz, vr ); }
      else if ( ot == "VarPrmLithoSTPThermalCond"          ) { return new VarPrmLithoSTPThermalCond(          dz, vr ); }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "VarPrmDiscrete deserialization error: Unknown type: " << ot;
      }

      return 0;
   }
   
   // Constructor from input stream, implements common part of deserialization for continuous variable parameters
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

      // register observable with deserializer under read ID to allow Parameters objects keep reference after deserializtion
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

      if ( objVer > 0 )
      {  
         ok = ok ? dz.load( m_name, "userGivenName" ) : ok;
      }

      // after restoring connections we do not need to keep connected ojbects ID any more, on 
      // another serialization call m_dependsOn set will be updated
      if ( objVer > 1 )
      {
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
      }
      return ok;
   }
}

