#ifndef _FASTCAULDRON__RUN_PARAMETERS_H_
#define _FASTCAULDRON__RUN_PARAMETERS_H_

#include "Interface/RunParameters.h"
#include "Interface/Interface.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}

using namespace DataAccess;


class RunParameters : public Interface::RunParameters {

public :

   RunParameters ( Interface::ProjectHandle * projectHandle, database::Record * record);

   /// Global switch on chemical compaction.
   ///
   /// This function depends on the project handle (FastcauldronSimulator) being initialised before calling.
   bool getChemicalCompaction () const;

};

#endif // _FASTCAULDRON__RUN_PARAMETERS_H_
