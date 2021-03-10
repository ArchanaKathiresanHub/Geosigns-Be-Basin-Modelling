#ifndef _FASTCAULDRON__RUN_PARAMETERS_H_
#define _FASTCAULDRON__RUN_PARAMETERS_H_

#include "RunParameters.h"

using namespace DataAccess;


class CauldronRunParameters : public Interface::RunParameters {

public :

   CauldronRunParameters ( Interface::ProjectHandle& projectHandle, database::Record * record);

   /// Global switch on chemical compaction.
   ///
   /// This function depends on the project handle (FastcauldronSimulator) being initialised before calling.
   bool getChemicalCompaction () const;

};

#endif // _FASTCAULDRON__RUN_PARAMETERS_H_
