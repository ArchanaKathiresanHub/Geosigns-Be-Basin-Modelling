//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmPermeabilityModel.C
/// @brief This file keeps API implementation for Permeability model parameter handling 

// CASA API
#include "PrmPermeabilityModel.h"
#include "VarPrmPermeabilityModel.h"

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

// Constructor
PrmPermeabilityModel::PrmPermeabilityModel( mbapi::Model & mdl, const char * lithoName )
   : m_parent( 0 )
   , m_lithoName(      lithoName )
   , m_modelType(      Unknown )
   , m_anisotCoeff(    1.0 )
   , m_depoPerm(       UndefinedDoubleValue )
   , m_clayPercentage( UndefinedDoubleValue )
   , m_sensitCoeff(    UndefinedDoubleValue )
   , m_recoveryCoeff(  UndefinedDoubleValue )
   , m_mpProfilePos(   0.0 )
{ 
   // construct parameter name
   std::ostringstream oss;
   oss << "PermeabilityModel(" << m_lithoName << ")";
   m_name = oss.str();

   mbapi::LithologyManager & lmgr = mdl.lithologyManager();

   mbapi::LithologyManager::LithologyID lid = lmgr.findID( m_lithoName );
   if ( UndefinedIDValue == lid )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) <<  "Can't find lithology type with name " <<
         m_lithoName << "in lithologies list";
   }

   mbapi::LithologyManager::PermeabilityModel mdlType;
   std::vector<double> modelPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;

   if ( ErrorHandler::NoError != lmgr.permeabilityModel( lid, mdlType, modelPrms, mpPor, mpPerm ) )
   {
      mdl.moveError( lmgr );
      throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
   }

   switch ( mdlType )
   {
      case mbapi::LithologyManager::PermNone:        m_modelType = None;        break;
      case mbapi::LithologyManager::PermImpermeable: m_modelType = Impermeable; break;

      case mbapi::LithologyManager::PermSandstone:
         m_modelType      = Sandstone;
         m_anisotCoeff    = modelPrms[AnisotropicCoeff];
         m_depoPerm       = modelPrms[DepositionalPerm];
         m_clayPercentage = modelPrms[ClayPercentage];
         break;

      case mbapi::LithologyManager::PermMudstone:
         m_modelType     = Mudstone;
         m_anisotCoeff   = modelPrms[AnisotropicCoeff];
         m_depoPerm      = modelPrms[DepositionalPerm];
         m_sensitCoeff   = modelPrms[SensitivityCoeff];
         m_recoveryCoeff = modelPrms[RecoverCoeff];
         break;

      case mbapi::LithologyManager::PermMultipoint:
         m_modelType     = Multipoint;
         m_anisotCoeff   = modelPrms[AnisotropicCoeff];
         m_mpPorosity.assign( mpPor.begin(),  mpPor.end() );
         m_mpPermeab.assign(  mpPerm.begin(), mpPerm.end() );

         if ( m_mpPermeab.size() != m_mpPorosity.size() ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
            "Multipoint permeability vs porosity profile - non matching dimensions of porosity array with dimension of permeability array";
         break;

      default:
         assert(0);
         break;
   }
}

 // Constructor
