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

#include "CrustalThicknessCalculator.h"
#include "PropertyValue.h"

namespace database
{
   class Record;
   class Database;
}

/// @class CrustalThicknessCalculatorFactory The CTC object factory
class CrustalThicknessCalculatorFactory : public DataAccess::Mining::ObjectFactory
{
public:
   /// @brief Produce the CrustalThicknessCalculator specific ProjectHandle
   virtual CrustalThicknessCalculator* produceProjectHandle( database::Database * database,
                                                             const string & name,  const string & accessMode);
   /// @brief Produce the InterfaceInput
   virtual DataAccess::Interface::CrustalThicknessData * produceCrustalThicknessData( DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record );

   /// @brief Produce the fasctc PropertyValue
   virtual Ctc::PropertyValue * producePropertyValue( ProjectHandle * projectHandle, database::Record * record,
                                                      const string & name, const Property * property, const Snapshot * snapshot,
                                                      const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage );
};


#endif



