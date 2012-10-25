#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "petscvec.h"
#include "petscda.h"

#include <dlfcn.h>

#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include "Interface/DistributedGridMap.h"
#include "Interface/DistributedGrid.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/SourceRock.h"
#include "Interface/Reservoir.h"
#include "Interface/MobileLayer.h"
#include "Interface/Trap.h"
#include "Interface/Lead.h"
#include "Interface/LeadTrap.h"
#include "Interface/InputValue.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/LithoType.h"
#include "Interface/AttributeValue.h"
#include "Interface/FaultCollection.h"
#include "Interface/Faulting.h"

#include "Interface/ObjectFactory.h"


#include <string>
using namespace std;

using namespace DataAccess;
using namespace Interface;
//using namespace Interface;

namespace database
{
   class Record;
}

using namespace std;

static char help[] = "Test Distributed Data Access \n\n";

#undef __FUNCT__
#define __FUNCT__ "main"

int main (int argc, char ** argv)
{
   int  ierr;
   PetscMPIInt rank;

   string tmpString;

   PetscInitialize (&argc, &argv, (char *) 0, help);
   ierr = MPI_Comm_rank (PETSC_COMM_WORLD, &rank);
   CHKERRQ (ierr);

   ProjectHandle *projectHandle = OpenCauldronProject ("../data/TestFaultHR.project3d", "rw");

   assert (projectHandle);

   projectHandle->asString (tmpString);
   // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

   SnapshotList *snapshotList = projectHandle->getSnapshots ();

   SnapshotList *snapshotList2 = projectHandle->getSnapshots ();

   assert (snapshotList != snapshotList2);
   delete snapshotList2;

   assert (snapshotList->size () == 7);
   assert ((*snapshotList)[3]->getTime () == 13);

   double times[] = { 0, 11, 12, 13, 14, 15, 100 };
   SnapshotList::iterator snapshotIter;
   int i, j, k;

   // PetscPrintf (PETSC_COMM_WORLD, "Input grid distribution\n");
   // projectHandle->getInputGrid ()->printDistributionOn (PETSC_COMM_WORLD);
   // PetscPrintf (PETSC_COMM_WORLD, "Highres Output grid distribution\n");
   // projectHandle->getHighResolutionOutputGrid ()->printDistributionOn (PETSC_COMM_WORLD);
   // PetscPrintf (PETSC_COMM_WORLD, "Lowres Output grid distribution\n");
   // projectHandle->getLowResolutionOutputGrid ()->printDistributionOn (PETSC_COMM_WORLD);

   // PetscPrintf (PETSC_COMM_WORLD, "Snapshots: \n");
   for (snapshotIter = snapshotList->begin (), i = 0; snapshotIter != snapshotList->end (); ++snapshotIter, ++i)
   {
      const Snapshot *snapshot = *snapshotIter;
      snapshot->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      assert (snapshot->getTime () == times[i]);
   }

   delete snapshotList;

   const char *surfaceNames[] = { "H0", "H1", "H2", "H3", "H4", "H5" };
   const char *formationNames[] = { "fH0", "fH1", "fH2", "fH3", "fH4" };

   SurfaceList *surfaceList = projectHandle->getSurfaces ();

   SurfaceList::iterator surfaceIter;

   // PetscPrintf (PETSC_COMM_WORLD, "Surfaces: \n");
   for (surfaceIter = surfaceList->begin (), i = 0; surfaceIter != surfaceList->end (); ++surfaceIter, ++i)
   {
      const Surface *surface = *surfaceIter;
      surface->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      const GridMap * gridMap = surface->getInputDepthMap ();
      assert (gridMap);
      gridMap->retrieveData ();
      // gridMap->printOn (PETSC_COMM_WORLD);
      gridMap->restoreData ();
      // gridMap->release ();

      assert (surface->getName () == surfaceNames[i]);
      if (i > 0)
      {
         assert (surface->getTopFormation ()->getName () == formationNames[i - 1]);
      }
      if (i < 5)
      {
         assert (surface->getBottomFormation ()->getName () == formationNames[i]);
      }
   }
   delete surfaceList;

   const GridMap * gridMapA = projectHandle->findSurface ("H3")->getInputDepthMap ();
   assert (gridMapA);
   gridMapA->retrieveData ();
   PetscPrintf (PETSC_COMM_WORLD, "gridMapA: \n");
   dynamic_cast<const DistributedGridMap * >(gridMapA)->printOn (PETSC_COMM_WORLD);
   gridMapA->restoreData ();

   GridMap * gridMapAA = new DistributedGridMap (0, 0, gridMapA->getGrid (), gridMapA->getUndefinedValue (), gridMapA->getDepth ());
   gridMapA->convertToGridMap (gridMapAA);
   gridMapAA->retrieveData ();
   gridMapAA->setValue (4, 12, gridMapAA->getUndefinedValue ());
   gridMapAA->setValue (1, 13, gridMapAA->getUndefinedValue ());
   PetscPrintf (PETSC_COMM_WORLD, "gridMapAA: \n");
   dynamic_cast<const DistributedGridMap *>(gridMapAA)->printOn (PETSC_COMM_WORLD);
   gridMapAA->restoreData ();

   GridMap * gridMapB = new DistributedGridMap (0, 0, projectHandle->getLowResolutionOutputGrid (), gridMapAA->getUndefinedValue (), gridMapAA->getDepth ());
   gridMapAA->convertToGridMap (gridMapB);
   gridMapB->retrieveData ();
   PetscPrintf (PETSC_COMM_WORLD, "gridMapB(1): \n");
   dynamic_cast<const DistributedGridMap *>(gridMapB)->printOn (PETSC_COMM_WORLD);
   gridMapB->restoreData ();

   gridMapB->retrieveData ();
   gridMapB->setValue (0, 0, gridMapB->getUndefinedValue ());
   gridMapB->setValue (1, 4, gridMapB->getUndefinedValue ());
   PetscPrintf (PETSC_COMM_WORLD, "gridMapB(2): \n");
   dynamic_cast<const DistributedGridMap *>(gridMapB)->printOn (PETSC_COMM_WORLD);
   gridMapB->restoreData ();

   GridMap * gridMapC = new DistributedGridMap (0, 0, projectHandle->getInputGrid (), gridMapB->getUndefinedValue (), gridMapB->getDepth ());
   gridMapB->convertToGridMap (gridMapC);
   gridMapC->retrieveData ();
   PetscPrintf (PETSC_COMM_WORLD, "gridMapC: \n");
   dynamic_cast<const DistributedGridMap *>(gridMapC)->printOn (PETSC_COMM_WORLD);
   gridMapC->restoreData ();


   surfaceList = projectHandle->getSurfaces (projectHandle->findSnapshot (12));

   // PetscPrintf (PETSC_COMM_WORLD, "Surfaces at snapshot 12: \n");
   for (surfaceIter = surfaceList->begin (), i = 0;
	 surfaceIter != surfaceList->end (); ++surfaceIter, ++i)
   {
      const Surface *surface = *surfaceIter;
      surface->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }
   delete surfaceList;

   FormationList *formationList = projectHandle->getFormations ();

   FormationList::iterator formationIter;

   // PetscPrintf (PETSC_COMM_WORLD, "Formations: \n");
   for (formationIter = formationList->begin (), i = 0; formationIter != formationList->end (); ++formationIter, ++i)
   {
      const Formation *formation = *formationIter;
      formation->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      const GridMap * gridMap = formation->getInputThicknessMap ();
      assert (gridMap);
      gridMap->retrieveData ();
      // gridMap->printOn (PETSC_COMM_WORLD);
      gridMap->restoreData ();
      // gridMap->release ();

      const LithoType * lithoType = formation->getLithoType1 ();
      assert (lithoType);
      lithoType->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      gridMap = formation->getLithoType1PercentageMap ();
      assert (gridMap);
      gridMap->retrieveData ();
      // gridMap->printOn (PETSC_COMM_WORLD);
      gridMap->restoreData ();
      // gridMap->release ();

      lithoType = formation->getLithoType2 ();
      if (lithoType)
      {
	 lithoType->asString (tmpString);
	 // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

         gridMap = formation->getLithoType2PercentageMap ();
	 assert (gridMap);
	 gridMap->retrieveData ();
         // gridMap->printOn (PETSC_COMM_WORLD);
	 gridMap->restoreData ();
         // gridMap->release ();
      }

      lithoType = formation->getLithoType3 ();
      if (lithoType)
      {
	 lithoType->asString (tmpString);
	 // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

         gridMap = formation->getLithoType3PercentageMap ();
	 assert (gridMap);
	 gridMap->retrieveData ();
         // gridMap->printOn (PETSC_COMM_WORLD);
	 gridMap->restoreData ();
         // gridMap->release ();
      }

      assert (formation->getName () == formationNames[i]);
      assert (formation->getTopSurface ()->getName () == surfaceNames[i]);
      assert (formation->getBottomSurface ()->getName () == surfaceNames[i + 1]);
   }
   delete formationList;

   formationList = projectHandle->getFormations (projectHandle->findSnapshot (12));
   // PetscPrintf (PETSC_COMM_WORLD, "Formations at snapshot 12: \n");
   for (formationIter = formationList->begin (), i = 0; formationIter != formationList->end (); ++formationIter, ++i)
   {
      const Formation *formation = *formationIter;
      formation->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }
   delete formationList;


   const char *reservoirNames[] = { "R3", "R2" };
   const char *reservoirFormationNames[] = { "fH3", "fH2" };

   ReservoirList *reservoirList = projectHandle->getReservoirs ();

   ReservoirList::iterator reservoirIter;

   for (reservoirIter = reservoirList->begin (), i = 0; reservoirIter != reservoirList->end (); ++reservoirIter, ++i)
   {
      const Reservoir *reservoir = *reservoirIter;
      reservoir->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      assert (reservoir->getName () == reservoirNames[i]);
      assert (reservoir->getFormation ()->getName () == reservoirFormationNames[i]);
      assert (reservoir->getMap (DepthOffset) == 0);
      assert (reservoir->isDiffusionOn () == true);
      assert (reservoir->isBioDegradationOn () == false);
   }
   delete reservoirList;

   reservoirList = (projectHandle->findFormation ("fH3"))->getReservoirs ();

   for (reservoirIter = reservoirList->begin (); reservoirIter != reservoirList->end (); ++reservoirIter)
   {
      const Reservoir *reservoir = *reservoirIter;
      reservoir->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }
   delete reservoirList;

   MobileLayerList * mobileLayerList = (projectHandle->findFormation ("fH3"))->getMobileLayers ();
   assert (mobileLayerList->size () == 1);
   const MobileLayer * mobileLayer = (*mobileLayerList)[0];
   assert (mobileLayer);
   assert (mobileLayer->getSnapshot ()->getTime () == 13);

   const GridMap * mobThicknessMap = mobileLayer->getMap (MobileLayerThicknessMap);
   assert (mobThicknessMap);
   
   const LithoType * lithoType = projectHandle->findLithoType ("Std. Siltstone");
   assert (lithoType);

   assert (lithoType->getAttributeType (Pixmap) == String);
   assert (lithoType->getAttributeValue (Pixmap).getString () == "Siltstone");
   assert (lithoType->getAttributeValue (Pixmap).getType () == String);
   lithoType->getAttributeValue (Pixmap).asString (tmpString);
   // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());


   assert (lithoType->getAttributeType (ThCondAnisotropy) == Double);
   assert (lithoType->getAttributeValue (ThCondAnisotropy).getDouble () == 1.5);
   assert (lithoType->getAttributeValue (ThCondAnisotropy).getType () == Double);
   lithoType->getAttributeValue (ThCondAnisotropy).asString (tmpString);
   // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

   const AttributeValue & av1 = lithoType->getAttributeValue (Pixmap);
   const AttributeValue & av2 = lithoType->getAttributeValue (Pixmap);
   assert (& av1 == & av2);

   av1.release ();

   /*
   FaultCollection * fc =  projectHandle->findFaultCollection ("TestFaultFaultH3");
   assert (fc != 0);

   assert (fc->size () == 1);

   assert (fc->findFault ("xyzzy") == 0);
   Fault * fault = fc->findFault ("H3test");
   assert (fault != 0);

   GridMap *faultGridMap;
   faultGridMap = projectHandle->findFormation ("fH3")->computeFaultGridMap (projectHandle->getHighResolutionOutputGrid (), projectHandle->findSnapshot (11));
   assert (faultGridMap);
   faultGridMap->retrieveData ();
   faultGridMap->printOn (PETSC_COMM_WORLD);
   faultGridMap->restoreData ();
   faultGridMap->release ();
   */

   /*
   faultGridMap = projectHandle->findFormation ("fH3")->computeFaultGridMap (projectHandle->getHighResolutionOutputGrid (), projectHandle->findSnapshot (15));
   assert (faultGridMap);
   faultGridMap->retrieveData ();
   faultGridMap->printOn (PETSC_COMM_WORLD);
   faultGridMap->restoreData ();
   faultGridMap->release ();
   */

   TrapList *trapList = projectHandle->getTraps (0, 0, 0);

   TrapList::iterator trapIter;
   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
      double massC3 = trap->getMass (C3);
      // PetscPrintf (PETSC_COMM_WORLD, "Mass[C3] = %lf\n", massC3);
      double volumeGas = trap->getVolume (Gas);
      // PetscPrintf (PETSC_COMM_WORLD, "volume[Gas] = %lf\n", volumeGas);
      double volumeOil = trap->getVolume (Oil);
      // PetscPrintf (PETSC_COMM_WORLD, "volume[Oil] = %lf\n", volumeOil);
   }

   delete trapList;

   trapList = projectHandle->getTraps (0, projectHandle->findSnapshot (11), 0);

   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }

   delete trapList;

   trapList = projectHandle->getTraps (projectHandle->findReservoir ("R2"),
	 projectHandle->findSnapshot (11), 0);

   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }

   delete trapList;

   trapList = projectHandle->getTraps (0, 0, 2);

   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }

   delete trapList;

   LeadList *leadList = projectHandle->getLeads ();

   LeadList::iterator leadIter;
   for (leadIter = leadList->begin ();
        leadIter != leadList->end (); ++leadIter)
   {
      const Lead *lead = *leadIter;
      lead->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      LeadTrapList *leadTrapList = lead->getLeadTraps ();

      LeadTrapList::iterator leadTrapIter;
      for (leadTrapIter = leadTrapList->begin ();
	    leadTrapIter != leadTrapList->end (); ++leadTrapIter)
      {
	 const LeadTrap *leadTrap = *leadTrapIter;
	 leadTrap->asString (tmpString);
	 // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

	 // PetscPrintf (PETSC_COMM_WORLD, "  Gas mass in reservoir gas phase: %lf\n", leadTrap->getGasMass (Gas));
	 // PetscPrintf (PETSC_COMM_WORLD, "  Oil mass in reservoir gas phase: %lf\n", leadTrap->getOilMass (Gas));

	 // PetscPrintf (PETSC_COMM_WORLD, "  Gas mass in reservoir oil phase: %lf\n", leadTrap->getGasMass (Oil));
	 // PetscPrintf (PETSC_COMM_WORLD, "  Oil mass in reservoir oil phase: %lf\n", leadTrap->getOilMass (Oil));

	 // PetscPrintf (PETSC_COMM_WORLD, "  api gravity in reservoir gas phase: %lf\n", leadTrap->getAPIGravity (Gas));
	 // PetscPrintf (PETSC_COMM_WORLD, "  api gravity in reservoir oil phase: %lf\n", leadTrap->getAPIGravity (Oil));
      }

      delete leadTrapList;
   }

   delete leadList;

   PropertyList *propertyList = projectHandle->getProperties (true);

   PropertyList::iterator propertyIter;

   const char *propertyNames[] = {
      "BulkDensity",
      "Diffusivity",
      "HeatFlow",
      "HeatFlowX",
      "HeatFlowY",
      "HeatFlowZ",
      "Vr",
      "Vre",
      "VreMaturity",
      "Ves",
      "MaxVes",
      "Permeability"
   };

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;
	  cout << property->getName () << "\t" << propertyNames[i] << endl;
      //assert (property->getName () == propertyNames[i]);
   }
   delete propertyList;

   propertyList = projectHandle->getProperties (false);
   // PetscPrintf (PETSC_COMM_WORLD, "Properties present: \n");

   const Reservoir * reservoirA = projectHandle->findReservoir ("R2");
   const Formation * formationA = projectHandle->findFormation ("fH4");
   assert (formationA);
   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () /* && i < 10 */; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      PropertyValueList *propertyValueList = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR, property);
      // PetscPrintf (PETSC_COMM_WORLD, "# values = %d\n", propertyValueList->size ());
      delete propertyValueList;

      propertyValueList = property->getPropertyValues (FORMATION , 0, 0, formationA, 0);
      // PetscPrintf (PETSC_COMM_WORLD, "# fH4 values = %d\n", propertyValueList->size ());
      delete propertyValueList;

      propertyValueList = property->getPropertyValues (RESERVOIR , 0, reservoirA, 0, 0);
      // PetscPrintf (PETSC_COMM_WORLD, "# R2 values = %d\n", propertyValueList->size ());
      delete propertyValueList;
   }

   delete propertyList;

   propertyList = projectHandle->getProperties (false, FORMATION,
         0, 0,  0, 0, VOLUME);
   // PetscPrintf (PETSC_COMM_WORLD, "Volume Properties present:\n");

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }
   delete propertyList;

   propertyList = projectHandle->getProperties (false, SURFACE,
         0, 0,  0, 0, SURFACE);
   // PetscPrintf (PETSC_COMM_WORLD, "Surface Properties present:\n");

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }
   delete propertyList;

   propertyList = projectHandle->getProperties (false, FORMATIONSURFACE,
         0, 0, 0, 0, SURFACE);
   // PetscPrintf (PETSC_COMM_WORLD, "Formation Surface Properties present:\n");

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }
   delete propertyList;

   propertyList = projectHandle->getProperties (false, FORMATION,
         0, 0, 0, 0, SURFACE);
   // PetscPrintf (PETSC_COMM_WORLD, "Formation Properties present:\n");

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
   }
   delete propertyList;


   const Property * depthProperty = projectHandle->findProperty ("Depth");
   assert (depthProperty);

   PropertyValueList *propertyValueList =
	    depthProperty->getPropertyValues (SURFACE,
	    projectHandle->findSnapshot (0),
	    0,
	    projectHandle->findFormation ("fH1"),
	    projectHandle->findSurface ("H1"));

   PropertyValueList::iterator propertyValueIter;

   for (propertyValueIter = propertyValueList->begin ();
        propertyValueIter != propertyValueList->end (); ++propertyValueIter)
   {
      const PropertyValue *propertyValue = *propertyValueIter;

      propertyValue->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      const GridMap *gridMap = propertyValue->getGridMap ();

      assert (gridMap != 0);

      assert (gridMap->getUndefinedValue () == 99999);

      assert (gridMap);
      gridMap->retrieveData ();
      // gridMap->printOn (PETSC_COMM_WORLD);
      gridMap->restoreData ();
      // gridMap->release ();
   }

   delete propertyValueList;

   propertyValueList =
      projectHandle->getPropertyValues (FORMATION,
	    projectHandle->findProperty ("Depth"),
	    projectHandle->findSnapshot (0),
	    0,
	    projectHandle->findFormation ("fH1"),
	    projectHandle->findSurface ("H1"));

   for (propertyValueIter = propertyValueList->begin ();
        propertyValueIter != propertyValueList->end (); ++propertyValueIter)
   {
      const PropertyValue *propertyValue = *propertyValueIter;

      propertyValue->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      const GridMap *gridMap = propertyValue->getGridMap ();

      gridMap->getGrid ()->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      assert (gridMap != 0);

      assert (gridMap->getUndefinedValue () == 99999);

      assert (gridMap);
      gridMap->retrieveData ();
      // gridMap->printOn (PETSC_COMM_WORLD);
      gridMap->restoreData ();

      const GridMap * gridMap2 = propertyValue->getGridMap ();
      assert (gridMap == gridMap2);
      // gridMap->release ();
   }

   delete propertyValueList;

   propertyValueList =
      projectHandle->getPropertyValues (FORMATION | FORMATIONSURFACE,
	    0,
	    0,
	    0,
	    projectHandle->findFormation ("fH2"),
	    0);

   for (propertyValueIter = propertyValueList->begin ();
        propertyValueIter != propertyValueList->end (); ++propertyValueIter)
   {
      const PropertyValue *propertyValue = *propertyValueIter;

      propertyValue->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

      const GridMap *gridMap = propertyValue->getGridMap ();

      assert (gridMap != 0);
      assert (gridMap->getUndefinedValue () == 99999);

      gridMap->getGrid ()->asString (tmpString);
      // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());
      
      gridMap->retrieveData ();
      // gridMap->printOn (PETSC_COMM_WORLD);
      gridMap->restoreData ();

      const GridMap * gridMap2 = propertyValue->getGridMap ();
      assert (gridMap == gridMap2);
      // gridMap->release ();
   }

   delete propertyValueList;

   const Surface * surface = projectHandle->findSurface ("H1");
   const GridMap * depthMap = surface->getInputDepthMap ();

   assert (depthMap);
   depthMap->retrieveData ();
   // depthMap->printOn (PETSC_COMM_WORLD);
   depthMap->restoreData ();

   // depthMap->release ();

   surface = projectHandle->findSurface ("H1");
   depthMap = surface->getInputDepthMap ();
   assert (depthMap);
   depthMap->retrieveData ();
   // depthMap->printOn (PETSC_COMM_WORLD);
   depthMap->restoreData ();

   const Formation * formation = projectHandle->findFormation ("fH2");
   const GridMap * thicknessMap = formation->getInputThicknessMap ();
   assert (thicknessMap);
   thicknessMap->retrieveData ();
   // thicknessMap->printOn (PETSC_COMM_WORLD);
   thicknessMap->restoreData ();

   // thicknessMap->release ();

   formation = projectHandle->findFormation ("fH2");
   thicknessMap = formation->getInputThicknessMap ();
   assert (thicknessMap);
   thicknessMap->retrieveData ();
   // thicknessMap->printOn (PETSC_COMM_WORLD);
   thicknessMap->restoreData ();

   // thicknessMap->release ();

   formation = projectHandle->findFormation ("fH3");
   thicknessMap = formation->getInputThicknessMap ();
   assert (thicknessMap);

   formation->asString (tmpString);
   // PetscPrintf (PETSC_COMM_WORLD, tmpString.c_str ());

   assert (formation->isSourceRock ());
   const SourceRock * sourceRock = formation->getSourceRock ();
   assert (sourceRock);

   const GridMap * nettThickIniMap = sourceRock->getMap (NettThickIni);
   assert (nettThickIniMap);
   
   nettThickIniMap->retrieveData ();
   // nettThickIniMap->printOn (PETSC_COMM_WORLD);
   nettThickIniMap->restoreData ();

   const GridMap * upperBiotMap = sourceRock->getMap (UpperBiot);
   assert (upperBiotMap);
   
   upperBiotMap->retrieveData ();
   // upperBiotMap->printOn (PETSC_COMM_WORLD);
   upperBiotMap->restoreData ();