PrmPermeabilityModel::PrmPermeabilityModel( const VarPrmPermeabilityModel * parent
                                          , const char                    * lithoName
                                          , PermeabilityModelType           mdlType
                                          , const std::vector<double>     & modelPrms
                                          )
   : m_parent(         parent )
   , m_lithoName(      lithoName )
   , m_modelType(      mdlType )
   , m_anisotCoeff(    1.0 )
   , m_depoPerm(       UndefinedDoubleValue )
   , m_clayPercentage( UndefinedDoubleValue )
   , m_sensitCoeff(    UndefinedDoubleValue )
   , m_recoveryCoeff(  UndefinedDoubleValue )
   , m_mpProfilePos(   0.0 )
{
  // construct parameter name
   std::ostringstream oss;
   oss << "PermeabilityModel(" << m_lithoName << ")";
   m_name = oss.str();

   // check model parameters for undefined values
   for ( size_t i = 0; i < modelPrms.size(); ++i )
   {
      if ( IsValueUndefined( modelPrms[i] ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Undefined parameter value for permeability model for lithology: " << lithoName;
      }
   }
   switch( m_modelType )
   {
      case mbapi::LithologyManager::PermNone:        m_modelType = None;        break;
      case mbapi::LithologyManager::PermImpermeable: m_modelType = Impermeable; break;

      case mbapi::LithologyManager::PermSandstone:
         if ( modelPrms.size() != 3 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameters number for Sandstone permeability model for " <<
               m_lithoName << " lithology. Expected 3, but were given: " << modelPrms.size();
         }
         m_modelType      = Sandstone;
         m_anisotCoeff    = modelPrms[AnisotropicCoeff];
         m_depoPerm       = modelPrms[DepositionalPerm];
         m_clayPercentage = modelPrms[ClayPercentage];
         break;

      case mbapi::LithologyManager::PermMudstone:
         if ( modelPrms.size() != 4 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameters number for Mudstone permeability model for " <<
               m_lithoName << " lithology. Expected 4, but were given: " << modelPrms.size();
         }
         m_modelType     = Mudstone;
         m_anisotCoeff   = modelPrms[AnisotropicCoeff];
         m_depoPerm      = modelPrms[DepositionalPerm];
         m_sensitCoeff   = modelPrms[SensitivityCoeff];
         m_recoveryCoeff = modelPrms[RecoverCoeff];
         break;

      case mbapi::LithologyManager::PermMultipoint:
         if ( modelPrms.size() < 6 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameters number for Multipoint permeability model for " <<
               m_lithoName << " lithology. Expected 1 parameter and at least 2 points for profile ";
         }
         m_modelType     = Multipoint;
         m_anisotCoeff   = modelPrms[AnisotropicCoeff];

         m_mpPorosity.resize( static_cast<size_t>( floor( modelPrms[MPProfileNumPoints] + 0.5 ) ) );
         m_mpPermeab.resize( m_mpPorosity.size() );

         // Perm vs porosity profile must be defined as set of (por,perm) values pairs
         for ( size_t i = 0, pos = MPProfileNumPoints+1; static_cast<double>( i ) < modelPrms[MPProfileNumPoints]; ++i, pos += 2 )
         {
            m_mpPorosity[i] = modelPrms[pos];
            m_mpPermeab[i]  = modelPrms[pos+1];
         }
         break;

      default:
         assert(0);
         break;
   }
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmPermeabilityModel::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
{
   std::vector<double> modelPrms;
   
   mbapi::LithologyManager::PermeabilityModel mdlType = mbapi::LithologyManager::PermUnknown;

   switch( m_modelType )
   {
      case None:        mdlType = mbapi::LithologyManager::PermNone;        break;
      case Impermeable: mdlType = mbapi::LithologyManager::PermImpermeable; break; // no any parameters

      case Sandstone:
         mdlType = mbapi::LithologyManager::PermSandstone;
         modelPrms.push_back( m_anisotCoeff );
         modelPrms.push_back( m_depoPerm );
         modelPrms.push_back( m_clayPercentage );
         break;

      case Mudstone:
         mdlType = mbapi::LithologyManager::PermMudstone;
         modelPrms.push_back( m_anisotCoeff );
         modelPrms.push_back( m_depoPerm );
         modelPrms.push_back( m_sensitCoeff );
         modelPrms.push_back( m_recoveryCoeff );
         break;

      case Multipoint:
         mdlType = mbapi::LithologyManager::PermMultipoint;
         modelPrms.push_back( m_anisotCoeff );
         break;
     
      default: return caldModel.reportError( ErrorHandler::OutOfRangeValue, "Unsupported permeability model" );
   }

   // get lithology ID
   mbapi::LithologyManager & lmgr = caldModel.lithologyManager();
   mbapi::LithologyManager::LithologyID lid = lmgr.findID( m_lithoName );
   if ( UndefinedIDValue == lid )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) <<  "Can't find lithology type with name " <<
         m_lithoName << "in lithologies list";
   }

   // set permeability model type and parameters
   if ( ErrorHandler::NoError != lmgr.setPermeabilityModel( lid, mdlType, modelPrms, m_mpPorosity, m_mpPermeab ) )
   {
      return caldModel.moveError( lmgr );
   }

   return ErrorHandler::NoError;
}

