//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmPorosityModel.C
/// @brief This file keeps API implementation for Porosity model parameter handling 


#include "PrmPorosityModel.h"
#include "VarPrmPorosityModel.h"

// CMB API
#include "cmbAPI.h"

#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmPorosityModel::PrmPorosityModel( mbapi::Model & mdl, const char * lithoName )
   : m_parent( 0 )
   , m_lithoName(   lithoName )
   , m_modelType(   UndefinedModel )
   , m_surfPor(     UndefinedDoubleValue )
   , m_compCoef(    UndefinedDoubleValue )
   , m_minPorosity( UndefinedDoubleValue )
   , m_compCoef1(   UndefinedDoubleValue )

{ 
   // construct parameter name
   std::ostringstream oss;
   oss << "PorosityModel(" << m_lithoName << ")";
   m_name = oss.str();

   mbapi::LithologyManager & mgr = mdl.lithologyManager();

   bool isFound = false;

   // go over all lithologies and look for the first lithology with the same name as given
   const std::vector<mbapi::LithologyManager::LithologyID> & lIDs = mgr.lithologiesIDs();
   for ( size_t i = 0; i < lIDs.size() && !isFound; ++i )
   {
      std::string lName = mgr.lithologyName( lIDs[i] );
      
      if ( lName != m_lithoName ) { continue; }

      mbapi::LithologyManager::PorosityModel mdlType;
      std::vector<double> modelPrms;
      if ( ErrorHandler::NoError != mgr.porosityModel( lIDs[i], mdlType, modelPrms ) ) { mdl.moveError( mgr ); return; }
      switch ( mdlType )
      {
         case mbapi::LithologyManager::Exponential:
            m_modelType   = Exponential;
            m_surfPor     = modelPrms[0];
            m_compCoef    = modelPrms[1];
            break;

         case mbapi::LithologyManager::SoilMechanics:
            initSoilMechanicsPorModel( modelPrms );
            break;

         case mbapi::LithologyManager::DoubleExponential:
            m_modelType = DoubleExponential;
            m_surfPor     = modelPrms[0];
            m_compCoef    = modelPrms[1];
            m_minPorosity = modelPrms[2];
            m_compCoef1   = modelPrms[3];
            break;

         default:
            assert(0);
            break;
      }
      isFound = true;
   }
   
   if ( !isFound )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) <<  "Can't find lithology type with name " <<
         m_lithoName << " in lithologies type list";
   }
}

 // Constructor
PrmPorosityModel::PrmPorosityModel( const VarPrmPorosityModel * parent, const char * lithoName, PorosityModelType mdlType, const std::vector<double> & mdlPrms )
   : m_parent(      parent )
   , m_lithoName(   lithoName )
   , m_modelType(   mdlType )
   , m_surfPor(     UndefinedDoubleValue )
   , m_compCoef(    UndefinedDoubleValue )
   , m_minPorosity( UndefinedDoubleValue )
   , m_compCoef1(   UndefinedDoubleValue )
{
  // construct parameter name
   std::ostringstream oss;
   oss << "PorosityModel(" << m_lithoName << ")";
   m_name = oss.str();

   switch( m_modelType )
   {
      case Exponential:
         if ( mdlPrms.size() != 2 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameters number for Exponential porosity model, expected 2, but given: " 
                  << mdlPrms.size();
         }
         m_surfPor  = mdlPrms[0];
         m_compCoef = mdlPrms[1];
         break;

      case SoilMechanics:
         if (      mdlPrms.size() == 1 ) m_clayFraction = mdlPrms[0];          // created from clayFraction 
         else if ( mdlPrms.size() == 2 ) initSoilMechanicsPorModel( mdlPrms ); // created from surfPor && compCoeff
         else  throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue)  << "Wrong parameters number for SoilMechanics porosity model: " << mdlPrms.size();
         break;

      case DoubleExponential:
         if ( mdlPrms.size() != 4 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameters number for Double Exponential porosity model, expected 4, but given: " 
                  << mdlPrms.size();
         }
         m_surfPor     = mdlPrms[0];
         m_minPorosity = mdlPrms[1];
         m_compCoef    = mdlPrms[2];
         m_compCoef1   = mdlPrms[3];
         break;

      default:
         assert( 0 );
         break;
   }
}

