//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file DoEGeneratorImpl.h
/// @brief This file keeps API declaration for the implementation of Design of Experiments generator

#ifndef CASA_API_DOE_GENERATOR_IMPL_H
#define CASA_API_DOE_GENERATOR_IMPL_H

#include "ErrorHandler.h"

#include "DoEGenerator.h"
#include "VarSpace.h"

// SUMLib includes
#include <BaseTypes.h>

// STL
#include <memory>

namespace SUMlib
{
   class Case;
}

namespace casa
{
   class RunCase;

   // Class DoEGenerator allows to generate various types of "Design of Experiments"
   class DoEGeneratorImpl : public DoEGenerator
   {
   public:
      DoEGeneratorImpl( DoEGenerator::DoEAlgorithm algo );
      virtual ~DoEGeneratorImpl();

      // Generate set of cases for DoE
      // return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode generateDoE( const VarSpace & varPrmsSet   // [in]  list of influential parameters
                                                  , RunCaseSet     & rcSet        // [out] list of cases for DoE
                                                  , size_t           runsNum = 0  // [in]  number of runs for DoE (if it is supported)
                                                  , std::string      doeName = "" // [in]  DoE name which can be assigned for further reference
                                                  );

      // Get DoE type
      // return DoE algorithm type
      virtual DoEAlgorithm algorithm() { return m_typeOfDoE; }

      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "DoEGeneratorImpl"; }

      // Serialize object to the given stream
      virtual bool save(CasaSerializer & sz) const;

      // Create a new instance and deserialize it from the given stream
      DoEGeneratorImpl( CasaDeserializer & inStream, const char * objName );

   private:
      DoEAlgorithm                         m_typeOfDoE;

      // Create SUMlib bounds.
      // [in] varSp       VarSpace
      // [out] lowCs      SUMlib lower bounds
      // [out] highCs     SUMlib upper bounds
      // [out] catIndices indexes of categorical parameters
      void createBounds( const VarSpace & varSp, SUMlib::Case & lowCs, SUMlib::Case & highCs, std::vector<SUMlib::IndexList> & catIndices );

      // Set base case
      // [in] varSp       VarSpace
      // [out] baseCs     base case
      void setBaseCase( const VarSpace & varPrmsSet, SUMlib::Case & baseCs );

      // Add case generated by SUMLib to the list of the CASA cases
      // [in] varSp as a parameters factory
      // [out] expSet list of CASA cases which will be extended for the new one
      // [in]  cs SUMLib generated case
      void addCase( const VarSpace & varSp, std::vector<std::shared_ptr<RunCase>> & expSet, const SUMlib::Case & cs );
   };
}
#endif // CASA_API_DOE_GENERATOR_H
