#include "PetscLogStages.h"

#include <cassert> 
#include <petscsys.h>

// disable warnings emitted by 'assert' macros, because the string literals are constant sub-expressions
#pragma warning (disable : 279 )


PetscLogStages & 
PetscLogStages
   :: instance()
{
   static PetscLogStages object;
   return object;
}

PetscLogStages
   :: PetscLogStages()
   : m_stageMap()
{
   PetscErrorCode pe = 0;
   int stageId = 0;
   pe = PetscLogStageRegister("Pressure - Linear solver", & stageId );
   assert( pe == 0 && "Adding petsc log stage");
   m_stageMap.push_back( stageId );

   pe = PetscLogStageRegister("Pressure - System assembly", &stageId);
   assert( pe == 0 && "Adding petsc log stage");
   m_stageMap.push_back( stageId );

   pe = PetscLogStageRegister("Temperature - Linear solver", &stageId);
   assert( pe == 0 && "Adding petsc log stage");
   m_stageMap.push_back( stageId );

   pe = PetscLogStageRegister("Temperature - System assembly", &stageId);
   assert( pe == 0 && "Adding petsc log stage");
   m_stageMap.push_back( stageId);

   pe = PetscLogStageRegister("Initial Temperature - Linear solver", &stageId);
   assert( pe == 0 && "Adding petsc log stage");
   m_stageMap.push_back( stageId );

   pe = PetscLogStageRegister("Initial Temperature - System assembly", &stageId);
   assert( pe == 0 && "Adding petsc log stage");
   m_stageMap.push_back( stageId);
}

void
PetscLogStages
  :: push( LogStage stage)
{
   PetscLogStages & obj = instance();

   // assert stage is within range
   assert( stage >= 0 && "Checking petsc log stage range");
   assert( stage < obj.m_stageMap.size() && "Checking petsc log stage range" );

   int pe = 0;
   pe = PetscLogStagePush( obj.m_stageMap[stage] );
   assert( pe == 0 && "Pushing petsc log stage");
}

void
PetscLogStages
  :: pop( )
{
   int pe = 0;
   pe = PetscLogStagePop();
   assert( pe == 0 && "Popping petsc log stage");
}

