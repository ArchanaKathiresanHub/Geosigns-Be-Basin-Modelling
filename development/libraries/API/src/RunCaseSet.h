//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCaseSet.h
/// @brief This file keeps class declaration to manage the set of RunCases.

#ifndef CASA_API_RUN_CASE_SET_H
#define CASA_API_RUN_CASE_SET_H

#include <string>
#include <vector>

namespace casa
{
   class RunCase;

   /// @brief 
   class RunCaseSet
   {
   public:
      /// @brief Destructor
      virtual ~RunCaseSet( ) {;}

      /// @brief Get size of the set
      /// @return size of the set
      virtual size_t size() const = 0;

      /// @brief Access ot i-th element
      /// @param i position element in the collection
      /// @return pointer to RunCase object on success, or null pointer otherwise. User should not delete this pointer
      virtual const RunCase * operator[] ( size_t i ) const = 0;

      /// @brief Filter RunCases which are correspond given experiment name. After applying filter size() and [] operator\n
      ///        will return only expName related RunCases. Such filter allows to keep different DoE & MC runs in one container.
      /// @param expName experiment name
      virtual void filterByExperimentName( const std::string expName ) = 0;

      /// @brief Get all experiment names for this case set as an array
      /// @return list of experiment names as an array
      virtual std::vector< std::string > experimentNames() = 0;

   protected:
      RunCaseSet( ) { ; }
   };

}

#endif // CASA_API_RUN_CASE_SET_H