// Validate all permeability model parameters
std::string PrmPermeabilityModel::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   switch ( m_modelType )
   {
      case None:
      case Impermeable: break; // nothing to check
      
      case Sandstone:
      case Mudstone:
         if ( m_anisotCoeff < 0 ) oss << "Anisotropy coeff/ for lithology "         << m_lithoName << " can not be negative: "  << m_anisotCoeff << std::endl;
         if ( m_depoPerm    < 0 ) oss << "Depositional permeability for lithology " << m_lithoName << " can not be negative: "  << m_depoPerm    << std::endl;

         if ( Sandstone == m_modelType )
         {
            if ( m_clayPercentage < 0 || m_clayPercentage > 100 )
            {
               oss << "Clay percentage for lithology " << m_lithoName << "is out of range [0:100]: " << m_clayPercentage << std::endl;
            }
         }
         else // Mudstone
         {
         }
         break;

      case Multipoint:
         if ( m_anisotCoeff < 0 ) oss << "Anisotropy coeff/ for lithology "       << m_lithoName << " can not be negative: "  << m_anisotCoeff << std::endl;
         for ( size_t i = 0; i < m_mpPorosity.size(); ++i )
         {
            if ( m_mpPorosity[i] < 0 || m_mpPorosity[i] > 100 )
            {
               oss << "Porosity value for multipoint perm. model for lithology " << m_lithoName << "is out of range [0:100]: " << m_mpPorosity[i] << std::endl;
            }
         }
         break;
   }

   mbapi::LithologyManager & lmgr = caldModel.lithologyManager();

   mbapi::LithologyManager::LithologyID lid = lmgr.findID( m_lithoName );
   if ( UndefinedIDValue == lid )
   {
      oss << "Can't find lithology type with name " << m_lithoName << "in lithologies list";
   }

   mbapi::LithologyManager::PermeabilityModel mdlType;
   std::vector<double> mdlPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;

   if ( ErrorHandler::NoError != lmgr.permeabilityModel( lid, mdlType, mdlPrms, mpPor, mpPerm ) )
   {
      caldModel.moveError( lmgr );
      oss << "Can not get permeability model parameters for lithology: " << m_lithoName << " from project file: " << caldModel.errorMessage() << std::endl;
   }
   else
   {
      bool samePermModel  = true;
      bool sameAnisoCoeff = true;
      bool sameDepoPerm   = true;
      bool sameClayPerc   = true;
      bool sameSensCoeff  = true;
      bool sameResCoeff   = true;
      bool sameMPCurve    = true;

      const double eps = 1.e-4;

      switch ( mdlType )
      {
         case mbapi::LithologyManager::PermNone:        samePermModel = m_modelType == None        ? true : false; break;
         case mbapi::LithologyManager::PermImpermeable: samePermModel = m_modelType == Impermeable ? true : false; break;

         case mbapi::LithologyManager::PermSandstone:
            samePermModel  = m_modelType == Sandstone ? true : false;
            sameAnisoCoeff = NumericFunctions::isEqual( m_anisotCoeff,    mdlPrms[AnisotropicCoeff], eps ); 
            sameDepoPerm   = NumericFunctions::isEqual( m_depoPerm,       mdlPrms[DepositionalPerm], eps );
            sameClayPerc   = NumericFunctions::isEqual( m_clayPercentage, mdlPrms[ClayPercentage],   eps );
            break;

         case mbapi::LithologyManager::PermMudstone:
            samePermModel  = m_modelType == Mudstone ? true : false;
            sameAnisoCoeff = NumericFunctions::isEqual( m_anisotCoeff,    mdlPrms[AnisotropicCoeff], eps ); 
            sameDepoPerm   = NumericFunctions::isEqual( m_depoPerm,       mdlPrms[DepositionalPerm], eps );
            sameSensCoeff  = NumericFunctions::isEqual( m_sensitCoeff,    mdlPrms[SensitivityCoeff], eps );
            sameResCoeff   = NumericFunctions::isEqual( m_recoveryCoeff,  mdlPrms[RecoverCoeff],     eps );
            break;

         case mbapi::LithologyManager::PermMultipoint:
            samePermModel  = m_modelType == Multipoint ? true : false;
            sameAnisoCoeff = NumericFunctions::isEqual( m_anisotCoeff,    mdlPrms[AnisotropicCoeff], eps ); 

            sameMPCurve    = mpPor.size() == m_mpPorosity.size() ? true : false;
            for ( size_t i = 0; i < mpPor.size() && sameMPCurve; ++i ) sameMPCurve &= NumericFunctions::isEqual( mpPor[i], m_mpPorosity[i], eps );
            
            sameMPCurve   &= mpPerm.size() == m_mpPermeab.size() ? true : false;
            for ( size_t i = 0; i < mpPerm.size() && sameMPCurve; ++i ) sameMPCurve &= NumericFunctions::isEqual( mpPerm[i], m_mpPermeab[i], eps );
            
            break;

         default: oss << "Unsupported permeability model for lithology: " << m_lithoName << std::endl; break;
      }  
      if ( !samePermModel  ) oss << "Permeability model type for lithology "              << m_lithoName << " defined in project, is different from the parameter value" << std::endl;
      if ( !sameAnisoCoeff ) oss << "Anisotropy permeability coefficient for lithology "  << m_lithoName << " in project: "        << mdlPrms[0] << 
                                  " is differ from the parameter value: "    << m_anisotCoeff       << std::endl;
      if ( !sameDepoPerm   ) oss << "Depositional permeability value for lithology "      << m_lithoName << " in project: "        << mdlPrms[1] << 
                                  " is differ from the parameter value: "    << m_depoPerm          << std::endl;
      if ( !sameClayPerc   ) oss << "Sandstone clay percentage for lithology "            << m_lithoName << " defined in project " << mdlPrms[2] << 
                                  " is differ from the parameter value "   << m_clayPercentage      << std::endl;
      if ( !sameSensCoeff  ) oss << "Permeability sensitivity coefficient for lithology " << m_lithoName << " in project: "        << mdlPrms[2] << 
                                  " is differ from the parameter value: "  << m_sensitCoeff         << std::endl;
      if ( !sameResCoeff   ) oss << "Permeability recovery coefficient for lithology "    << m_lithoName << " in project: "        << mdlPrms[3] << 
                                  " is differ from the parameter value: "  << m_recoveryCoeff       << std::endl;
      if ( !sameMPCurve    ) oss << "Permeability profile for multipoint model defined in project file is differ from the parameter curve." << std::endl;
   }
   return oss.str();
}

