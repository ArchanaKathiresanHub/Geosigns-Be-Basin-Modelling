//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file LithologyManagerImpl.C
/// @brief This file keeps API implementation for manipulating lithologies in Cauldron model

#include "LithologyManagerImpl.h"

#include <exception>
#include <string>

namespace mbapi
{

// Constructor
LithologyManagerImpl::LithologyManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
}

// Copy operator
LithologyManagerImpl & LithologyManagerImpl::operator = ( const LithologyManagerImpl & otherLythMgr )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Get list of lithologies in the model
std::vector<LithologyManager::LithologyID> LithologyManagerImpl::getLithologiesID() const
{
   throw std::runtime_error( "Not implemented yet" );
   return std::vector<LithologyID>();
}

// Create new lithology
LithologyManager::LithologyID LithologyManagerImpl::createNewLithology()
{
   throw std::runtime_error( "Not implemented yet" );
}


// Get lithology name
ErrorHandler::ReturnCode LithologyManagerImpl::getLithologyName( LithologyID id         // [in]  lithology ID
                                                               , std::string & lythName // [out] on succes - lithology name, or empty string otherwise
                                                               )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

// Get lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::getPorosityModel( LithologyID id           // [in] lithology ID
                                                               , PorosityModel & porModel // [out] porosity model type
                                                               )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

// Set lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::setPorosityModel( LithologyID id         // [in] lithology ID
                                                               , PorosityModel porModel // [out] new type of porosity model
                                                               )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

// Get surface porosity for Eponential or Soil Mechanic lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::getSurfacePorosity( LithologyID id   // [in] lithology ID
                                                                 , double & surfPor // [out] Surface porosity
                                                                 )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

// Set surface porosity for Eponential or Soil Mechanic lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::setSurfacePorosity( LithologyID id // [in] lithology ID
                                                                 , double surfPor // [in] Surface porosity
                                                                 )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

// Get compaction coefficient for Exponential @f$(c_{ef})@f$ or Soil Mechanic @f$(\beta)@f$ lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::getCompactionCoeff( LithologyID id      // [in] lithology ID
                                                                 , double & compCoeff  // [out] Compaction coefficient valued
                                                                 )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

// Set compaction coefficient for Exponential @f$(c_{ef})@f$ or Soil Mechanic @f$(\beta)@f$ lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::setCompactionCoeff( LithologyID id     // [in] lithology ID
                                                                  , double compCoeff  // [in] The new value for compaction coefficient
                                                                  )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

// Set lithology STP thermal conductivity coefficient
ErrorHandler::ReturnCode LithologyManagerImpl::getSTPThermalConductivityCoeff( LithologyID id        // [in] lithology ID
                                                                             , double & stpThermCond // [out] thermal cond. coeff., or unchanged on error
                                                                             )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}


// Set lithology STP thermal conductivity coefficient
ErrorHandler::ReturnCode LithologyManagerImpl::setSTPThermalConductivityCoeff( LithologyID id      // [in] lithology ID
                                                                             , double stpThermCond // [in] the new value of therm. cond. coeff.
                                                                             )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}


}