// Destructor
PrmPorosityModel::~PrmPorosityModel() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmPorosityModel::setInModel( mbapi::Model & caldModel )
{
   mbapi::LithologyManager & mgr = caldModel.lithologyManager();
   std::vector<double> porModelPrms;

   mbapi::LithologyManager::PorosityModel mdlType = mbapi::LithologyManager::Unknown;
   switch ( m_modelType )
   {
      case Exponential:
         mdlType = mbapi::LithologyManager::Exponential;
         porModelPrms.push_back( m_surfPor );
         porModelPrms.push_back( m_compCoef );
         break;

      case SoilMechanics:
         mdlType = mbapi::LithologyManager::SoilMechanics;
         porModelPrms.push_back( SMcf2sp( m_clayFraction ) );
         porModelPrms.push_back( SMcf2cc( m_clayFraction ) );
         break;

      case DoubleExponential:
         mdlType = mbapi::LithologyManager::DoubleExponential;
         porModelPrms.push_back( m_surfPor );
         porModelPrms.push_back( m_minPorosity );
         porModelPrms.push_back( m_compCoef );
         porModelPrms.push_back( m_compCoef1 );
         break;

      default:
         return caldModel.reportError( ErrorHandler::OutOfRangeValue, "Unsupported porosity model" );
   }

   // go over all lithologies to find the lithology type with given name and set up porosity model
   const std::vector<mbapi::LithologyManager::LithologyID> & ids = mgr.lithologiesIDs();
   for ( size_t i = 0; i < ids.size(); ++i )
   {
      if ( mgr.lithologyName( ids[i] ) == m_lithoName )
      {
         if ( ErrorHandler::NoError != mgr.setPorosityModel( ids[i], mdlType, porModelPrms ) )
         {
            return caldModel.moveError( mgr );
         }
         break;
      }
   }

   return ErrorHandler::NoError;
}

