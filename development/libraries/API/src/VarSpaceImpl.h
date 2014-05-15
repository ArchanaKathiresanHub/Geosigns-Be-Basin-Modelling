//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarSpaceImpl.h
/// @brief This file keeps declaration of the implementation part of variable parameters set manager

#ifndef CASA_API_VAR_SPACE_IMPL_H
#define CASA_API_VAR_SPACE_IMPL_H

#include "VarSpace.h"

#include "CategoricalParameter.h"
#include "ContinuousParameter.h"

#include <vector>

namespace casa
{
// Variable parameters set manager. It keeps a unique set of variable parameters
class VarSpaceImpl : public VarSpace
{
public:  
   // VarSpace interface implementation
   
   // Add a new categorical parameter
   virtual ErrorHandler::ReturnCode addParameter( CategoricalParameter * prm )
   {
      return ReportError( ErrorHandler::NotImplementedAPI, "VarSpaceImpl::addParameter() not implemented yet" );
   }

   // Add a new continuous parameter
   virtual ErrorHandler::ReturnCode addParameter( ContinuousParameter * prm )
   {
      if ( prm ) m_cntPrms.push_back( prm );
      else
      {
         return ReportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" );
      }
      return NoError;
   }

   // Get number of variable parameters defined in VarSpace
   virtual size_t size() const { return m_catPrms.size() + m_cntPrms.size(); } 

   // Get number of continuous parameters defined in VarSpace
   virtual size_t numberOfContPrms() const { return m_cntPrms.size(); } 

   // Get number of categorical parameters defined in VarSpace
   virtual size_t numberOfCategPrms() const { return m_catPrms.size(); } 

   // Implementation part
   
   // Constructor/Destructor
   VarSpaceImpl() {;}
   virtual ~VarSpaceImpl() {;}

   // Get continuous parameters list
   const std::vector<ContinuousParameter*>  & continuousParameters() const { return m_cntPrms; }

   // Get categorical parameters list
   const std::vector<CategoricalParameter*> & categoricalParameters() const { return m_catPrms; }

private:
   std::vector<CategoricalParameter*> m_catPrms;
   std::vector<ContinuousParameter*>  m_cntPrms;
};

}

#endif // CASA_API_VAR_SPACE_IMPL_H
