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

#include "CasaDeserializer.h"

#include <vector>

namespace casa
{
// Variable parameters set manager. It keeps a unique set of variable parameters
class VarSpaceImpl : public VarSpace
{
public:  
   // VarSpace interface implementation
   
   // Add a new variable parameter
   virtual ErrorHandler::ReturnCode addParameter( VarParameter * prm );

   // Get number of variable parameters defined in VarSpace
   virtual size_t size() const { return m_prms.size(); } 

   // Get number of continuous parameters defined in VarSpace
   virtual size_t numberOfContPrms() const { return m_cntPrms.size(); } 

   // Get number of categorical parameters defined in VarSpace
   virtual size_t numberOfCategPrms() const { return m_catPrms.size(); } 
 
   // Get number of discrete parameters defined in VarSpace
   virtual size_t numberOfDiscrPrms() const { return m_disPrms.size(); }
 
   // Implementation part
   
   // Constructor/Destructor
   VarSpaceImpl() { ; }

   virtual ~VarSpaceImpl();

   // Get i-th parameter (numeration is first continuous, then discrete and then categorical)
   virtual const VarParameter * parameter( size_t i ) const { return i < m_prms.size() ? m_prms[i] : 0; }

   // Get i-th continuous parameter from the list
   virtual const VarPrmContinuous * continuousParameter( size_t i ) const { return i < m_cntPrms.size() ? m_cntPrms[ i ] : NULL; }

   // Get i-th categorical parameter from the list
   virtual const VarPrmCategorical * categoricalParameter( size_t i ) const { return i < m_catPrms.size() ? m_catPrms[ i ] : NULL; }

   // Get i-th discrete parameter from the list
   virtual const VarPrmDiscrete * discreteParameter( size_t i ) const { return i < m_disPrms.size() ? m_disPrms[ i ] : NULL; }
 
   // Access to i-th element
   // i position element in the collection
   // return pointer to VarParameter object which should not be deleted by the user on success
   virtual VarParameter * operator[] ( size_t i ) const { return i < m_prms.size() ? m_prms[i] : 0; }

   // Serialization / Deserialization

   // version of serialized object representation
   virtual unsigned int version() const { return 0; }

   // Get type name of the serialaizable object, used in deserialization to create object with correct type
   virtual const char * typeName() const { return "VarSpaceImpl"; }

   // Serialize object to the given stream
   virtual bool save( CasaSerializer & sz, unsigned int version ) const;

   // Create a new instance and deserialize it from the given stream
   VarSpaceImpl( CasaDeserializer & inStream, const char * objName );

private:
   VarSpaceImpl( const VarSpaceImpl & );
   VarSpaceImpl & operator = ( const VarSpaceImpl & );

   std::vector< VarParameter *>     m_prms;    // set of all variable parameters

   std::vector< VarPrmCategorical*> m_catPrms; // set of categorical variable parameters
   std::vector< VarPrmDiscrete*>    m_disPrms; // set of discrete variable parameters
   std::vector< VarPrmContinuous*>  m_cntPrms; // set of continuous variable parameters
};

}

#endif // CASA_API_VAR_SPACE_IMPL_H