// Validate all porosity model parameters
std::string PrmPorosityModel::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   switch ( m_modelType )
   {
      case Exponential:
         if ( m_surfPor < 0   ) oss << "Surface porosity for lithology " << m_lithoName << " can not be negative: "       << m_surfPor  << std::endl;
         if ( m_surfPor > 100 ) oss << "Surface porosity for lithology " << m_lithoName << " can not be more than 100%: " << m_surfPor  << std::endl;
         if ( m_compCoef < 0  ) oss << "Compaction coef. for lithology " << m_lithoName << " can not be negative: "       << m_compCoef << std::endl;
         if ( m_compCoef > 50 ) oss << "To high value for compaction coef. for lithology " << m_lithoName << ": "         << m_compCoef << std::endl;
         break;

      case SoilMechanics:
         if ( m_clayFraction < 0 ) oss << "Clay fraction for lithology " << m_lithoName << " can not be negative: "       << m_clayFraction * 100 << std::endl;
         if ( m_clayFraction > 1 ) oss << "Clay fraction for lithology " << m_lithoName << " can not be more than 100%: " << m_clayFraction * 100 << std::endl;
         break;

      case DoubleExponential:
         if ( m_surfPor < 0              ) oss << "Surface porosity for lithology " << m_lithoName << " can not be negative: "       << m_surfPor  << std::endl;
         if ( m_surfPor > 100            ) oss << "Surface porosity for lithology " << m_lithoName << " can not be more than 100%: " << m_surfPor  << std::endl;
         if ( m_minPorosity < 0          ) oss << "Minimal porosity for lithology " << m_lithoName << " can not be negative: "       << m_surfPor  << std::endl;
         if ( m_minPorosity > 100        ) oss << "Minimal porosity for lithology " << m_lithoName << " can not be more than 100%: " << m_surfPor  << std::endl;
         if ( m_minPorosity >= m_surfPor ) oss << "Minimal porosity for lithology " << m_lithoName << " can not be more than surface porosity"     << std::endl;

         if ( m_compCoef  < 0  ) oss << "Compaction coef. \"A\" for lithology " << m_lithoName << " can not be negative: " << m_compCoef << std::endl;
         if ( m_compCoef  > 50 ) oss << "To high value for compaction coef. \"A\" for lithology " << m_lithoName << ": "   << m_compCoef << std::endl;
         if ( m_compCoef1 < 0  ) oss << "Compaction coef. \"B\" for lithology " << m_lithoName << " can not be negative: " << m_compCoef << std::endl;
         if ( m_compCoef1 > 50 ) oss << "To high value for compaction coef. \"B\" for lithology " << m_lithoName << ": "   << m_compCoef << std::endl;
         break;
   }

   mbapi::LithologyManager & mgr = caldModel.lithologyManager();

   bool lFound = false;

   // go over all source rock lithologies and check do we have TOC map set for the layer with the same name
   const std::vector<mbapi::LithologyManager::LithologyID> & ids = mgr.lithologiesIDs();

   // check that parameter value in project the same as in this parameter
   for ( size_t i = 0; i < ids.size() && !lFound; ++i )
   {
      if ( mgr.lithologyName( ids[i] ) == m_lithoName )
      {
         lFound = true;
         
         std::vector<double> mdlPrms;
         mbapi::LithologyManager::PorosityModel porModel;

         if ( ErrorHandler::NoError != mgr.porosityModel( ids[i], porModel, mdlPrms ) )
         {
            oss << "Can not get porosity model parameters for lithology: " << m_lithoName << " from project" << std::endl;
         }
         else
         {
            bool samePorModel = true;
            bool sameSurfPor  = true;
            bool sameCC       = true;
            bool sameMinPor   = true;
            bool sameCCA      = true;
            bool sameCCB      = true;

            switch( porModel )
            {
               case mbapi::LithologyManager::Exponential:
                  samePorModel = m_modelType == Exponential ? true : false;
                  if ( samePorModel )
                  {
                     sameSurfPor = NearlyEqual( m_surfPor,  mdlPrms[0], 1.e-4 ); 
                     sameCC      = NearlyEqual( m_compCoef, mdlPrms[1], 1.e-4 );
                  }
                  break;
 
               case mbapi::LithologyManager::SoilMechanics:
                  samePorModel = m_modelType == SoilMechanics ? true : false;
                  if ( samePorModel )
                  {
                     sameSurfPor = NearlyEqual( SMcf2sp( m_clayFraction ), mdlPrms[0], 1.e-4 ); 
                     sameCC      = NearlyEqual( SMcf2cc( m_clayFraction ), mdlPrms[1], 1.e-4 ); 
                  }
                  break;

              case mbapi::LithologyManager::DoubleExponential:
                  samePorModel = m_modelType == SoilMechanics ? true : false;
                  if ( samePorModel )
                  {
                     sameSurfPor = NearlyEqual( m_surfPor,     mdlPrms[0], 1.e-4 );
                     sameMinPor  = NearlyEqual( m_minPorosity, mdlPrms[1], 1.e-4 );
                     sameCCA     = NearlyEqual( m_compCoef,    mdlPrms[2], 1.e-4 );
                     sameCCB     = NearlyEqual( m_compCoef1,   mdlPrms[3], 1.e-4 );
                  }
                  break;

               default:
                  oss << "Unsupported porosity model for lithology: " << m_lithoName << " is defined in project" << std::endl;
                  break;
            }
            if ( !samePorModel ) oss << "Porosity model type for lithology "     << m_lithoName << " defined in project, is different from the parameter value" << std::endl;
            if ( !sameSurfPor  ) oss << "Surface porosity for lithology "        << m_lithoName << " in project: "        << mdlPrms[0] << 
                                        " is differ from the parameter value: "  << m_surfPor << std::endl;
            if ( !sameCC       ) oss << "Compaction coeff. for lithology "       << m_lithoName << " in project: "        << mdlPrms[1] << 
                                        " is differ from the parameter value: "  << m_compCoef << std::endl;
            if ( !sameMinPor   ) oss << "Minimal porosity for lithology "        << m_lithoName << " defined in project " << mdlPrms[2] << 
                                        " is differ from the parameter value "   << m_minPorosity << std::endl;
            if ( !sameCCA      ) oss << "Compaction coeff. \"A\" for lithology " << m_lithoName << " in project: "        << mdlPrms[3] << 
                                        " is differ from the parameter value: "  << m_compCoef << std::endl;
            if ( !sameCCB      ) oss << "Compaction coeff. \"B\" for lithology " << m_lithoName << " in project: "        << mdlPrms[4] << 
                                        " is differ from the parameter value: "  << m_compCoef1 << std::endl;
         }
      }
   }

   if ( !lFound ) oss << "There is no such lithology in the model: " << m_lithoName << std::endl;

   return oss.str();
}