#if IMPLEMENTEDSAVEHDF5
   InputValueList * inputValueList = projectHandle->getInputValues ();

   InputValueList::iterator inputValueIter;

   bool first = true;
   for (inputValueIter = inputValueList->begin ();
        inputValueIter != inputValueList->end (); ++inputValueIter, ++i)
   {
      const InputValue *inputValue = *inputValueIter;

      const GridMap * inputMap;
      if ((inputMap = inputValue->getGridMap ()) != 0)
      {
	 if (first)
	 {
	    inputMap->saveHDF5 ("file.HDF");
	    system ("h5dump file.HDF");
	    first = false;
	 }
      }
   }
   delete inputValueList;
#endif

   const Grid * myGrid = projectHandle->getHighResolutionOutputGrid ();
   assert (myGrid);

   const string myActivity = "MyActivity";
   projectHandle->startActivity (myActivity, myGrid);

   const Formation * myFormation = projectHandle->findFormation ("fH3");
   assert (myFormation);

   const Surface * mySurface = projectHandle->findSurface ("H3");
   assert (mySurface);

   const Snapshot * mySnapshot = projectHandle->findSnapshot (15);
   assert (mySnapshot);

   const string myName = "MaxVes";

   PropertyValue * myMapPropertyValue = projectHandle->createMapPropertyValue (myName, mySnapshot, 0, myFormation, mySurface);
   assert (myMapPropertyValue);

   GridMap * myMapGridMap = myMapPropertyValue->getGridMap ();
   assert (myMapGridMap);

   myMapGridMap->retrieveData ();
   // myMapGridMap->printOn (PETSC_COMM_WORLD);

   assert (99999 == myMapGridMap->getUndefinedValue ());
   for (i = myMapGridMap->firstI (); i < myMapGridMap->firstI () + myMapGridMap->numI (); ++i)
   {
      for (j = myMapGridMap->firstJ (); j < myMapGridMap->firstJ () + myMapGridMap->numJ (); ++j)
      {
	 if  (j % 2 != 0)
	 {
	 double myValue = myMapGridMap->getValue ((unsigned int) i, (unsigned int) j);
	 assert (myValue == myMapGridMap->getUndefinedValue ());
	 myMapGridMap->setValue (i, j, i * myMapGridMap->getGrid ()->numJGlobal () + j);
	 }
      }
   }

   myMapGridMap->restoreData ();


   PropertyValue * myVolumePropertyValue = projectHandle->createVolumePropertyValue (myName, mySnapshot, 0, myFormation, 4);
   assert (myVolumePropertyValue);

   GridMap * myVolumeGridMap = myVolumePropertyValue->getGridMap ();
   assert (myVolumeGridMap);
   assert (myVolumeGridMap->getDepth () == 4);

   myVolumeGridMap->retrieveData ();
   // myVolumeGridMap->printOn (PETSC_COMM_WORLD);

   for (i = myVolumeGridMap->firstI (); i < myVolumeGridMap->firstI () + myVolumeGridMap->numI (); ++i)
   {
      for (j = myVolumeGridMap->firstJ (); j < myVolumeGridMap->firstJ () + myVolumeGridMap->numJ (); ++j)
      {
	 for (k = 0; k < myVolumeGridMap->getDepth (); ++k)
	 {
	    myVolumeGridMap->setValue (i, j, k, (i * myVolumeGridMap->getGrid ()->numJGlobal ()  + j) * myVolumeGridMap->getDepth () + k);
	 }
      }
   }

   myVolumeGridMap->restoreData ();

   projectHandle->finishActivity ();
   projectHandle->saveToFile ("../data/MyFile.project3d");

   delete projectHandle;
   PetscPrintf (PETSC_COMM_WORLD, "Project closed\n");

   ierr = PetscFinalize ();
   CHKERRQ (ierr);

   return 0;
}
