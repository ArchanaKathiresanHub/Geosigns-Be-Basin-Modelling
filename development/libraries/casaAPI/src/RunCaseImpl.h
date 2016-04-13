//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file CaseImp.h
/// @brief This file keeps declaration of the implementation part to keep a single run of Cauldron or a single Monte Carlo point

#ifndef CASA_API_CASE_IMPL_H
#define CASA_API_CASE_IMPL_H

#include "CasaDeserializer.h"
#include "cmbAPI.h"
#include "Parameter.h"
#include "RunCase.h"
#include "Observable.h"

#include <vector>
#include <memory>

namespace casa
{
   /// @brief Case class is devoted to keep parameters and results for a single Cauldron project run or for
   ///        a single point of Monte Carlo simulation results
   class RunCaseImpl : public RunCase
   {
   public:
      // Default constructor, used with Monte Carlo cases
      RunCaseImpl();

      virtual ~RunCaseImpl();

      // Get number of parameters 
      // return parameters number
      virtual size_t parametersNumber() const { return m_prmsSet.size(); }

      // Get i-th parameter
      // i position of requested parameter
      // return i-th parameter or null pointer if there is no such parameter
      virtual SharedParameterPtr parameter( size_t i ) const;

      // Add new parameter to the list
      virtual void addParameter( SharedParameterPtr prm );

      // Get number of observables value
      // return observables value number
      virtual size_t observablesNumber() const { return m_results.size(); }

      // Get i-th observable value
      // return i-th observable value or null pointer if there is no such observable
      virtual ObsValue * obsValue( size_t i ) const;

      // Add new observable value to the list
      virtual void addObsValue( ObsValue * obs );

      // Mutate give case to the given project file
      virtual void mutateCaseTo( mbapi::Model & baseCase, const char * newProjectName );

      // Do checking, are all variable parameters case value in their ranges
      // if validation is OK, return empty string. otherwise - the list of validation
      //         failed parameters with theirs values
      virtual std::string validateCase();

      // Get state of the case
      virtual CaseStatus runStatus() const { return m_runState; }

      // Set run state of the case (used by RunManager)
      virtual void setRunStatus( CaseStatus st ) { assert( st >= m_runState ); m_runState = st; }

      // Get a model associated with this Case
      // return pointer to the model
      virtual mbapi::Model * caseModel() const { return m_model.get(); }

      // Load associated with this case project file into mbapi::Model object
      // return loaded model. If any error happened during loading, Model object will contain error description
      mbapi::Model & loadProject();

      // Get full path to the project path (including project file name). If this case has no project associated with 
      // it, it will return null pointer
      // return full path to the project file (including project file name) or null pointer if project wasn't defined during mutation.
      virtual const char * projectPath() const { return m_modelProjectFileName.empty() ? NULL : m_modelProjectFileName.c_str(); }

      // Set full path to the project path (including project file name).
      virtual void setProjectPath( const char * pth ) { m_modelProjectFileName = pth; }

      // Compare cases. It is neccessary because DoE generator could return the same cases for different DoE
      virtual bool operator == ( const RunCase & cs ) const;

      // Compare cases taking in account application dependency level. Some cases could have equal
      // parameters up to some application in pipeline. To avoid unnecessary runs, same case results could be just copied
      virtual bool isEqual( const RunCase &cs, AppPipelineLevel upTo ) const;

      void setCleanDuplicatedLithologies( bool val ) { m_cleanDupLith = val; }
      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 1; }

      // Get type name of the serialaizable object, used in deserialization to create object with correct type
      virtual const char * typeName() const { return "RunCaseImpl"; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      // Create a new instance and deserialize it from the given stream
      RunCaseImpl( CasaDeserializer & inStream, const char * objName );

      /// @brief Get run case ID - unique case number in the RunCaseSet object
      /// @return run case ID
      virtual size_t id() const { return m_id; }

      /// @brief Set run case ID - unique case number in the RunCaseSet object
      /// @param id run case ID
      void setID( size_t id ) { m_id = id; }

   private:
      std::unique_ptr<mbapi::Model>    m_model;                // Mutated model, available after mutateCaseTo call
      std::string                      m_modelProjectFileName; // full path to the project file
      std::vector<SharedParameterPtr>  m_prmsSet;              // list of parameters for this case
      std::vector<ObsValue*>           m_results;              // list of observables values
      CaseStatus                       m_runState;             // Stat of the run case (submitted/completed/failed)

      size_t                           m_id;                   // unique number in RunCaseSet

      bool                             m_cleanDupLith;         // delete or not duplicated lithologies when apply mutations
                                                               // used only for creation of calibrated case
      // disable copy constructor and copy operator
      RunCaseImpl( const RunCaseImpl & );
      RunCase & operator = ( const RunCaseImpl & );
   };
}

#endif // CASA_API_CASE_IMPL_H
