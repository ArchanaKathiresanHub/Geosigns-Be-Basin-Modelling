#ifndef _CRUSTAL_THICKNESS_FACTORY_H_
#define _CRUSTAL_THICKNESS_FACTORY_H_

#include "GeoPhysicsObjectFactory.h"

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


class CrustalThicknessCalculatorFactory : public GeoPhysics::ObjectFactory
{
public:
   /// Produce the CrustalThicknessCalculator specific ProjectHandle
   virtual GeoPhysics::ProjectHandle * produceProjectHandle (database::Database * database, 
                                                                        const string & name,  const string & accessMode);
   /// Produce the InterfaceInput
   virtual DataAccess::Interface::CrustalThicknessData * produceCrustalThicknessData (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record);
                                                                            
                                                                   
};


#endif