// Get parameter value as an array of doubles
std::vector<double> PrmPorosityModel::asDoubleArray() const
{
   std::vector<double> vals;
   switch ( m_modelType )
   {
      case Exponential:
         vals.push_back( m_surfPor );
         vals.push_back( m_compCoef );
         break;

      case SoilMechanics:
         vals.push_back( m_clayFraction );
         break;

      case DoubleExponential:
         vals.push_back( m_surfPor );
         vals.push_back( m_compCoef );
         vals.push_back( m_minPorosity );
         vals.push_back( m_compCoef1 );
         break;
   }

   return vals;
}


// Are two parameters equal?
bool PrmPorosityModel::operator == ( const Parameter & prm ) const
{
   const PrmPorosityModel * pp = dynamic_cast<const PrmPorosityModel *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_lithoName != pp->m_lithoName || m_modelType != pp->m_modelType ) return false;

   switch ( m_modelType )
   {
      case Exponential:
         if ( !NearlyEqual( m_surfPor,  pp->m_surfPor,  eps ) ) return false;
         if ( !NearlyEqual( m_compCoef, pp->m_compCoef, eps ) ) return false;
         break;

      case SoilMechanics:
         if ( !NearlyEqual( m_clayFraction, pp->m_clayFraction, eps ) ) return false;
         break;

      case DoubleExponential:
         if ( !NearlyEqual( m_surfPor,     pp->m_surfPor,     eps ) ) return false;
         if ( !NearlyEqual( m_compCoef,    pp->m_compCoef,    eps ) ) return false;
         if ( !NearlyEqual( m_minPorosity, pp->m_minPorosity, eps ) ) return false;
         if ( !NearlyEqual( m_compCoef1,   pp->m_compCoef1,   eps ) ) return false;
   }
   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmPorosityModel::save( CasaSerializer & sz, unsigned int version ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,                        "name"             ) : ok;
   ok = ok ? sz.save( m_lithoName,                   "LithologyName"    ) : ok;
   ok = ok ? sz.save( static_cast<int>(m_modelType), "ModelType"        ) : ok;
   ok = ok ? sz.save( m_surfPor,                     "SurfacePororsity" ) : ok;
   ok = ok ? sz.save( m_compCoef,                    "CompactionCoeff"  ) : ok;
   ok = ok ? sz.save( m_minPorosity,                 "MinimalPorosity"  ) : ok;
   ok = ok ? sz.save( m_compCoef1,                   "CompactionCoeffB" ) : ok;
   ok = ok ? sz.save( m_clayFraction,                "ClayFraction"     ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmPorosityModel::PrmPorosityModel( CasaDeserializer & dz, unsigned int objVer )
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
   if ( ok ) m_modelType = static_cast<PrmPorosityModel::PorosityModelType>( mdlTypeSaved );
   
   ok = ok ? dz.load( m_surfPor,      "SurfacePororsity" ) : ok;
   ok = ok ? dz.load( m_compCoef,     "CompactionCoeff"  ) : ok;
   ok = ok ? dz.load( m_minPorosity,  "MinimalPorosity"  ) : ok;
   ok = ok ? dz.load( m_compCoef1,    "CompactionCoeffB" ) : ok;
   ok = ok ? dz.load( m_clayFraction, "ClayFraction"     ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmPorosityModel deserialization unknown error";
   }
}

