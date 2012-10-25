#include "readproperties.h"

#include "PropertiesIoTbl.h"

#include "database.h"
#include "cauldronschema.h"
using namespace database;

#undef __FUNCT__  
#define __FUNCT__ "ReadProjectFile::Read"

#include "FastcauldronSimulator.h"
#include "HydraulicFracturingManager.h"

bool ReadProjectFile::Read ( AppCtx* Basin_Model )
{

   // Try to remove this function and those it calls.

   bool projectFileReadOkay = true;

   if ((Basin_Model == NULL))
      return false;

   PetscPrintf (PETSC_COMM_WORLD, "o Reading Project File... ");

   if (!Basin_Model -> openProject ())
      return false;

   Basin_Model->Create_Reference_DA_for_Io_Maps();

   if ( ! projectFileReadOkay ) {
      return false;
   }

   RunOptionsIoTbl::writeToContext ( Basin_Model );
   
   // Litho and Fluid Types must be set before the Strat IO table.
   projectFileReadOkay = projectFileReadOkay && StratIoTbl::writeToContext ( Basin_Model );
   projectFileReadOkay = projectFileReadOkay && HydraulicFracturingManager::getInstance ().setSelectedFunction ();
   projectFileReadOkay = projectFileReadOkay && BasementIoTbl::writeToContext ( Basin_Model );

   // these have no dependencies.
   SnapshotIoTbl::writeToContext ( Basin_Model );
   FilterTimeIoTbl::writeToContext ( Basin_Model );
   RelatedProjectsIoTbl::writeToContext ( Basin_Model );

   if ( projectFileReadOkay ) {
     PetscPrintf (PETSC_COMM_WORLD, "DONE\n");
   }

   return projectFileReadOkay;
}

