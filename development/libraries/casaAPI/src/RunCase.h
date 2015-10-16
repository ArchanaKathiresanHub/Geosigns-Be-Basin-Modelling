//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunCase.h
/// @brief This file keeps API declaration to keep a single run of Cauldron or a single Monte Carlo point

#ifndef CASA_API_CASE_H
#define CASA_API_CASE_H

#include "CasaSerializer.h"
#include "Parameter.h"
#include "cmbAPI.h"
#include "ObsValue.h"


#include <vector>

namespace casa
{
   /// @brief Class which is devoted to keep parameters and results for a single Cauldron project run or for
   ///        a single point of Monte Carlo simulation results
   class RunCase : public CasaSerializable
   {
   public:
      /// @brief Denote run case status
      typedef enum
      {
         NotSubmitted, ///< Case wasn't submitted for execution
         Scheduled,    ///< Case submitted for execution
         Completed,
         Failed
      } CaseStatus;

      /// @brief Destructor
      virtual ~RunCase() { ; }

      /// @brief Add new parameter to this case. Throw if case already has such type parameter
      /// @param prm shared object pointer to a new parameter 
      virtual void addParameter( SharedParameterPtr prm ) = 0;

      /// @brief Get number of parameters 
      /// @return parameters number
      virtual size_t parametersNumber() const = 0;

      /// @brief Get i-th parameter
      /// @param i position of requested parameter
      /// @return i-th parameter or null pointer if there is no such parameter
      virtual SharedParameterPtr parameter( size_t i ) const = 0;

      /// @brief a Add new observable value to this case. Throw if such type observable already in the list
      /// @param obs observable value pointer
      virtual void addObsValue( ObsValue * obs ) = 0;

      /// @brief Get number of observables defined for this case
      /// @return observables number
      virtual size_t observablesNumber() const = 0;

      /// @brief Get i-th observable value
      /// @param i position of requested observable value
      /// @return i-th observable value or null pointer if there is no such observable
      virtual ObsValue * obsValue( size_t i ) const = 0;

      /// @brief Mutate case to given project file
      /// @param baseCase base case of the scenario which will be mutated to a new case
      /// @param newProjectName the name of the mutated project
      virtual void mutateCaseTo( mbapi::Model & baseCase, const char * newProjectName ) = 0;

      /// @brief Do checking, are all variable parameters case value in their ranges
      /// @return if validation is OK, empty string, otherwise - the list of validation
      ///         failed parameters with theirs values
      virtual std::string validateCase() = 0;

      /// @brief Get state of the case
      /// @return execution case status
      virtual CaseStatus runStatus() const = 0;

      /// @brief Set run state of the case
      /// @param st new run case state
      virtual void setRunStatus( CaseStatus st ) = 0;

      /// @brief Get a model associated with this Case
      /// @return pointer to the model
      virtual mbapi::Model * caseModel() const = 0;

      /// @brief Get full path to the project path (including project file
      ///        name. If this case has no project associated with it, it will return
      ///        null pointer
      /// @return full path to the project file (including project file name) or null pointer if project wasn't defined during mutation.
      virtual const char * projectPath() const = 0;

      /// @brief Compare cases. It is neccessary because DoE generator could return the same cases for different DoE
      /// @param cs RunCase for compare with
      /// @return true if RunCases have the same parameters set, false otherwise
      virtual bool operator == ( const RunCase & cs ) const = 0;

      /// @brief Compare cases taking in account application dependency level. Some cases could have equal
      ///        parameters up to some application in pipeline. To avoid unnecessary runs, same case results could be just copied
      /// @param cs RunCase for compare with
      /// @param upTo position of application in pipeline
      /// @return true if RunCases have the same parameters set up to application dependency level, false otherwise
      virtual bool isEqual( const RunCase &cs, AppPipelineLevel upTo ) const = 0;

      /// @brief Get run case ID - unique case number in the RunCaseSet object
      /// @return run case ID
      virtual size_t id() const = 0;

   protected:
      RunCase() { ; }
   };

}

#endif // CASA_API_CASE_H
