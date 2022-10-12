//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FASTCTC_CRUSTALTHICKNESS_CALCULATORFACTORY_H_
#define _FASTCTC_CRUSTALTHICKNESS_CALCULATORFACTORY_H_

// DataMining library
#include "DataMiningObjectFactory.h"

// TableIO library
#include "ProjectFileHandler.h"
#include "CTCPropertyValue.h"

#include "CrustalThicknessCalculator.h"

// Forward declarations
namespace database
{
   class Record;
   class Database;
}
namespace DataAccess
{
   namespace Interface
   {
      class Property;
      class Snapshot;
      class Reservoir;
      class Formation;
      class Surface;
   }
}

/// @class CrustalThicknessCalculatorFactory The CTC object factory
class CrustalThicknessCalculatorFactory : public DataAccess::Mining::ObjectFactory
{
public:
   /// @brief Produce the CrustalThicknessCalculator specific ProjectHandle
   virtual CrustalThicknessCalculator* produceProjectHandle( database::ProjectFileHandlerPtr database,
                                                             const std::string & name ) const override;

   /// @brief Produce the fasctc PropertyValue
   virtual Ctc::CTCPropertyValue * producePropertyValue(Interface::ProjectHandle& projectHandle,
                                                      database::Record * record,
                                                      const std::string & name,
                                                      const DataAccess::Interface::Property * property,
                                                      const DataAccess::Interface::Snapshot * snapshot,
                                                      const DataAccess::Interface::Reservoir * reservoir,
                                                      const DataAccess::Interface::Formation * formation,
                                                      const DataAccess::Interface::Surface * surface,
                                                      DataAccess::Interface::PropertyStorage storage,
                                                      const std::string & fileName = "") const override;
};

#endif
