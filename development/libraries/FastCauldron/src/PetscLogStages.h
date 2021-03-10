#ifndef _FASTCAULDRON__PETSCLOGSTAGES__H_
#define _FASTCAULDRNO__PETSCLOGSTAGES__H_

#include <vector>

class PetscLogStages
{
public:
   enum LogStage { 
      PRESSURE_LINEAR_SOLVER, PRESSURE_SYSTEM_ASSEMBLY,
      TEMPERATURE_LINEAR_SOLVER, TEMPERATURE_SYSTEM_ASSEMBLY,
      TEMPERATURE_INITIALISATION_LINEAR_SOLVER, TEMPERATURE_INITIALISATION_SYSTEM_ASSEMBLY
   };


   static PetscLogStages & instance();
   static void push( LogStage );
   static void pop();

private:
   PetscLogStages(); // private constructor, because is singleton object
   std::vector< int > m_stageMap;
};

#endif
