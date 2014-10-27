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

#include "VarPrmCategorical.h"
#include "VarPrmContinuous.h"
#include "VarPrmDiscrete.h"

#include <vector>

namespace casa
{
// Variable parameters set manager. It keeps a unique set of variable parameters
class VarSpaceImpl : public VarSpace
{
public:  
   // VarSpace interface implementation
   
   // Add a new categorical parameter
   virtual ErrorHandler::ReturnCode addParameter( VarPrmCategorical * prm )
   {
      if ( prm ) { m_catPrms.push_back( prm ); }
      else       { return reportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" ); }
      return NoError;
   }

   // Add a new continuous parameter
   virtual ErrorHandler::ReturnCode addParameter( VarPrmContinuous * prm )
   {
      if ( prm ) { m_cntPrms.push_back( prm ); }
      else       { return reportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" ); }
      return NoError;
   }

   // Add a new discrete parameter
   virtual ErrorHandler::ReturnCode addParameter( VarPrmDiscrete * prm )
   {
      if ( prm ) { m_disPrms.push_back( prm ); }
      else       { return reportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" ); }
      return NoError;
   }

   // Get number of variable parameters defined in VarSpace
   virtual size_t size() const { return m_catPrms.size() + m_cntPrms.size() + m_disPrms.size(); } 

   // Get number of continuous parameters defined in VarSpace
   virtual size_t numberOfContPrms() const { return m_cntPrms.size(); } 

   // Get number of categorical parameters defined in VarSpace
   virtual size_t numberOfCategPrms() const { return m_catPrms.size(); } 
 
   // Get number of discrete parameters defined in VarSpace
   virtual size_t numberOfDiscrPrms() const { return m_disPrms.size(); }
 
   // Implementation part
   
   // Constructor/Destructor
   VarSpaceImpl() { ; }
   virtual ~VarSpaceImpl()
   {
      for ( size_t i = 0; i < m_catPrms.size(); ++i ) delete m_catPrms[i];
      for ( size_t i = 0; i < m_disPrms.size(); ++i ) delete m_disPrms[i];
      for ( size_t i = 0; i < m_cntPrms.size(); ++i ) delete m_cntPrms[i];
      m_catPrms.clear();
      m_disPrms.clear();
      m_cntPrms.clear();
   }

   // Get i-th continuous parameter from the list
   virtual const VarPrmContinuous * continuousParameter( size_t i ) const { return i < m_cntPrms.size() ? m_cntPrms[ i ] : NULL; }

   // Get i-th categorical parameter from the list
   virtual const VarPrmCategorical * categoricalParameter( size_t i ) const { return i < m_catPrms.size() ? m_catPrms[ i ] : NULL; }

   // Get i-th discrete parameter from the list
   virtual const VarPrmDiscrete * discreteParameter( size_t i ) const { return i < m_disPrms.size() ? m_disPrms[ i ] : NULL; }

private:
   VarSpaceImpl( const VarSpaceImpl & );
   VarSpaceImpl & operator = ( const VarSpaceImpl & );

   std::vector< VarPrmCategorical*> m_catPrms; // set of categorical variable parameters
   std::vector< VarPrmDiscrete*>    m_disPrms; // set of discrete variable parameters
   std::vector< VarPrmContinuous*>  m_cntPrms; // set of continuous variable parameters
};

}

#endif // CASA_API_VAR_SPACE_IMPL_H
