#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

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

#include "Interface/GridMap.h"
#include "Interface/Grid.h"
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


#include <string>
using namespace std;

using namespace DataAccess;
using namespace Interface;

namespace database
{
   class Record;
}

#ifdef NOTIFDYNAMICALLYLOADED
class MySurface : public Interface::Surface
{
public:
   MySurface (Interface::ProjectHandle * projectHandle, database::Record * record);

   virtual void printOn (ostream &ostr) const;
};

MySurface::MySurface (Interface::ProjectHandle * projectHandle, database::Record * record) :
   Interface::Surface (projectHandle, record)
{
}

void MySurface::printOn (ostream & ostr) const
{
   ostr << "My:";
   Interface::Surface::printOn (ostr);
}

class MyObjectFactory : public Interface::ObjectFactory
{
   virtual Interface::Surface * produceSurface (Interface::ProjectHandle * projectHandle, database::Record * record);
};

Interface::Surface * Interface::ObjectFactory::produceSurface (Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new MySurface (projectHandle, record);
}
#endif

using namespace std;

int main (int argc, char ** argv)
{
   ProjectHandle *projectHandle = OpenCauldronProject ("../data/TestFaultHR.project3d", "rw");

   cerr << "Opened: ../data/TestFaultHR.project3d" << endl;

   assert (projectHandle);
   projectHandle->printOn (cerr);

   SnapshotList *snapshotList = projectHandle->getSnapshots ();

   SnapshotList *snapshotList2 = projectHandle->getSnapshots ();

   assert (snapshotList != snapshotList2);
   delete snapshotList2;

   assert (snapshotList->size () == 7);
   assert ((*snapshotList)[3]->getTime () == 13);

   double times[] = { 0, 11, 12, 13, 14, 15, 100 };
   SnapshotList::iterator snapshotIter;
   int i;

   cerr << "Snapshots: " << endl;
   for (snapshotIter = snapshotList->begin (), i = 0; snapshotIter != snapshotList->end (); ++snapshotIter, ++i)
   {
      const Snapshot *snapshot = *snapshotIter;
      snapshot->printOn (cerr);

      assert (snapshot->getTime () == times[i]);
   }

   delete snapshotList;

   const char *surfaceNames[] = { "H0", "H1", "H2", "H3", "H4", "H5" };
   const char *formationNames[] = { "fH0", "fH1", "fH2", "fH3", "fH4" };

   SurfaceList *surfaceList = projectHandle->getSurfaces ();

   SurfaceList::iterator surfaceIter;

   cerr << "Surfaces:" << endl;
   for (surfaceIter = surfaceList->begin (), i = 0; surfaceIter != surfaceList->end (); ++surfaceIter, ++i)
   {
      const Surface *surface = *surfaceIter;
      surface->printOn (cerr);
	  
      const GridMap * gridMap = surface->getInputDepthMap ();
      gridMap->printOn (cerr);
      gridMap->release ();

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

   surfaceList = projectHandle->getSurfaces (projectHandle->findSnapshot (12));
   cerr << "Surfaces at snapshot 12:" << endl;
   for (surfaceIter = surfaceList->begin (), i = 0; surfaceIter != surfaceList->end (); ++surfaceIter, ++i)
   {
      const Surface *surface = *surfaceIter;
      surface->printOn (cerr);
   }
   delete surfaceList;

   FormationList *formationList = projectHandle->getFormations ();

   FormationList::iterator formationIter;

   cerr << "Formations:" << endl;
   for (formationIter = formationList->begin (), i = 0; formationIter != formationList->end (); ++formationIter, ++i)
   {
      const Formation *formation = *formationIter;
      formation->printOn (cerr);

      const GridMap * gridMap = formation->getInputThicknessMap ();
      gridMap->printOn (cerr);
      gridMap->release ();

      const LithoType * lithoType = formation->getLithoType1 ();
      assert (lithoType);
      lithoType->printOn (cerr);

      gridMap = formation->getLithoType1PercentageMap ();
      assert (gridMap->isConstant ());
      gridMap->printOn (cerr);
      gridMap->release ();

      lithoType = formation->getLithoType2 ();
      if (lithoType)
      {
         lithoType->printOn (cerr);

         gridMap = formation->getLithoType2PercentageMap ();
         assert (gridMap->isConstant ());
         gridMap->printOn (cerr);
         gridMap->release ();
      }

      lithoType = formation->getLithoType3 ();
      if (lithoType)
      {
         lithoType->printOn (cerr);

         gridMap = formation->getLithoType3PercentageMap ();
         assert (gridMap->isConstant ());
         gridMap->printOn (cerr);
         gridMap->release ();
      }

      assert (formation->getName () == formationNames[i]);
      assert (formation->getTopSurface ()->getName () == surfaceNames[i]);
      assert (formation->getBottomSurface ()->getName () == surfaceNames[i + 1]);
      
   }
   delete formationList;

   formationList = projectHandle->getFormations (projectHandle->findSnapshot (12));
   cerr << "Formations at snapshot 12:" << endl;
   for (formationIter = formationList->begin (), i = 0; formationIter != formationList->end (); ++formationIter, ++i)
   {
      const Formation *formation = *formationIter;
      formation->printOn (cerr);
   }
   delete formationList;


   const char *reservoirNames[] = { "R3", "R2" };
   const char *reservoirFormationNames[] = { "fH3", "fH2" };

   ReservoirList *reservoirList = projectHandle->getReservoirs ();

   ReservoirList::iterator reservoirIter;

   for (reservoirIter = reservoirList->begin (), i = 0; reservoirIter != reservoirList->end (); ++reservoirIter, ++i)
   {
      const Reservoir *reservoir = *reservoirIter;
      reservoir->printOn (cerr);

      assert (reservoir->getName () == reservoirNames[i]);
      assert (reservoir->getFormation ()->getName () == reservoirFormationNames[i]);
      assert (reservoir->getMap (DepthOffset) == 0);
      assert (reservoir->isDiffusionOn () == false);
      assert (reservoir->isBioDegradationOn () == false);
   }
   delete reservoirList;

   reservoirList = (projectHandle->findFormation ("fH3"))->getReservoirs ();

   for (reservoirIter = reservoirList->begin (); reservoirIter != reservoirList->end (); ++reservoirIter)
   {
      const Reservoir *reservoir = *reservoirIter;
      reservoir->printOn (cerr);
   }
   delete reservoirList;

   MobileLayerList * mobileLayerList = (projectHandle->findFormation ("fH3"))->getMobileLayers ();
   assert (mobileLayerList->size () == 1);
   const MobileLayer * mobileLayer = (*mobileLayerList)[0];
   assert (mobileLayer);
   assert (mobileLayer->getSnapshot ()->getTime () == 13);

   const GridMap * mobThicknessMap = mobileLayer->getMap (MobileLayerThicknessMap);
   assert (mobThicknessMap);
   
   assert (mobThicknessMap->isConstant ());
   assert (mobThicknessMap->getConstantValue () == 75);

   const LithoType * lithoType = projectHandle->findLithoType ("Std. Siltstone");
   assert (lithoType);

   assert (lithoType->getAttributeType (Pixmap) == String);
   assert (lithoType->getAttributeValue (Pixmap).getString () == "Siltstone");
   assert (lithoType->getAttributeValue (Pixmap).getType () == String);
   lithoType->getAttributeValue (Pixmap).printOn (cerr);

   assert (lithoType->getAttributeType (ThCondAnisotropy) == Double);
   assert (lithoType->getAttributeValue (ThCondAnisotropy).getDouble () == 1.5);
   assert (lithoType->getAttributeValue (ThCondAnisotropy).getType () == Double);
   lithoType->getAttributeValue (ThCondAnisotropy).printOn (cerr);

   const AttributeValue & av1 = lithoType->getAttributeValue (Pixmap);
   const AttributeValue & av2 = lithoType->getAttributeValue (Pixmap);
   assert (& av1 == & av2);

   av1.release ();

   TrapList *trapList = projectHandle->getTraps (0, 0, 0);

   TrapList::iterator trapIter;
   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->printOn (cerr);
      double massC3 = trap->getMass (C3);
      cerr << "Mass[C3] = " << massC3 << endl;
      double volumeGas = trap->getVolume (Gas);
      cerr << "volume[Gas] = " << volumeGas << endl;
      double volumeOil = trap->getVolume (Oil);
      cerr << "volume[Oil] = " << volumeOil << endl;
   }

   delete trapList;

   trapList = projectHandle->getTraps (0, projectHandle->findSnapshot (11), 0);

   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->printOn (cerr);
   }

   delete trapList;

   trapList = projectHandle->getTraps (projectHandle->findReservoir ("R2"),
	 projectHandle->findSnapshot (11), 0);

   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->printOn (cerr);
   }

   delete trapList;

   trapList = projectHandle->getTraps (0, 0, 2);

   for (trapIter = trapList->begin ();
        trapIter != trapList->end (); ++trapIter)
   {
      const Trap *trap = *trapIter;
      trap->printOn (cerr);
   }

   delete trapList;

   LeadList *leadList = projectHandle->getLeads ();

   LeadList::iterator leadIter;
   for (leadIter = leadList->begin ();
        leadIter != leadList->end (); ++leadIter)
   {
      const Lead *lead = *leadIter;
      lead->printOn (cerr);

      LeadTrapList *leadTrapList = lead->getLeadTraps ();

      LeadTrapList::iterator leadTrapIter;
      for (leadTrapIter = leadTrapList->begin ();
	    leadTrapIter != leadTrapList->end (); ++leadTrapIter)
      {
	 const LeadTrap *leadTrap = *leadTrapIter;
	 leadTrap->printOn (cerr);
	 cerr << "  Gas mass in reservoir gas phase: " << leadTrap->getGasMass (Gas) << endl;
	 cerr << "  Oil mass in reservoir gas phase: " << leadTrap->getOilMass (Gas) << endl;
	 cerr << "  Gas mass in reservoir oil phase: " << leadTrap->getGasMass (Oil) << endl;
	 cerr << "  Oil mass in reservoir oil phase: " << leadTrap->getOilMass (Oil) << endl;
	 cerr << "  api gravity in reservoir gas phase: " << leadTrap->getAPIGravity (Gas) << endl;
	 cerr << "  api gravity in reservoir oil phase: " << leadTrap->getAPIGravity (Oil) << endl;
      }

      delete leadTrapList;
   }

   delete leadList;

   PropertyList *propertyList = projectHandle->getProperties (true);

   PropertyList::iterator propertyIter;

   cerr << "First 10 Properties available: " << endl;
   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;
      property->printOn (cerr);
   }

   delete propertyList;

   propertyList = projectHandle->getProperties (false);
   cerr << "Properties present: " << endl;

   const Reservoir * reservoirA = projectHandle->findReservoir ("R2");
   const Formation * formationA = projectHandle->findFormation ("fH4");
   assert (formationA);
   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () /* && i < 10 */; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->printOn (cerr);
      PropertyValueList *propertyValueList = projectHandle->getPropertyValues (SURFACE | FORMATION | FORMATIONSURFACE | RESERVOIR, property);
      cerr << "# values = " << propertyValueList->size () << endl;
      delete propertyValueList;
      propertyValueList = property->getPropertyValues (FORMATION , 0, 0, formationA, 0);
      cerr << "# fH4 values = " << propertyValueList->size () << endl;
      delete propertyValueList;
      propertyValueList = property->getPropertyValues (RESERVOIR , 0, reservoirA, 0, 0);
      cerr << "# R2 values = " << propertyValueList->size () << endl;
      delete propertyValueList;
   }

   delete propertyList;

   propertyList = projectHandle->getProperties (false, FORMATION,
         0, 0,  0, 0, VOLUME);
   cerr << "Volume Properties present: " << endl;

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->printOn (cerr);
   }
   delete propertyList;

   propertyList = projectHandle->getProperties (false, SURFACE,
         0, 0,  0, 0, SURFACE);
   cerr << "Surface Properties present: " << endl;

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->printOn (cerr);
   }
   delete propertyList;

   propertyList = projectHandle->getProperties (false, FORMATIONSURFACE,
         0, 0, 0, 0, SURFACE);
   cerr << "Formation Surface Properties present: " << endl;

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->printOn (cerr);
   }
   delete propertyList;

   propertyList = projectHandle->getProperties (false, FORMATION,
         0, 0, 0, 0, SURFACE);
   cerr << "Formation Properties present: " << endl;

   for (propertyIter = propertyList->begin (), i = 0;
        propertyIter != propertyList->end () && i < 10; ++propertyIter, ++i)
   {
      const Property *property = *propertyIter;

      property->printOn (cerr);
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
      propertyValue->printOn (cerr);

      const GridMap *gridMap = propertyValue->getGridMap ();

      assert (gridMap != 0);

//	  cout << gridMap->getUndefinedValue () << endl;

      assert (gridMap->getUndefinedValue () == 99999);

      gridMap->printOn (cerr);
      gridMap->release ();
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

      propertyValue->printOn (cerr);

      const GridMap *gridMap = propertyValue->getGridMap ();

      gridMap->getGrid ()->printOn (cerr);

      assert (gridMap != 0);

      assert (gridMap->getUndefinedValue () == 99999);

      double testValue = 0;

      const unsigned int one = 1;
      const unsigned int two = 2;

      testValue += gridMap->getValue (one, one, one);
      testValue += gridMap->getValue (one, one, two);
      testValue += gridMap->getValue (one, two, one);
      testValue += gridMap->getValue (one, two, two);
      testValue += gridMap->getValue (two, one, one);
      testValue += gridMap->getValue (two, one, two);
      testValue += gridMap->getValue (two, two, one);
      testValue += gridMap->getValue (two, two, two);

      testValue /= 8;
      double gridMapValue = gridMap->getValue (1.5, 1.5, 1.5);

      cerr << "value[1.5][1.5][1.5] = " << gridMap->getValue (1.5, 1.5, 1.5) << endl;
      assert (gridMapValue > (1 - 0.00000001) * testValue & gridMapValue < (1 + 0.00000001) * testValue);

      gridMap->printOn (cerr);

      const GridMap * gridMap2 = propertyValue->getGridMap ();
      assert (gridMap == gridMap2);
      gridMap->release ();
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

      propertyValue->printOn (cerr);

      const GridMap *gridMap = propertyValue->getGridMap ();

      gridMap->getGrid ()->printOn (cerr);

      assert (gridMap != 0);

      assert (gridMap->getUndefinedValue () == 99999);

      gridMap->printOn (cerr);

      const GridMap * gridMap2 = propertyValue->getGridMap ();
      assert (gridMap == gridMap2);
      gridMap->release ();
   }

   delete propertyValueList;

   const Surface * surface = projectHandle->findSurface ("H1");
   const GridMap * depthMap = surface->getInputDepthMap ();
   assert (depthMap);

   depthMap->printOn (cerr);

   depthMap->release ();

   surface = projectHandle->findSurface ("H1");
   depthMap = surface->getInputDepthMap ();
   assert (depthMap);
   depthMap->printOn (cerr);

   const Formation * formation = projectHandle->findFormation ("fH2");
   const GridMap * thicknessMap = formation->getInputThicknessMap ();
   assert (thicknessMap);
   thicknessMap->printOn (cerr);

   thicknessMap->release ();

   formation = projectHandle->findFormation ("fH2");
   thicknessMap = formation->getInputThicknessMap ();
   assert (thicknessMap);
   thicknessMap->printOn (cerr);

   thicknessMap->release ();

   formation = projectHandle->findFormation ("fH3");
   thicknessMap = formation->getInputThicknessMap ();
   assert (thicknessMap);
   formation->printOn (cerr);

   assert (formation->isSourceRock ());
   const SourceRock * sourceRock = formation->getSourceRock ();
   assert (sourceRock);

   const GridMap * nettThickIniMap = sourceRock->getMap (NettThickIni);
   assert (nettThickIniMap);
   
   nettThickIniMap->printOn (cerr);

   const GridMap * upperBiotMap = sourceRock->getMap (UpperBiot);
   assert (upperBiotMap);
   
   upperBiotMap->printOn (cerr);

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
	    first = false;
	 }
      }
   }
   delete inputValueList;

   // projectHandle->saveForBPA ("../data/noresults/noresults.project3d");

   // delete projectHandle;
   CloseCauldronProject (projectHandle);
   cerr << "Project closed" << endl;

   return 0;
}
