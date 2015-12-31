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

// DataAccess library
#include "Interface/ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ObjectFactory;
      class ProjectHandle;  
   }
}

namespace database
{
   class Record;
   class Database;
}

/// @class CrustalThicknessCalculatorFactory The CTC object factory
class CrustalThicknessCalculatorFactory : public DataAccess::Interface::ObjectFactory
{
public:
   /// @brief Produce the CrustalThicknessCalculator specific ProjectHandle
   virtual DataAccess::Interface::ProjectHandle * produceProjectHandle (database::Database * database, 
                                                                             const string & name,  const string & accessMode);
   /// @brief Produce the InterfaceInput
   virtual DataAccess::Interface::CrustalThicknessData * produceCrustalThicknessData (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record);                                                                    
};


#endif