// check parameters and setup model
void PrmPorosityModel::initSoilMechanicsPorModel( const std::vector<double> & mdlPrms )
{
   m_modelType = SoilMechanics;

   if ( mdlPrms.size() != 2 )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) <<  "Wrong parameters number for Soil Mechanics porosity model " <<
         ", expected 2, but were given: "  << mdlPrms.size();
   }

   double surfPor = mdlPrms[0];
   double cc      = mdlPrms[1];

   if ( !IsValueUndefined( surfPor ) && IsValueUndefined( cc ) )
   {
      m_clayFraction = SMsp2cf( surfPor ); // calculate clay fraction from surface porosity
      if ( m_clayFraction < 0.0 || m_clayFraction > 1.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
            "Soil mechanics porosity model: given surface porosity: " << surfPor << " out of range value: [" <<
            SMcf2sp( 0.0 ) << ":" << SMcf2sp(1.0) << "]";
      }
   }
   else if ( IsValueUndefined( surfPor ) && !IsValueUndefined( cc ) )
   {
      m_clayFraction = SMcc2cf( cc ); // calculate clay fraction from compaction coefficient
      if ( m_clayFraction < 0.0 || m_clayFraction > 1.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
            "Soil mechanics porosity model: given compaction coeff.: " << cc << " out of range value: [" <<
            SMcf2cc( 0.0 ) << ":" << SMcf2cc(1.0) << "]";
      }
   }
   else if ( !IsValueUndefined( surfPor ) && !IsValueUndefined( cc ) )
   {
      m_clayFraction = SMsp2cf( surfPor );
      if ( !NearlyEqual( m_clayFraction, SMcc2cf( cc ), 1.e-4 ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
            "Lithology: " << m_lithoName <<
            ". Soil Mechanics porosity model - inconsistent surface porosity and compaction coefficient. " <<
            "Expected compaction coefficient: " << SMcf2cc( m_clayFraction ) << ", but given: " << cc;
      }

      if ( m_clayFraction < 0.0 || m_clayFraction > 1.0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
            "Soil mechanics porosity model: given surface porosity: " << surfPor << " out of range value: [" <<
            SMcf2sp( 0.0 ) << ":" << SMcf2sp(1.0) << "]";
      }
   }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Lithology: " << m_lithoName << 
         ". Wrong parameters for Soil Mechanics porosity model.";
   }
}
 
double SMcc2sp( double cc )
{
   double clayFraction = SMcc2cf( cc );
   return SMcf2sp( clayFraction );
}

double SMsp2cc( double surfPor )
{
   double clayFraction = SMsp2cf( surfPor );
   return SMcf2cc( clayFraction );
}

double SMsp2cf( double surfPor )
{
   surfPor *= 0.01; // convert units from % to fraction

   double e100 = surfPor / ( 1.0 - surfPor );
   return ( -1.6867 + std::sqrt( 1.6867 * 1.6867 - 4.0 * 1.9505 * ( 0.3024 - e100 ))) / ( 2.0 * 1.9505 );
}

double SMcc2cf( double cc )
{
   return (-0.2479 + std::sqrt( 0.2479 * 0.2479 - 4.0 * 0.3684 * ( 0.0407 - cc ))) / ( 2.0 * 0.3684 );
}

double SMcf2sp( double clayFrac )
{
   double e100 = 0.3024 + 1.6867 * clayFrac + 1.9505 * clayFrac * clayFrac;
   return 100.0 * e100 / ( 1.0 + e100 );
}

double SMcf2cc( double clayFrac )
{
   return 0.0407 + 0.2479 * clayFrac + 0.3684 * clayFrac * clayFrac;
}

} // namespace casa
