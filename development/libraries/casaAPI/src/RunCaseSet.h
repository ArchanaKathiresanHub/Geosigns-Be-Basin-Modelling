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
/// @brief This file keeps class declaration to manage the set of run cases.

#ifndef CASA_API_RUN_CASE_SET_H
#define CASA_API_RUN_CASE_SET_H

// CASA
#include "CasaSerializer.h"

#include <string>
#include <vector>

// STL
#include <memory>

namespace casa
{
   class RunCase;

   /// @brief RunCaseSet class keep list of casa::RunCase objects and allows to manage them.
   ///        Also it can filter objects in set depending on experiment name.
   class RunCaseSet : public CasaSerializable
   {
   public:
      /// @brief Destructor
      virtual ~RunCaseSet( ) {;}

      /// @brief Get size of the set
      /// @return size of the set
      virtual size_t size() const = 0;

      /// @brief Access to i-th element
      /// @param i position element in the list
      /// @return pointer to RunCase object on success, or null pointer otherwise. User should not delete this object
      virtual std::shared_ptr<RunCase> operator[] ( size_t i ) const = 0;

      /// @brief Another way to access to i-th element. C# doesn't support operator[] through swig
      /// @param i position of the element in the list
      /// @return pointer to RunCase object on success, or null pointer otherwise. User should not delete this object
      RunCase * runCase( size_t i ) const { return (*this)[ i ].get(); }

      /// @brief Filtering run cases which are correspond to the given experiment name. After applying filter, the size() and [] operator
      ///        will return only expName related run cases. Such filtering allows to keep different DoE & MC runs in one container.
      /// @param expName experiment name
      virtual void filterByExperimentName( const std::string & expName ) = 0;

      /// @brief Returns experiment name which was set as a filter
      /// @return filter as a string, or empty string if it wasn't set
      virtual std::string filter() const = 0;

      /// @brief Get all experiment names for this case set as an array
      /// @return list of experiment names as an array
      virtual std::vector< std::string > experimentNames() const = 0;

      /// @brief Is set empty
      /// @return true if set is empty, false otherwise
      virtual bool empty() const = 0;

      /// @brief Collect completed cases for given DoEs name list
      /// @param doeList list of DoE names
      /// @return array of completed cases for given DoEs
      virtual std::vector<const RunCase*> collectCompletedCases( const std::vector<std::string> & doeList ) = 0;


   protected:
      RunCaseSet() { ; }
   };

}

#endif // CASA_API_RUN_CASE_SET_H
