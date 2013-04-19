#ifdef sgi
  #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
     using namespace std;
      #define USESTANDARD
  #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
  #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include <algorithm>
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "FastReqProjectHandle.h"
using namespace FastReq;

#include "math.h"
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"

using namespace DataAccess;

using Interface::Formation;

using Interface::FormationList; 
using Interface::Grid;

FastReqProjectHandle::FastReqProjectHandle (database::Database * database, const std::string & name, const std::string & accessMode)
:Interface::ProjectHandle (database, name, accessMode)
{
   loadRequiredRunOptions();
   loadCrustVariables();
   loadMantleThicknessPresentDay();	

   setNumberOfHighResGridNodes();
   setNumberOfLowResGridNodes();

   computeNodes();
}
FastReqProjectHandle::~FastReqProjectHandle (void)
{
	
}
FastReqProjectHandle *FastReqProjectHandle::CreateFrom(const std::string & inputFileName)
{
   return (FastReqProjectHandle *) ( ( Interface::ProjectHandle*)Interface::OpenCauldronProject (inputFileName, "rw") );	
}
void FastReqProjectHandle::setNumberOfLowResGridNodes()
{
   const Grid *theGrid = getLowResolutionOutputGrid ();
   unsigned int numI = theGrid->numI();
   unsigned int numJ = theGrid->numJ();
   
   m_LowResGridNodes = numI * numJ;
}
void FastReqProjectHandle::setNumberOfHighResGridNodes()
{
   const Grid *theGrid = getHighResolutionOutputGrid ();
   unsigned int numI = theGrid->numI();
   unsigned int numJ = theGrid->numJ();
   
   m_HighResGridNodes = numI * numJ;
}
void FastReqProjectHandle::loadRequiredRunOptions()
{ 
   Table * RunOptionsIoTbl = getTable ("RunOptionsIoTbl");
   Table::iterator tblIter = RunOptionsIoTbl->begin ();
   
   Record * RunOptionsIoRecord = * tblIter;
   
   m_BrickHeightSediment = database::getBrickHeightSediment(RunOptionsIoRecord);
   m_BrickHeightCrust    = database::getBrickHeightCrust   (RunOptionsIoRecord);
   m_BrickHeightMantle   = database::getBrickHeightMantle  (RunOptionsIoRecord);
}
void FastReqProjectHandle::loadCrustVariables()
{
   m_MaxCrustThickness  = 0.0;
   m_CrustThinningRatio = 0.0;
   m_MinCrustThickness = 1E+30;
   
   const string tableName = "CrustIoTbl";
   Table * CrustIoTbl = getTable (tableName);
   Table::iterator tblIter;
   
   for (tblIter = CrustIoTbl->begin (); tblIter != CrustIoTbl->end (); ++tblIter)
   {
       Record * CrustIoRecord = * tblIter;
       const string & ThicknessGrid = database::getThicknessGrid(CrustIoRecord);
       double currentMaxThickness = 0.0;
       double currentMinThickness = 0.0;
       if(ThicknessGrid == "")
       {
	       currentMaxThickness = database::getThickness(CrustIoRecord); 
	       currentMinThickness = database::getThickness(CrustIoRecord);  
       }
       else
       {
	       const Interface::GridMap * CrustThicknessMap = static_cast <const Interface::GridMap*> (loadInputMap (tableName, ThicknessGrid));  

	       CrustThicknessMap->getMinMaxValue(currentMinThickness, currentMaxThickness);
       }

       if( currentMaxThickness > m_MaxCrustThickness )
       {
          m_MaxCrustThickness = currentMaxThickness;
       }
       if( currentMinThickness < m_MinCrustThickness )
       {
          m_MinCrustThickness = currentMinThickness;
       }
    }
    m_CrustThinningRatio = m_MinCrustThickness / m_MaxCrustThickness;
}
void FastReqProjectHandle::loadMantleThicknessPresentDay()
{
   Table * BasementIoTbl = getTable ("BasementIoTbl");
   Table::iterator tblIter = BasementIoTbl->begin ();
   
   Record * BasementIoRecord = * tblIter;
   
   m_MantleThicknessPresentDay = database::getLithoMantleThickness(BasementIoRecord);
	
}
void FastReqProjectHandle::computeNodes() 
{
   FormationList *formationList = this->getFormations ();
   FormationList::iterator formationIter;
   
   unsigned int numberOfFormations = formationList->size();
   
   //Compute Outer Nodes(nodes lying on the upper and lower bounding surfaces  )
   unsigned int NumberOfOuterNodesLowRes     = m_LowResGridNodes * (numberOfFormations + 3);/*Crust + Mantle + 1(closing surface)*/ 
   unsigned int NumberOfOuterNodesHighRes    = m_HighResGridNodes * (numberOfFormations + 3);/*Crust + Mantle + 1(closing surface)*/ 	
   
   //Compute Inner Nodes(Nodes within the crust, the mantle or a formation )
   unsigned int SumOfInnerFormationNodes = 0;	
  
   for (formationIter = formationList->begin (); formationIter != formationList->end (); ++formationIter)
   {
      const Formation *formation = *formationIter;

      //const Interface::GridMap * gridMap = dynamic_cast<const Interface::GridMap *> (formation->getInputThicknessMap ());
      //const Interface::GridMap * gridMap = (const Interface::GridMap*) (formation->getInputThicknessMap ());
      
      const Interface::GridMap * gridMap = static_cast <const Interface::GridMap*>  (formation->getInputThicknessMap ());
      
      double minFormationThickness = 0.0;
      double maxFormationThickness = 0.0;
      
      gridMap->getMinMaxValue(minFormationThickness, maxFormationThickness);
 
      if( formation->isMobileLayer() && maxFormationThickness < 1000.0 )
      {
          maxFormationThickness = 1000.0;      
      }

      unsigned int FormationInnerNodes = static_cast<unsigned int>( ceil( maxFormationThickness / m_BrickHeightSediment ) ) - 1;

      SumOfInnerFormationNodes += FormationInnerNodes;
  
   }

   delete formationList;
   
   unsigned int SumOfInnerCrustNodes = static_cast<unsigned int>( ceil( m_MaxCrustThickness / m_BrickHeightCrust ) ) - 1;

   unsigned int SumOfInnerMantleNodes = static_cast<unsigned int>( ceil( m_CrustThinningRatio * m_MantleThicknessPresentDay / m_BrickHeightMantle ) ) - 1;

   unsigned int NumberOfInnerNodesLowRes  = m_LowResGridNodes * (SumOfInnerFormationNodes + SumOfInnerCrustNodes + SumOfInnerMantleNodes); 
   unsigned int NumberOfInnerNodesHighRes = m_HighResGridNodes * (SumOfInnerFormationNodes + SumOfInnerCrustNodes + SumOfInnerMantleNodes); 
   
   //   Pressure-Temperature  
   m_PressureTemperatureNodes = NumberOfOuterNodesLowRes + NumberOfInnerNodesLowRes;
   
   //   HighRes Decompaction
   m_HighResDecompationNodes  = NumberOfOuterNodesHighRes + NumberOfInnerNodesHighRes;
   	
   //   Genex
   computeGenexNodes();
   
   //   HighRes Decompaction
   computeMigrationNodes();
   
}
void FastReqProjectHandle::computeGenexNodes() 
{
   m_GenexNodes  =  m_LowResGridNodes * (m_sourceRocks.size());
}
void FastReqProjectHandle::computeMigrationNodes() 
{
   m_MigrationNodes  =  m_HighResGridNodes * (m_reservoirs.size());
} 
unsigned int FastReqProjectHandle::getNumberOfSnapShots() const
{
   Interface::MutableSnapshotList::const_iterator itEnd   = m_snapshots.end();
   Interface::MutableSnapshotList::const_iterator it      = m_snapshots.begin();
   
   unsigned int counter = 0;
   
   while(it != itEnd)
   {
      if( (*it)->getType() == Interface::MAJOR )
      {
         ++counter;
      }
      ++it;
   }   
   return counter;
}





