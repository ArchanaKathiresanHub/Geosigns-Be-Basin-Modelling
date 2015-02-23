//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCaseSetImpl.h
/// @brief This file keeps declaration for RunCaseSet API implementation

#ifndef CASA_API_RUN_CASE_SET_IMPL_H
#define CASA_API_RUN_CASE_SET_IMPL_H

#include <map>
#include <memory>
#include <vector>

#include "CasaDeserializer.h"
#include "DoEGenerator.h"
#include "RunCaseSet.h"

namespace casa
{
   // Implementation of RunCaseSet API class
   class RunCaseSetImpl : public RunCaseSet
   {
   public:
      typedef std::map< std::string, std::vector<size_t> > ListOfDoEIndexesSet;

      // Constructor/destructor
      RunCaseSetImpl() { ; }
      virtual ~RunCaseSetImpl();

      // Size of the collection
      virtual size_t size() const;

      // Access to i-th element
      // i position element in the collection
      // return pointer to RunCase object which should not be deleted by the user on success
      virtual RunCase * operator[] ( size_t i ) const;

      // Set filter for experiments by experiment name
      virtual void filterByExperimentName( const std::string & expName );

      // Returns experiment name which was set as a filter
      // return filter as a string, or empty string if it wasn't set
      virtual std::string filter() const { return m_filter; }

      // Get all experiment names for this case set as an array
      virtual std::vector< std::string > experimentNames() const;

      // Is set empty
      // return true if set is empty, false otherwise
      virtual bool empty() const { return size() == 0 ? true : false; }

      // Move a new Cases to the collection and clear array 
      void addNewCases( std::vector<RunCase*> & newCases, const std::string & expName );

      // collect completed cases for given DoEs name list
      virtual std::vector<const RunCase*> collectCompletedCases( const std::vector<std::string> & doeList );

      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 0; }

      // Get type name of the serialaizable object, used in deserialization to create object with correct type
      virtual const char * typeName() const { return "RunCaseSetImpl"; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      // Create a new instance and deserialize it from the given stream
      RunCaseSetImpl( CasaDeserializer & inStream, const char * objName );

   protected:
      std::vector< RunCase* > m_caseSet;    // keeps all RunCases
      
      ListOfDoEIndexesSet     m_expSet;     // keeps set of indexes for each experiment
      std::string             m_filter;     // keeps filter value
      std::vector<size_t>     m_expIndSet;  // keeps index set for given filter
   };
}

#endif // CASA_API_RUN_CASE_SET_IMPL_H
