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

#include "RunCaseSet.h"
#include "DoEGenerator.h"

namespace casa
{
   // Implementation of RunCaseSet API class
   class RunCaseSetImpl : public RunCaseSet
   {
   public:
      // Constructor/destructor
      RunCaseSetImpl() { ; }
      virtual ~RunCaseSetImpl();

      // Size of the collection
      virtual size_t size() const;

      // Read only access to i-th element
      // i position element in the collection
      // return pointer to RunCase object which should not be deleted by the user on success
      virtual const RunCase * operator[] ( size_t i ) const;

      // Set filter for experiments by experiment name
      virtual void filterByExperimentName( const std::string expName );

      // Get all experiment names for this case set as an array
      virtual std::vector< std::string > experimentNames();

      // Access to i-th element
      RunCase * operator() ( size_t i ) const;

      // Move a new Cases to the collection and clear array 
      void addNewCases( std::vector<RunCase*> & newCases, const std::string & expName );

   protected:
      std::vector< RunCase* >                      m_caseSet;    // keeps all RunCases
      
      std::map< std::string, std::vector<size_t> > m_expSet;     // keeps set of indexes for each experiment
      std::string                                  m_filter;     // keeps filter value
      std::vector<size_t>                          m_expIndSet;  // keeps index set for given filter
   };
}

#endif // CASA_API_RUN_CASE_SET_IMPL_H