// Get parameter value as an array of doubles
std::vector<double> PrmPermeabilityModel::asDoubleArray() const
{
   std::vector<double> vals;

   switch( m_modelType )
   {
      case None:        break;
      case Impermeable: break; // no any parameters

      case Sandstone:
         vals.resize( 3, UndefinedDoubleValue );
         vals[AnisotropicCoeff] = m_anisotCoeff;
         vals[DepositionalPerm] = m_depoPerm;
         vals[ClayPercentage]   = m_clayPercentage;
         break;

      case Mudstone:
         vals.resize( 4 );
         vals[AnisotropicCoeff] = m_anisotCoeff;
         vals[DepositionalPerm] = m_depoPerm;
         vals[SensitivityCoeff] = m_sensitCoeff;
         vals[RecoverCoeff]     = m_recoveryCoeff;
         break;

      case Multipoint:
         vals.resize( 1 );
         vals[AnisotropicCoeff] = m_anisotCoeff;
         vals.push_back( m_mpProfilePos );
         break;
     
      default: break;
   }

   return vals;
}


// Are two parameters equal?
bool PrmPermeabilityModel::operator == ( const Parameter & prm ) const
{
   const PrmPermeabilityModel * pp = dynamic_cast<const PrmPermeabilityModel *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_lithoName != pp->m_lithoName || m_modelType != pp->m_modelType ) return false;

   switch ( m_modelType )
   {
      case Sandstone:
         if ( !NumericFunctions::isEqual( m_anisotCoeff,    pp->m_anisotCoeff,    eps ) ) return false;
         if ( !NumericFunctions::isEqual( m_depoPerm,       pp->m_depoPerm,       eps ) ) return false;
         if ( !NumericFunctions::isEqual( m_clayPercentage, pp->m_clayPercentage, eps ) ) return false;
         break;

      case Mudstone:
         if ( !NumericFunctions::isEqual( m_anisotCoeff,   pp->m_anisotCoeff,   eps ) ) return false;
         if ( !NumericFunctions::isEqual( m_depoPerm,      pp->m_depoPerm,      eps ) ) return false;
         if ( !NumericFunctions::isEqual( m_sensitCoeff,   pp->m_sensitCoeff,   eps ) ) return false;
         if ( !NumericFunctions::isEqual( m_recoveryCoeff, pp->m_recoveryCoeff, eps ) ) return false;
         break;

      case Multipoint:
         if ( !NumericFunctions::isEqual( m_anisotCoeff,  pp->m_anisotCoeff,  eps ) ) return false;
         if ( !NumericFunctions::isEqual( m_mpProfilePos, pp->m_mpProfilePos, eps ) ) return false;

         if ( m_mpPorosity.size() != pp->m_mpPorosity.size() || m_mpPermeab.size() != pp->m_mpPermeab.size() ) return false;

         for ( size_t i = 0; i < m_mpPorosity.size(); ++i ) 
         {
            if ( !NumericFunctions::isEqual( m_mpPorosity[i], pp->m_mpPorosity[i], eps )  ) return false;
            if ( !NumericFunctions::isEqual( m_mpPermeab[i],  pp->m_mpPermeab[i],  eps )  ) return false;
         }
         break;

      case None:
      case Impermeable:
      default:
         break;
   }
   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmPermeabilityModel::save( CasaSerializer & sz, unsigned int version ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,                        "name"               ) : ok;
   ok = ok ? sz.save( m_lithoName,                   "LithologyName"      ) : ok;
   ok = ok ? sz.save( static_cast<int>(m_modelType), "ModelType"          ) : ok;
   ok = ok ? sz.save( m_anisotCoeff,                 "AnisotCoeff"        ) : ok;
   ok = ok ? sz.save( m_depoPerm,                    "DepositionalPerm"   ) : ok;
   ok = ok ? sz.save( m_clayPercentage,              "PermClayPercentage" ) : ok;
   ok = ok ? sz.save( m_sensitCoeff,                 "PermSensitivCoeff"  ) : ok;
   ok = ok ? sz.save( m_recoveryCoeff,               "PermRecoveryCoeff"  ) : ok;
   ok = ok ? sz.save( m_mpProfilePos,                "MPPermProfilePos"   ) : ok;
   ok = ok ? sz.save( m_mpPorosity,                  "MPPermPorosityVals" ) : ok;
   ok = ok ? sz.save( m_mpPermeab,                   "MPPermPermeabVals"  ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmPermeabilityModel::PrmPermeabilityModel( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name,        "name"             ) : ok;
   ok = ok ? dz.load( m_lithoName,   "LithologyName"    ) : ok;
   
   int mdlTypeSaved;
   ok = ok ? dz.load( mdlTypeSaved,  "ModelType" ) : ok;
   if ( ok ) m_modelType = static_cast<PrmPermeabilityModel::PermeabilityModelType>( mdlTypeSaved );
   
   ok = ok ? dz.load( m_anisotCoeff,    "AnisotCoeff"        ) : ok;
   ok = ok ? dz.load( m_depoPerm,       "DepositionalPerm"   ) : ok;
   ok = ok ? dz.load( m_clayPercentage, "PermClayPercentage" ) : ok;
   ok = ok ? dz.load( m_sensitCoeff,    "PermSensitivCoeff"  ) : ok;
   ok = ok ? dz.load( m_recoveryCoeff,  "PermRecoveryCoeff"  ) : ok;
   ok = ok ? dz.load( m_mpProfilePos,   "MPPermProfilePos"   ) : ok;
   ok = ok ? dz.load( m_mpPorosity,     "MPPermPorosityVals" ) : ok;
   ok = ok ? dz.load( m_mpPermeab,      "MPPermPermeabVals"  ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "PrmPermeabilityModel deserialization unknown error";
   }
}

} // namespace casa
