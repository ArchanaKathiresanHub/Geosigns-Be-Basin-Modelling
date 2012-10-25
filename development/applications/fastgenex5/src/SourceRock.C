#include <values.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

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

#include <vector>

#include "petscvec.h"
#include "petscda.h"

#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/ProjectData.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/PropertyValue.h"
#include "Interface/AttributeValue.h"
#include "Interface/LithoType.h"
#include "Interface/Interface.h"

using namespace DataAccess;
using namespace CBMGenerics;
using Interface::GridMap;
using Interface::Grid;
using Interface::Snapshot;
using Interface::Formation;
using Interface::Surface;
using Interface::PropertyValue;
using Interface::PropertyValueList;
using Interface::AttributeValue;
using Interface::LithoType;


#include "SourceRock.h"
#include "GenexSimulation.h"
#include "GenexSimulator.h"
#include "LocalGridInterpolator.h"
#include "SnapshotInterval.h"

#include "Simulator.h"
#include "SourceRockNode.h"
#include "SourceRockNodeInput.h"
#include "SourceRockNodeOutput.h"
#include "Constants.h"
#include "SpeciesResult.h"
#include "SpeciesState.h"
#include "Utilities.h"
#include "Constants.h"

#include "AdsorptionSimulator.h"
#include "AdsorptionFunction.h"
#include "C1AdsorptionSimulator.h"
#include "LangmuirAdsorptionFunction.h"

#include "NodeAdsorptionHistory.h"
#include "PVTCalculator.h"

#include "GenexResultManager.h"
#include "ComponentManager.h"
using namespace Genex5;

#include "AdsorptionFunctionFactory.h"
#include "AdsorptionSimulatorFactory.h"

#define ADSORPED_EXPELLED_MAPS 0

namespace GenexSimulation
{
std::map<std::string, std::string> SourceRock::s_CfgFileNameBySRType;

SourceRock::SourceRock (Interface::ProjectHandle * projectHandle, database::Record * record)
: Interface::SourceRock (projectHandle, record)
{
   m_theSimulator = 0;
   m_adsorptionProjectHandle = 0;

   if(s_CfgFileNameBySRType.empty())
   {
      initializeCfgFileNameBySRType();
   }
}
void SourceRock::initializeCfgFileNameBySRType()
{
   s_CfgFileNameBySRType["LacustrineAlgal"]           = "TypeI";
   s_CfgFileNameBySRType["MesozoicMarineShale"]       = "TypeII";
   s_CfgFileNameBySRType["MesozoicCalcareousShale"]   = "TypeIIS";
   s_CfgFileNameBySRType["PaleozoicMarineShale"]      = "TypeIIHS";
   s_CfgFileNameBySRType["VitriniticCoals"]           = "TypeIII";
   
}
SourceRock::~SourceRock(void)
{
   clearSnapshotIntervals();
   clearSimulator();
   clearSourceRockNodes();

   m_theSnapShotOutputMaps.clear();
   m_adsorpedOutputMaps.clear ();
   m_expelledOutputMaps.clear ();
   m_sourceRockExpelledOutputMaps.clear ();
   m_freeOutputMaps.clear ();
   m_retainedOutputMaps.clear ();

   m_hcSaturationOutputMap = 0;
   m_irreducibleWaterSaturationOutputMap = 0;

   m_gasVolumeOutputMap = 0;
   m_oilVolumeOutputMap = 0;
   m_gasExpansionRatio = 0;
   m_gasGeneratedFromOtgc = 0;
   m_totalGasGenerated = 0;

   m_fracOfAdsorptionCap = 0;
   m_hcLiquidSaturation = 0;
   m_hcVapourSaturation = 0;
   m_adsorptionCapacity = 0;

   m_retainedOilApiOutputMap = 0;
   m_retainedCondensateApiOutputMap = 0;

   m_retainedGor = 0;
   m_retainedCgr = 0;

   if ( m_adsorptionProjectHandle != 0 ) {
      m_adsorptionProjectHandle = 0;
   }

   if(!s_CfgFileNameBySRType.empty())
   {
      s_CfgFileNameBySRType.clear();
   }
}
void SourceRock::clearSimulator()
{
   if(m_theSimulator)
   {
      delete m_theSimulator;
      m_theSimulator = 0;
   }

}
void SourceRock::clearSnapshotIntervals()
{
   std::vector<SnapshotInterval*>::iterator itEnd = m_theIntervals.end();
   for(std::vector<SnapshotInterval*>::iterator it = m_theIntervals.begin(); it != itEnd; ++it)
   {
     delete (*it); 
   }
   m_theIntervals.clear();
}
void SourceRock::clearSourceRockNodes()
{
   std::vector<Genex5::SourceRockNode*>::iterator itEnd = m_theNodes.end();

   for(std::vector<Genex5::SourceRockNode*>::iterator it = m_theNodes.begin(); it !=itEnd; ++it)
   {
      delete (*it);
   }
   m_theNodes.clear();
}
double SourceRock::getDepositionTime() const
{
   const Formation * formation = m_projectHandle->findFormation (this->getLayerName());
   const Surface * topSurface = formation->getTopSurface ();
   const Snapshot * depoSnapshot = topSurface->getSnapshot ();
   return depoSnapshot->getTime ();
}

void SourceRock::setAdsorptionProjectHandle ( AdsorptionProjectHandle* adsorptionProjectHandle ) {
   m_adsorptionProjectHandle = adsorptionProjectHandle;
}


bool SourceRock::compute()
{
   bool status = true;
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());
   if(GetRank() == 0)
   {
      cout<<"Ready to compute SourceRock at : "<<theFormation->getName()<<endl;
   }
   status = preprocess();
  
   if(status) status = initialize();
   
   if(status) status = process();
   
   return status;
}
bool SourceRock::validateGuiValue(const double & GuiValue, const double & LowerBound,const double & UpperBound)
{
   if(GuiValue > (LowerBound - Constants::ZERO) &&
      GuiValue < (UpperBound + Constants::ZERO) &&
      GuiValue > 0.0)
   {
      return true; 
   }
   return false;
}
const string & SourceRock::determineConfigurationFileName(const string & SourceRockType)
{ 
   static string ret("TypeI");

   std::map<std::string, std::string>::iterator it = s_CfgFileNameBySRType.find(SourceRockType);

   if(it != s_CfgFileNameBySRType.end())
   {
       return it->second;
   }
   else
   {
       if(GetRank() == 0)
       {
           cout<<"!!Warning!!: Source rock "<<SourceRockType<<" not found. Setting configuration file to TypeI."<<endl;
       }
   }
   return ret;
}
bool SourceRock::initialize()
{
   bool status = true;
   if (m_theIntervals.size () == 0) return true;
   if(GetRank() == 0)
   {
      cout<<"Start Of Initialization..."<<endl;
   }
   
   std::string fullPath;
   char *GENEX5DIR = getenv("GENEX5DIR"); 
   char *MY_GENEX5DIR = getenv("MY_GENEX5DIR");
   
   if(MY_GENEX5DIR != 0)
   {
      fullPath = MY_GENEX5DIR;
   }
   else if(GENEX5DIR != 0)
   {
       fullPath = GENEX5DIR;
   }
   else
   {
       if(GetRank() == 0)
       {
          cout<<"Environment Variable GENEX5DIR not set. Aborting..."<<endl;
       } 
       return false;
   }

   const string & SourceRockType  = getType();

   const string & theType  = determineConfigurationFileName( getType() );

   if(GetRank() == 0)
   {
      cout<<"Source Rock Type  : "<<SourceRockType<<endl;
      cout<<"Configuration File: "<<theType<<endl;
   } 


   const double & theHcIni = getHcVRe05();
   const double theEact     = 1000.0 * getPreAsphaltStartAct();

   const double theAsphalteneDiffusionEnergy = 1000.0 * getAsphalteneDiffusionEnergy();
   const double theResinDiffusionEnergy  = 1000.0 * getResinDiffusionEnergy();
   const double theC15AroDiffusionEnergy = 1000.0 * getC15AroDiffusionEnergy();
   const double theC15SatDiffusionEnergy = 1000.0 * getC15SatDiffusionEnergy();

   double in_HC = validateGuiValue(theHcIni,0.5, 1.8)    == true ? theHcIni :1.56;
   double in_Emean = validateGuiValue(theEact,190000.0, 230000.0) == true ? theEact : 216000.0;

   double in_VRE = 0.5; //overwrite, no other value should be passed to the kernel
   double in_asphalteneDiffusionEnergy = validateGuiValue(theAsphalteneDiffusionEnergy,50000.0,100000.0)== true ? theAsphalteneDiffusionEnergy : 88000.0;
   double in_resinDiffusionEnergy  = validateGuiValue(theResinDiffusionEnergy,50000.0,100000.0)== true ? theResinDiffusionEnergy : 85000.0;
   double in_C15AroDiffusionEnergy = validateGuiValue(theC15AroDiffusionEnergy,50000.0,100000.0)== true ? theC15AroDiffusionEnergy : 80000.0;
   double in_C15SatDiffusionEnergy = validateGuiValue(theC15SatDiffusionEnergy,50000.0,100000.0)== true ? theC15SatDiffusionEnergy : 75000.0;

   m_theSimulator = new Genex5::Simulator(fullPath, theType, in_HC, in_Emean, in_VRE,
                                          in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy, 
                                          in_C15AroDiffusionEnergy, in_C15SatDiffusionEnergy);

   if ( doApplyAdsorption ()) {

      if ( GetRank() == 0 ) {
#if 0
         PetscErrorPrintf ( "Applying adsorption, TOCDependent is %s, function is %s\n",
                            (adsorptionIsTOCDependent () ? "true" : "false" ),
                            getAdsorptionCapacityFunctionName ().c_str ());
#endif
         cerr << "Applying adsorption, TOCDependent is " << (adsorptionIsTOCDependent () ? "true" : "false" ) << ", function is " << getAdsorptionCapacityFunctionName () << endl;
      }

      AdsorptionFunction*  adsorptionFunction = AdsorptionFunctionFactory::getInstance ().getAdsorptionFunction ( m_adsorptionProjectHandle, adsorptionIsTOCDependent (), getAdsorptionCapacityFunctionName ());
      AdsorptionSimulator* adsorptionSimulator = AdsorptionSimulatorFactory::getInstance ().getAdsorptionSimulator ( m_adsorptionProjectHandle, getAdsorptionSimulatorName (), doComputeOTGC ());

      // AdsorptionSimulator* adsorptionSimulator = AdsorptionSimulatorFactory::getInstance ().getAdsorptionSimulator ( m_adsorptionProjectHandle, getAdsorptionSimulatorName (), doApplyAdsorption () );


      assert ( adsorptionSimulator != 0 );
      assert ( adsorptionFunction != 0 );

      adsorptionSimulator->setAdsorptionFunction ( adsorptionFunction );
      m_theSimulator->setAdsorptionSimulator ( adsorptionSimulator ); 
   }

   //#ifdef DEBUG
   //std::ofstream out("testSimulator.txt", std::ios::out);
   //m_theSimulator->PrintBenchmarkOutput(out);
   //#endif 

   status =  m_theSimulator->Validate();

   if(status)
   {
      initializeSnapShotOutputMaps();
   }

   if(GetRank() == 0)
   {
      if(status)
      {
         cout<<"End Of Initialization."<<endl;
         cout<<"-------------------------------------"<<endl;
      }
      else
      {
          cout<<"ERROR : Invalid Chemical Model. Please check your source rock input parameters. Aborting..."<<endl;
          cout<<"----------------------------------------------------------------------------------"<<endl;
      }
   }
 
   return status;
}
bool SourceRock::preprocess()
{
   bool status = true;

   if (GetRank () == 0)
   {
      cout << "Start of preprocessing..." << endl;
   }

   double sgMeanBulkDensity = 2500.0;

   if ( m_adsorptionProjectHandle != 0 and
        doApplyAdsorption () and
        m_adsorptionProjectHandle->getSGDensitySample () != 0 ) {

      sgMeanBulkDensity = m_adsorptionProjectHandle->getSGDensitySample ()->getDensity ();
   }

   computeSnapshotIntervals ();

   if (m_theIntervals.size () == 0) return true;
   const SnapshotInterval *last = m_theIntervals.back ();
   const Snapshot *presentDay = last->getEnd ();

   const GridMap *temperatureAtPresentDay = getTopSurfacePropertyGridMap ("Temperature", presentDay);
   const GridMap *VREPresentDay = getTopSurfacePropertyGridMap ("Vr", presentDay);

   //load thickness in ActivityOutputGrid
   const GridMap *InputThickness = getInputThicknessGridMap ();

   //TOC in ActivityOutputGrid
   Interface::SourceRockMapAttributeId dataIndex = Interface::TocIni;
   const GridMap *TOCmap = getMap (dataIndex);

   bool isVreOn = isVREoptimEnabled ();

   if (temperatureAtPresentDay && InputThickness && TOCmap)
   {
      temperatureAtPresentDay->retrieveData ();
      InputThickness->retrieveData ();
      TOCmap->retrieveData ();

      if (VREPresentDay && isVreOn)
      {
         VREPresentDay->retrieveData ();
      }

      double lithoDensity1, lithoDensity2, lithoDensity3;

      lithoDensity1 = lithoDensity2 = lithoDensity3 = 0.0;

      const LithoType *litho1 = getLithoType1 ();
      const LithoType *litho2 = getLithoType2 ();
      const LithoType *litho3 = getLithoType3 ();

      const GridMap *litho1PercentageMap = getLithoType1PercentageMap ();
      const GridMap *litho2PercentageMap = getLithoType2PercentageMap ();
      const GridMap *litho3PercentageMap = getLithoType3PercentageMap ();

      unsigned int depthlitho1PercentageMap, depthlitho2PercentageMap, depthlitho3PercentageMap;

      depthlitho1PercentageMap = depthlitho2PercentageMap = depthlitho3PercentageMap = 0;
      if (litho1 && litho1PercentageMap)
      {
         lithoDensity1 = getLithoDensity (litho1);
         litho1PercentageMap->retrieveData ();
         depthlitho1PercentageMap = litho1PercentageMap->getDepth ();
      }
      if (litho2 && litho2PercentageMap)
      {
         lithoDensity2 = getLithoDensity (litho2);
         litho2PercentageMap->retrieveData ();
         depthlitho2PercentageMap = litho2PercentageMap->getDepth ();
      }
      if (litho3 && litho3PercentageMap)
      {
         lithoDensity3 = getLithoDensity (litho3);
         litho3PercentageMap->retrieveData ();
         depthlitho3PercentageMap = litho3PercentageMap->getDepth ();
      }

      unsigned int lowResI, lowResJ;

      unsigned int depthTemp = temperatureAtPresentDay->getDepth ();
      unsigned int depthVre = VREPresentDay->getDepth ();
      unsigned int depthThickness = InputThickness->getDepth ();
      unsigned int depthTOC = TOCmap->getDepth ();

      Interface::ModellingMode theMode = m_projectHandle->getModellingMode ();

      unsigned int endMapI = 0;
      unsigned int endMapJ = 0;

      if (Interface::MODE3D == theMode)
      {
         endMapI = temperatureAtPresentDay->lastI ();
         endMapJ = temperatureAtPresentDay->lastJ ();
      }
      else if (Interface::MODE1D == theMode)
      {
         endMapI = temperatureAtPresentDay->firstI ();
         endMapJ = temperatureAtPresentDay->firstJ ();
      }

      for (lowResI = temperatureAtPresentDay->firstI (); lowResI <= endMapI; ++lowResI)
      {
         for (lowResJ = temperatureAtPresentDay->firstJ (); lowResJ <= endMapJ; ++lowResJ)
         {
            double temperaturePD = temperatureAtPresentDay->getValue (lowResI, lowResJ, depthTemp - 1);
            double VreAtPresentDay = 0.0;

            if (VREPresentDay && isVreOn)
            {
               VreAtPresentDay = VREPresentDay->getValue (lowResI, lowResJ, depthVre - 1);
            }
            double in_thickness = InputThickness->getValue (lowResI, lowResJ, depthThickness - 1);
            double in_TOC = TOCmap->getValue (lowResI, lowResJ, depthTOC - 1);
            double inorganicDensity = 0.0;

            if (litho1 && litho1PercentageMap)
            {
               inorganicDensity +=
                     lithoDensity1 * 0.01 * litho1PercentageMap->getValue (lowResI, lowResJ,
                                                                           depthlitho1PercentageMap - 1);
            }
            if (litho2 && litho2PercentageMap)
            {
               inorganicDensity +=
                     lithoDensity2 * 0.01 * litho2PercentageMap->getValue (lowResI, lowResJ,
                                                                           depthlitho2PercentageMap - 1);
            }
            if (litho3 && litho3PercentageMap)
            {
               inorganicDensity +=
                     lithoDensity3 * 0.01 * litho3PercentageMap->getValue (lowResI, lowResJ,
                                                                           depthlitho3PercentageMap - 1);
            }
            if (isNodeValid
                (temperaturePD, VreAtPresentDay, in_thickness, in_TOC, inorganicDensity,
                 Interface::DefaultUndefinedMapValue))
            {
	       if (!isNodeActive (VreAtPresentDay, in_thickness, in_TOC, inorganicDensity))
	       {
		  // not sure if this still constitutes an optimization.....
		  in_thickness = 0;
		  in_TOC = 0;
		  inorganicDensity = 0;
	       }

               Genex5::SourceRockNode * theNode =
                     new Genex5::SourceRockNode (in_thickness, in_TOC, inorganicDensity, sgMeanBulkDensity, lowResI, lowResJ);
               addNode (theNode);
	       if ( doApplyAdsorption ()) {
		  findNodeHistoryObjects ( theNode );
	       }
            }
         }
      }

#ifdef OBSOLETE
      if (Interface::MODE1D == theMode)
      {
         if (m_theNodes.empty ())
         {
	    // This is a perfectly legitimate situation!!!
            status = false;
            cout << "!!Warning!!: No valid Source Rock Nodes. Terminating preprocessing..." << endl;
         }
      }
#endif

      //restore local map data
      temperatureAtPresentDay->restoreData ();
      InputThickness->restoreData ();
      TOCmap->restoreData ();


      if (VREPresentDay && isVreOn)
      {
         VREPresentDay->restoreData ();
      }
      if (litho1PercentageMap)
      {
         litho1PercentageMap->restoreData ();
      }
      if (litho2PercentageMap)
      {
         litho2PercentageMap->restoreData ();
      }
      if (litho3PercentageMap)
      {
         litho3PercentageMap->restoreData ();
      }
      if (GetRank () == 0)
      {
         cout << "End of preprocessing." << endl;
         cout << "-------------------------------------" << endl;
      }
   }
   else
   {
      status = false;

      if (GetRank () == 0 && temperatureAtPresentDay == 0)
      {
         cout << "Unsuccessful upload of temperature property for layer : " << getLayerName () <<
               " Terminating preprocessing..." << endl;
      }
      else if (GetRank () == 0 && InputThickness == 0)
      {
         cout << "Unsuccessful upload of thickness for layer : " << getLayerName () << " Terminating preprocessing..."
               << endl;
      }
      else if (GetRank () == 0 && TOCmap == 0)
      {
         cout << "Unsuccessful upload of TOC for layer : " << getLayerName () << " Terminating preprocessing..." <<
               endl;
      }
      else
      {
         cout << "Terminating preprocessing..." << endl;
      }
   }

#if 0
   const GridMap* formationThickness = m_projectHandle->findFormation (this->getLayerName())->getInputThicknessMap ();

   if ( InputThickness != 0 and InputThickness != formationThickness ) {
      delete InputThickness;
   }
#endif

   return status;
}

void SourceRock::saveSourceRockNodeAdsorptionHistory () {

   SourceRockAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_sourceRockNodeAdsorptionHistory.begin (); histIter != m_sourceRockNodeAdsorptionHistory.end (); ++histIter ) {
      (*histIter)->save ();
   }

      }

void SourceRock::findNodeHistoryObjects ( Genex5::SourceRockNode * node ) {

   if ( m_adsorptionProjectHandle != 0 ) {
      PointAdsorptionHistoryList::iterator histIter;
      PointAdsorptionHistoryList::iterator endHist = m_adsorptionProjectHandle->getPointAdsorptionHistoryListEnd ();
      Genex5::NodeAdsorptionHistory* adsorptionHistory;

      const double originX = m_projectHandle->getActivityOutputGrid ()->minIGlobal ();
      const double originY = m_projectHandle->getActivityOutputGrid ()->minJGlobal ();

      const double deltaX = m_projectHandle->getActivityOutputGrid ()->deltaIGlobal ();
      const double deltaY = m_projectHandle->getActivityOutputGrid ()->deltaJGlobal ();

      for ( histIter = m_adsorptionProjectHandle->getPointAdsorptionHistoryListBegin (); histIter != endHist; ++histIter ) {

         const std::string& name = (*histIter)->getFormationName ();
         const double x = (*histIter)->getX ();
         const double y = (*histIter)->getY ();

         if ( name == getLayerName ()) {
            int i = int ( floor (( x - originX ) / deltaX + 0.5 ));
            int j = int ( floor (( y - originY ) / deltaY + 0.5 ));

            if ( i == (int)(node->GetI ()) and j == (int)(node->GetJ ())) {
               SourceRockAdsorptionHistory* history = new SourceRockAdsorptionHistory ( m_projectHandle, m_adsorptionProjectHandle, *histIter );
               adsorptionHistory = AdsorptionSimulatorFactory::getInstance ().allocateNodeAdsorptionHistory ( getAdsorptionSimulatorName () );

               if ( adsorptionHistory != 0 ) {
                  // Add the node-adsorption-history object to the sr-history-object.
                  history->setNodeAdsorptionHistory ( adsorptionHistory );
                  node->addNodeAdsorptionHistory ( adsorptionHistory );
                  m_sourceRockNodeAdsorptionHistory.push_back ( history );
               } else {
                  delete history;
               }

            }

         }

      }
   }
}


bool SourceRock::isNodeActive(const double &VreAtPresentDay, const double &in_thickness, const double &in_TOC, const double &inorganicDensity) const
{
   bool ret = true;

   // Only active if thickness is greater than 1cm.
   if ( in_thickness <= 0.01 ) ret = false;
   if (in_TOC < 0) ret = false;
   if (inorganicDensity < 0) ret = false;

   //and if the VRE optimization is enabled, 
   if (isVREoptimEnabled ())
   {
      //the VRE value at present day must be defined...
      static const double &VREthreshold = getVREthreshold ();

      //and above the VRE threshold that was defined through the GUI...
      if (VreAtPresentDay <= (VREthreshold - 0.001))
      {
	 ret = false;
      }
   }

   return ret;
}

bool SourceRock::isNodeValid(const double & temperatureAtPresentDay, const double &VreAtPresentDay, const double &thickness, const double &TOC, const double &inorganicDensity, const double &mapUndefinedValue) const
{
   using namespace Genex5;      //the fabsEqualDouble and EqualDouble are defined in Utitilies.h of genex5_kernel
   bool ret = false;

   //A node needs to be defined in low res...
   if (!fabsEqualDouble (temperatureAtPresentDay, mapUndefinedValue)
       && !fabsEqualDouble (VreAtPresentDay, mapUndefinedValue)
       && !fabsEqualDouble (thickness, mapUndefinedValue)
       && !fabsEqualDouble (TOC, mapUndefinedValue)
       && !fabsEqualDouble (inorganicDensity, mapUndefinedValue))
   {
      ret = true;
   }

   return ret;
}

bool SourceRock::process()
{
   bool status = true;
   if (m_theIntervals.size () == 0) return true;

   const SnapshotInterval *first   = m_theIntervals.front();
   const Snapshot *simulationStart = first->getStart();
   double t                        = simulationStart->getTime();
   double previousTime;
   double dt                       = m_theSimulator->GetMaximumTimeStepSize(m_depositionTime);

   if(GetRank() == 0)
   {      
      cout<<"Chosen timestep size:"<< dt <<endl;
      cout<<"-------------------------------------"<<endl;

      cout<<"Start Of processing..."<<endl;
      cout<<"-------------------------------------"<<endl;
   }
   
   //compute first snapshot 
   status = computeSnapShot( t + dt, simulationStart);

   if(status == false)
   {
      return status;
   }

   //increment time
   previousTime = t;
   t -= dt;
  
   std::vector<SnapshotInterval*>::iterator itSnapInterv ;

   for(itSnapInterv = m_theIntervals.begin(); itSnapInterv != m_theIntervals.end(); ++itSnapInterv)
   {
      const Snapshot *intervalStart = (*itSnapInterv)->getStart();
      const Snapshot *intervalEnd   = (*itSnapInterv)->getEnd();
      
//       double startTime = intervalStart->getTime ();
//       double endTime   = intervalEnd->getTime ();

//       cout << " interval times: " << startTime << "  " << endTime << endl;

      //can optimize here 
      LocalGridInterpolator *VESInterpolator  = new LocalGridInterpolator;
      LocalGridInterpolator *TempInterpolator = new LocalGridInterpolator;
      LocalGridInterpolator *lithostaticPressureInterpolator = new LocalGridInterpolator;
      LocalGridInterpolator *hydrostaticPressureInterpolator = new LocalGridInterpolator;
      LocalGridInterpolator *porePressureInterpolator = new LocalGridInterpolator;
      LocalGridInterpolator *porosityInterpolator = new LocalGridInterpolator;
      LocalGridInterpolator *permeabilityInterpolator = new LocalGridInterpolator;

      LocalGridInterpolator *vreInterpolator  = new LocalGridInterpolator;

      LocalGridInterpolator *ThicknessScalingInterpolator = 0;

      const GridMap *VESmapAtStart   =  getTopSurfacePropertyGridMap("Ves",intervalStart );
      const GridMap *VESmapAtEnd     =  getTopSurfacePropertyGridMap("Ves",intervalEnd );
      
      const GridMap *TempmapAtStart  =  getTopSurfacePropertyGridMap("Temperature",intervalStart );
      const GridMap *TempmapAtEnd    =  getTopSurfacePropertyGridMap("Temperature",intervalEnd );

      const GridMap *lithostaticPressureMapAtStart =  getTopSurfacePropertyGridMap ( "LithoStaticPressure", intervalStart );
      const GridMap *lithostaticPressureMapAtEnd   =  getTopSurfacePropertyGridMap ( "LithoStaticPressure", intervalEnd );

      const GridMap *hydrostaticPressureMapAtStart =  getTopSurfacePropertyGridMap ( "HydroStaticPressure", intervalStart );
      const GridMap *hydrostaticPressureMapAtEnd   =  getTopSurfacePropertyGridMap ( "HydroStaticPressure", intervalEnd );

      const GridMap *porePressureMapAtStart =  getTopSurfacePropertyGridMap ( "Pressure", intervalStart );
      const GridMap *porePressureMapAtEnd   =  getTopSurfacePropertyGridMap ( "Pressure", intervalEnd );

      const GridMap *porosityMapAtStart =  getSurfaceFormationPropertyGridMap ( "Porosity", intervalStart );
      const GridMap *porosityMapAtEnd   =  getSurfaceFormationPropertyGridMap ( "Porosity", intervalEnd );
      
      const GridMap *permeabilityMapAtStart =  getSurfaceFormationPropertyGridMap ( "Permeability", intervalStart );
      const GridMap *permeabilityMapAtEnd   =  getSurfaceFormationPropertyGridMap ( "Permeability", intervalEnd );

      const GridMap *vreAtStart   =  getTopSurfacePropertyGridMap("Vr",intervalStart );
      const GridMap *vreAtEnd     =  getTopSurfacePropertyGridMap("Vr",intervalEnd );

      if ( VESmapAtStart && VESmapAtEnd && TempmapAtStart && TempmapAtEnd and
           lithostaticPressureMapAtStart != 0 and lithostaticPressureMapAtEnd != 0 and
           hydrostaticPressureMapAtStart != 0 and hydrostaticPressureMapAtEnd != 0 and
           porePressureMapAtStart != 0 and porePressureMapAtEnd != 0 and 
           porosityMapAtStart != 0 and porosityMapAtEnd != 0 and
           permeabilityMapAtStart != 0 and permeabilityMapAtEnd != 0 and
           vreAtStart != 0 and vreAtEnd != 0 )
      {
         VESInterpolator ->compute(intervalStart, VESmapAtStart,  intervalEnd, VESmapAtEnd);
         TempInterpolator->compute(intervalStart, TempmapAtStart, intervalEnd, TempmapAtEnd); 
         vreInterpolator->compute  ( intervalStart, vreAtStart,     intervalEnd, vreAtEnd );

         lithostaticPressureInterpolator->compute(intervalStart, lithostaticPressureMapAtStart, intervalEnd, lithostaticPressureMapAtEnd ); 
         hydrostaticPressureInterpolator->compute(intervalStart, hydrostaticPressureMapAtStart, intervalEnd, hydrostaticPressureMapAtEnd ); 
         porePressureInterpolator->compute(intervalStart, porePressureMapAtStart, intervalEnd, porePressureMapAtEnd ); 
         porosityInterpolator->compute(intervalStart, porosityMapAtStart, intervalEnd, porosityMapAtEnd ); 
         permeabilityInterpolator->compute(intervalStart, permeabilityMapAtStart, intervalEnd, permeabilityMapAtEnd ); 


         //erosion 
         const GridMap *thicknessScalingAtStart    = getFormationPropertyGridMap("ErosionFactor",intervalStart );
         const GridMap *thicknessScalingAtEnd      = getFormationPropertyGridMap("ErosionFactor",intervalEnd );

         if(thicknessScalingAtStart && thicknessScalingAtEnd)
         {
	         ThicknessScalingInterpolator = new LocalGridInterpolator;
	         ThicknessScalingInterpolator->compute(intervalStart, thicknessScalingAtStart, intervalEnd, thicknessScalingAtEnd);  
         }

         double snapShotIntervalEndTime = intervalEnd->getTime();

         while(t > snapShotIntervalEndTime)
         {
            //within the interval just compute, do not save 
            computeTimeInstance ( previousTime, t,
                                  VESInterpolator,
                                  TempInterpolator,
                                  ThicknessScalingInterpolator,
                                  lithostaticPressureInterpolator,
                                  hydrostaticPressureInterpolator,
                                  porePressureInterpolator,
                                  porosityInterpolator,
                                  permeabilityInterpolator,
                                  vreInterpolator );
            previousTime = t;
            t -= dt;
         }
         //if t has passed Major snapshot time, compute snapshot and save results
         if( intervalEnd->getType() == Interface::MAJOR )
         {
            computeSnapShot(previousTime, intervalEnd);
         }    
      }
      else
      {
         status = false;
         if(GetRank() == 0)
         {
          cout << "Missing maps for the interval between  "<<intervalStart->getTime()<<" and "<< intervalEnd->getTime()<<endl;

          cout << " ves           " << (VESmapAtStart  == 0 ? " null " : " not null " ) << "  " << ( VESmapAtEnd  == 0 ? " null " : " not null " ) << endl;
          cout << " temperature   " << (TempmapAtStart  == 0 ? " null " : " not null " ) << "  " << ( TempmapAtEnd == 0 ? " null " : " not null "  ) << endl;
          cout << " lithostatic   " << (lithostaticPressureMapAtStart  == 0 ? " null " : " not null " ) << "  " << ( lithostaticPressureMapAtEnd  == 0 ? " null " : " not null " ) << endl;
          cout << " hysrostatic   " << (hydrostaticPressureMapAtStart  == 0 ? " null " : " not null " ) << "  " << ( hydrostaticPressureMapAtEnd  == 0 ? " null " : " not null " ) << endl;
          cout << " pore-pressure " << (porePressureMapAtStart  == 0 ? " null " : " not null " ) << "  " << ( porePressureMapAtEnd  == 0 ? " null " : " not null " ) << endl;
          cout << " porosity      " << (porosityMapAtStart  == 0 ? " null " : " not null "  ) << "  " << ( porosityMapAtEnd  == 0 ? " null " : " not null " ) << endl;
          cout << " permeability  " << (permeabilityMapAtStart  == 0 ? " null " : " not null "  ) << "  " << ( permeabilityMapAtEnd  == 0 ? " null " : " not null " ) << endl;
          cout << " vre           " << (vreAtStart  == 0 ? " null " : " not null "  ) << "  " << ( vreAtEnd  == 0 ? " null " : " not null " ) << endl;


          cout << "VESInterpolator "  << (VESInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "TempInterpolator "  << (TempInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "ThicknessScalingInterpolator "  << (ThicknessScalingInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "lithostaticPressureInterpolator "  << (lithostaticPressureInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "hydrostaticPressureInterpolator "  << (hydrostaticPressureInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "porePressureInterpolator "  << (porePressureInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "porosityInterpolator "  << (porosityInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "permeabilityInterpolator "  << (permeabilityInterpolator == 0 ? " null " : " not null " ) << endl;
          cout << "vreInterpolator "  << (vreInterpolator == 0 ? " null " : " not null " ) << endl;

          cout<<" ------------------------------------:"<<endl;
         }
         delete VESInterpolator;
         delete TempInterpolator;
         delete ThicknessScalingInterpolator;

         delete lithostaticPressureInterpolator;
         delete hydrostaticPressureInterpolator;
         delete porePressureInterpolator;
         delete porosityInterpolator;
         delete permeabilityInterpolator;

         delete vreInterpolator;

         break;
      }

      delete VESInterpolator;
      delete TempInterpolator;
      delete ThicknessScalingInterpolator;

      delete lithostaticPressureInterpolator;
      delete hydrostaticPressureInterpolator;
      delete porePressureInterpolator;
      delete porosityInterpolator;
      delete permeabilityInterpolator;
      delete vreInterpolator;

   }

   clearSimulator();
  
   if(status && GetRank() == 0)
   {
      cout<<"-------------------------------------"<<endl;
      cout<<"End of processing."<<endl;
      cout<<"-------------------------------------"<<endl;
   }
   
   saveSourceRockNodeAdsorptionHistory ();

   return status;
}
void SourceRock::initializeSnapShotOutputMaps()
{

   const vector<string> & theRequestedPropertyNames = ((GenexSimulator*)m_projectHandle)->getRequestedProperties();
   vector<string>::const_iterator it;
   for(it = theRequestedPropertyNames.begin(); it != theRequestedPropertyNames.end(); ++it)
   {

      if ( not ((GenexSimulator*)m_projectHandle)->isShaleGasProperty ( *it )) {
       m_theSnapShotOutputMaps[(*it)] =  0; 
   }

   }

   m_hcSaturationOutputMap = 0;
   m_irreducibleWaterSaturationOutputMap = 0;

   m_gasVolumeOutputMap = 0;
   m_oilVolumeOutputMap = 0;
   m_gasExpansionRatio = 0;
   m_gasGeneratedFromOtgc = 0;
   m_totalGasGenerated = 0;
   m_fracOfAdsorptionCap = 0;
   m_hcLiquidSaturation = 0;
   m_hcVapourSaturation = 0;
   m_adsorptionCapacity = 0;

   m_retainedOilApiOutputMap = 0;
   m_retainedCondensateApiOutputMap = 0;

   m_retainedGor = 0;
   m_retainedCgr = 0;

   if ( m_theSimulator != 0 ) {

      if ( doApplyAdsorption ()) {
         const AdsorptionSimulator* adsorptionSimulator = m_theSimulator->getAdsorptionSimulator ();

         unsigned int speciesIndex;

         for (speciesIndex = 0; speciesIndex < ComponentManager::NumberOfOutputSpecies; ++speciesIndex) {
            ComponentManager::SpeciesNamesId species = ComponentManager::SpeciesNamesId ( speciesIndex );

            if ( adsorptionSimulator->speciesIsSimulated ( species )) {

               m_adsorpedOutputMaps [ species ] = 0;
#if ADSORPED_EXPELLED_MAPS
               m_expelledOutputMaps [ species ] = 0;
#endif
//                m_freeOutputMaps [ species ] = 0;
            }

            m_sourceRockExpelledOutputMaps [ species ] = 0;
            m_retainedOutputMaps [ species ] = 0;
         }

         m_hcSaturationOutputMap = 0;
         m_irreducibleWaterSaturationOutputMap = 0;

         m_gasVolumeOutputMap = 0;
         m_oilVolumeOutputMap = 0;
         m_gasExpansionRatio = 0;
         m_gasGeneratedFromOtgc = 0;
         m_totalGasGenerated = 0;
         m_fracOfAdsorptionCap = 0;
         m_hcLiquidSaturation = 0;
         m_hcVapourSaturation = 0;
         m_adsorptionCapacity = 0;
         m_retainedOilApiOutputMap = 0;
         m_retainedCondensateApiOutputMap = 0;
         m_retainedGor = 0;
         m_retainedCgr = 0;
      }

   }

}
void SourceRock::createSnapShotOutputMaps(const Snapshot *theSnapshot)
{

   std::map<std::string, GridMap*>::iterator it;
   for(it = m_theSnapShotOutputMaps.begin(); it != m_theSnapShotOutputMaps.end(); ++it)
   { 
      GridMap *theMap = createSnapshotResultPropertyValueMap(it->first, theSnapshot);
      if(theMap)
      {   
        it->second = theMap;
       (it->second)->retrieveData();
      }
      else
      {
         if(GetRank() == 0)
         {
            cout<<"Unsuccessful creation of map :"<<it->first<<endl;
         }
      }
   }


   if ( doApplyAdsorption ()) {
      GridMap* theMap;
      unsigned int speciesIndex;
      const AdsorptionSimulator* adsorptionSimulator = m_theSimulator->getAdsorptionSimulator ();

      m_hcSaturationOutputMap = createSnapshotResultPropertyValueMap ( "HcSaturation",
                                                                       theSnapshot );

      if ( m_hcSaturationOutputMap != 0 ) {
         m_hcSaturationOutputMap->retrieveData ();
      }

      m_irreducibleWaterSaturationOutputMap = createSnapshotResultPropertyValueMap ( "ImmobileWaterSat",
                                                                                     theSnapshot );

      if ( m_irreducibleWaterSaturationOutputMap != 0 ) {
         m_irreducibleWaterSaturationOutputMap->retrieveData ();
      }

      m_adsorptionCapacity = createSnapshotResultPropertyValueMap ( "AdsorptionCapacity",
                                                                    theSnapshot );

      if ( m_adsorptionCapacity != 0 ) {
         m_adsorptionCapacity->retrieveData ();
      }


      m_fracOfAdsorptionCap = createSnapshotResultPropertyValueMap ( "FractionOfAdsorptionCap",
                                                                     theSnapshot );

      if ( m_fracOfAdsorptionCap != 0 ) {
         m_fracOfAdsorptionCap->retrieveData ();
      } 

      m_hcLiquidSaturation = createSnapshotResultPropertyValueMap ( "HcLiquidSat",
                                                                    theSnapshot );

      if ( m_hcLiquidSaturation != 0 ) {
         m_hcLiquidSaturation->retrieveData ();
      }

      m_hcVapourSaturation = createSnapshotResultPropertyValueMap ( "HcVapourSat",
                                                                    theSnapshot );

      if ( m_hcVapourSaturation != 0 ) {
         m_hcVapourSaturation->retrieveData ();
      }

      m_gasVolumeOutputMap = createSnapshotResultPropertyValueMap ( "RetainedGasVolumeST",
                                                                    theSnapshot );

      if ( m_gasVolumeOutputMap != 0 ) {
         m_gasVolumeOutputMap->retrieveData ();
      }

      m_oilVolumeOutputMap = createSnapshotResultPropertyValueMap ( "RetainedOilVolumeST",
                                                                    theSnapshot );

      if ( m_oilVolumeOutputMap != 0 ) {
         m_oilVolumeOutputMap->retrieveData ();
      }

      m_gasExpansionRatio = createSnapshotResultPropertyValueMap ( "GasExpansionRatio_Bg",
                                                                   theSnapshot );
      
      if ( m_gasExpansionRatio != 0 ) {
         m_gasExpansionRatio->retrieveData ();
      }

      m_gasGeneratedFromOtgc = createSnapshotResultPropertyValueMap ( "Oil2GasGeneratedCumulative",
                                                                      theSnapshot );
      
      if ( m_gasGeneratedFromOtgc != 0 ) {
         m_gasGeneratedFromOtgc->retrieveData ();
      }

      m_totalGasGenerated = createSnapshotResultPropertyValueMap ( "TotalGasGeneratedCumulative",
                                                                   theSnapshot );
      
      if ( m_totalGasGenerated != 0 ) {
         m_totalGasGenerated->retrieveData ();
      }

      m_retainedOilApiOutputMap = createSnapshotResultPropertyValueMap ( "RetainedOilApiSR",
                                                                         theSnapshot );

      if ( m_retainedOilApiOutputMap != 0 ) {
         m_retainedOilApiOutputMap->retrieveData ();
      }

      m_retainedCondensateApiOutputMap = createSnapshotResultPropertyValueMap ( "RetainedCondensateApiSR",
                                                                                theSnapshot );

      if ( m_retainedCondensateApiOutputMap != 0 ) {
         m_retainedCondensateApiOutputMap->retrieveData ();
      }

      m_retainedGor = createSnapshotResultPropertyValueMap ( "RetainedGorSR",
                                                             theSnapshot );

      if ( m_retainedGor != 0 ) {
         m_retainedGor->retrieveData ();
      }

      m_retainedCgr = createSnapshotResultPropertyValueMap ( "RetainedCgrSR",
                                                             theSnapshot );

      if ( m_retainedCgr != 0 ) {
         m_retainedCgr->retrieveData ();
      }


      for (speciesIndex = 0; speciesIndex < ComponentManager::NumberOfOutputSpecies; ++speciesIndex) {
         ComponentManager::SpeciesNamesId species = ComponentManager::SpeciesNamesId ( speciesIndex );

         theMap = createSnapshotResultPropertyValueMap ( ComponentManager::getInstance().GetSpeciesName ( speciesIndex ) + "Retained",
                                                         theSnapshot );

         if ( theMap != 0 ) {
            m_retainedOutputMaps [ species ] = theMap;
            theMap->retrieveData ();
         }

         theMap = createSnapshotResultPropertyValueMap ( ComponentManager::getInstance().getSpeciesSourceRockExpelledByName ( speciesIndex ),
                                                         theSnapshot );

         if ( theMap != 0 ) {
            m_sourceRockExpelledOutputMaps [ species ] = theMap;
            theMap->retrieveData ();
         }

         if ( adsorptionSimulator->speciesIsSimulated ( species )) {
            theMap = createSnapshotResultPropertyValueMap ( adsorptionSimulator->getAdsorpedSpeciesName ( species ), theSnapshot );

            if ( theMap != 0 ) {
               m_adsorpedOutputMaps [ species ] = theMap;
               theMap->retrieveData ();
            }

#if ADSORPED_EXPELLED_MAPS
            theMap = createSnapshotResultPropertyValueMap ( adsorptionSimulator->getExpelledSpeciesName ( species ), theSnapshot );

            if ( theMap != 0 ) {
               m_expelledOutputMaps [ species ] = theMap;
               theMap->retrieveData ();
            }
#endif

#if 0
            theMap = createSnapshotResultPropertyValueMap ( adsorptionSimulator->getFreeSpeciesName ( species ), theSnapshot );

            if ( theMap != 0 ) {
               m_freeOutputMaps [ species ] = theMap;
               theMap->retrieveData ();
            }
#endif

         }

      }

   }


}
void SourceRock::saveSnapShotOutputMaps(const Snapshot *theSnapshot)
{
   std::map<std::string, GridMap*>::iterator it;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* >::iterator adsorpedIt;

   for(it = m_theSnapShotOutputMaps.begin(); it != m_theSnapShotOutputMaps.end(); ++it)
   {
      if(it->second)
      {
         (it->second)->restoreData();
         (it->second) = 0;
      }
   }  

   if ( m_hcSaturationOutputMap != 0 ) {
      m_hcSaturationOutputMap->restoreData ();
      m_hcSaturationOutputMap = 0;
   }

   if ( m_irreducibleWaterSaturationOutputMap != 0 ) {
      m_irreducibleWaterSaturationOutputMap->restoreData ();
      m_irreducibleWaterSaturationOutputMap = 0;
   }

   if ( m_adsorptionCapacity != 0 ) {
      m_adsorptionCapacity->restoreData ();
      m_adsorptionCapacity = 0;
   }

   if ( m_fracOfAdsorptionCap != 0 ) {
      m_fracOfAdsorptionCap->restoreData ();
      m_fracOfAdsorptionCap = 0;
   }

   if ( m_hcLiquidSaturation != 0 ) {
      m_hcLiquidSaturation->restoreData ();
      m_hcLiquidSaturation = 0;
   }

   if ( m_hcVapourSaturation != 0 ) {
      m_hcVapourSaturation->restoreData ();
      m_hcVapourSaturation = 0;
   }

   if ( m_gasVolumeOutputMap != 0 ) {
      m_gasVolumeOutputMap->restoreData ();
      m_gasVolumeOutputMap = 0;
   }


   if ( m_oilVolumeOutputMap != 0 ) {
      m_oilVolumeOutputMap->restoreData ();
      m_oilVolumeOutputMap = 0;
   }

   if ( m_gasExpansionRatio != 0 ) {
      m_gasExpansionRatio->restoreData ();
      m_gasExpansionRatio = 0;
   }

   if ( m_gasGeneratedFromOtgc != 0 ) {
      m_gasGeneratedFromOtgc->restoreData ();
      m_gasGeneratedFromOtgc = 0;
   }

   if ( m_totalGasGenerated != 0 ) {
      m_totalGasGenerated->restoreData ();
      m_totalGasGenerated = 0;
   }

   if ( m_retainedOilApiOutputMap != 0 ) {
      m_retainedOilApiOutputMap->restoreData ();
      m_retainedOilApiOutputMap = 0;
   }

   if ( m_retainedCondensateApiOutputMap != 0 ) {
      m_retainedCondensateApiOutputMap->restoreData ();
      m_retainedCondensateApiOutputMap = 0;
   }

   if ( m_retainedGor != 0 ) {
      m_retainedGor->restoreData ();
      m_retainedGor = 0;
   }

   if ( m_retainedCgr != 0 ) {
      m_retainedCgr->restoreData ();
      m_retainedCgr = 0;
   }

   for ( adsorpedIt = m_sourceRockExpelledOutputMaps.begin (); adsorpedIt != m_sourceRockExpelledOutputMaps.end (); ++adsorpedIt ) {

      if ( adsorpedIt->second ) {
         (adsorpedIt->second)->restoreData ();
         (adsorpedIt->second) = 0;
      }

   }

   for ( adsorpedIt = m_retainedOutputMaps.begin (); adsorpedIt != m_retainedOutputMaps.end (); ++adsorpedIt ) {

      if ( adsorpedIt->second ) {
         (adsorpedIt->second)->restoreData ();
         (adsorpedIt->second) = 0;
      }

   }

   for ( adsorpedIt = m_adsorpedOutputMaps.begin (); adsorpedIt != m_adsorpedOutputMaps.end (); ++adsorpedIt ) {

      if ( adsorpedIt->second ) {
         (adsorpedIt->second)->restoreData ();
         (adsorpedIt->second) = 0;
      }

   }

#if ADSORPED_EXPELLED_MAPS
   for ( adsorpedIt = m_expelledOutputMaps.begin (); adsorpedIt != m_expelledOutputMaps.end (); ++adsorpedIt ) {

      if ( adsorpedIt->second ) {
         (adsorpedIt->second)->restoreData ();
         (adsorpedIt->second) = 0;
      }

   }
#endif

}
void SourceRock::updateSnapShotOutputMaps(Genex5::SourceRockNode *theNode)
{

   ComponentManager & theManager = ComponentManager::getInstance();
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   const double meanBulkDensity = theNode->getSGMeanBulkDensity ();

   // Converts cubic-metres to barrels.
   const double CubicMetresToBarrel = 6.2898107704; 

   // The volume of a mole of methane at surface conditions (mol/m^3).
   const double VolumeMoleMethaneAtSurfaceConditions = 42.306553;

   // Converts cubic-metres to cubic-feet.
   const double CubicMetresToCubicFeet = 35.3146667;

   // Converts kg to US ton.
   const double KilogrammeToUSTon = 1.0 / 907.18474;

   // Converts m^3/m^3 to ft^3/ton.
   const double SCFpTonGasVolumeConversionFactor = CubicMetresToCubicFeet / ( KilogrammeToUSTon * meanBulkDensity );

   // Converts m^3/m^2 -> bcf/km^2
   const double BCFpKm2GasVolumeConversionFactor = CubicMetresToCubicFeet / 1.0e3; // = 1.0e6 * m^3->f^3 / 1.0e9.

   // Convert m^3/m^2 -> mega barrel/km^2.
   const double OilVolumeConversionFactor = 1.0e6 * CubicMetresToBarrel / 1.0e6;

   const double GorConversionFactor = CubicMetresToCubicFeet / CubicMetresToBarrel;

   const double CgrConversionFactor = CubicMetresToBarrel / ( 1.0e-3 * CubicMetresToCubicFeet );

   const unsigned int i = theNode->GetI ();
   const unsigned int j = theNode->GetJ ();

   const Genex5::SourceRockNodeInput* nodeInputData = theNode->getTheLastInput ();

   double thicknessScaling = ( nodeInputData == 0 ? 1.0 : nodeInputData->GetThicknessScaleFactor ());

   const SourceRockNodeOutput *theOutput = theNode->GetTheLastOutput();

   std::map<std::string, GridMap*>::iterator it;
   std::map<std::string, GridMap*>::iterator snapshotMapContainerEnd = m_theSnapShotOutputMaps.end();
 
   double gasVolume;
   double oilVolume;
   double gasExpansionRatio;
   double oilApi;
   double condensateApi;
   double gor;
   double cgr;
 
   //first the mandatory
   int speciesIndex;
   for (speciesIndex = 0; speciesIndex < ComponentManager::NumberOfOutputSpecies; ++speciesIndex)
   {
      it = m_theSnapShotOutputMaps.find(theManager.GetSpeciesOutputPropertyName(speciesIndex));

      if(it != snapshotMapContainerEnd)
      {               
         Genex5::SpeciesResult *theResult = theOutput->GetSpeciesResultByName(theManager.GetSpeciesName(speciesIndex));
         if(theResult) 
         {
            (it->second)->setValue(theNode->GetI(), theNode->GetJ(), theResult->GetExpelledMass()); 
         }
         else
         {
            if(GetRank() == 0)
            {
               cout<<"Genex5 Simulator does not support species:"<<theManager.GetSpeciesName(speciesIndex)<<endl;
            }
         }
      }
   }
   //then the optional 
   int resultIndex = 0;
   for(resultIndex = 0; resultIndex < GenexResultManager::NumberOfResults; ++resultIndex)
   {
      if(theResultManager.IsResultRequired(resultIndex))
      {
         it = m_theSnapShotOutputMaps.find(theResultManager.GetResultName(resultIndex));

         if(it != snapshotMapContainerEnd )
         { 
            (it->second)->setValue(theNode->GetI(), theNode->GetJ(), theOutput->GetResult( resultIndex ));
         }
      }
   } 

   if ( doApplyAdsorption ()) {
      const AdsorptionSimulator* adsorptionSimulator = m_theSimulator->getAdsorptionSimulator ();

      m_irreducibleWaterSaturationOutputMap->setValue ( i, j, theNode->getSimulatorState ()->getIrreducibleWaterSaturation ());
      m_hcSaturationOutputMap->setValue ( i, j, theNode->getSimulatorState ()->getHcSaturation ());

      computeHcVolumes ( *theNode, gasVolume, oilVolume, gor, cgr, gasExpansionRatio, oilApi, condensateApi );

      m_gasVolumeOutputMap->setValue ( i, j, gasVolume * BCFpKm2GasVolumeConversionFactor );
      m_oilVolumeOutputMap->setValue ( i, j, oilVolume * OilVolumeConversionFactor );
      m_gasExpansionRatio->setValue ( i, j, gasExpansionRatio );

      if ( m_retainedGor != 0 and gor != 99999.0 ) {
         m_retainedGor->setValue ( i, j, gor * GorConversionFactor );
      } else if ( m_retainedGor != 0 ) {
         m_retainedGor->setValue ( i, j, 99999.0 );
      }

      if ( m_retainedCgr != 0 and cgr != 99999.0 ) {
         m_retainedCgr->setValue ( i, j, cgr * CgrConversionFactor );
      } else if ( m_retainedCgr != 0 ) {
         m_retainedCgr->setValue ( i, j, 99999.0 );
      }

      m_gasGeneratedFromOtgc->setValue ( i, j, theNode->getSimulatorState ()->getTotalGasFromOtgc ());
      m_totalGasGenerated->setValue ( i, j, theNode->getSimulatorState ()->getTotalGasFromOtgc () +
                                            theOutput->GetResult ( CBMGenerics::GenexResultManager::HcGasGeneratedCum ));

      if ( m_retainedOilApiOutputMap != 0 ) {
         m_retainedOilApiOutputMap->setValue ( i, j, oilApi );
      }

      if ( m_retainedCondensateApiOutputMap != 0 ) {
         m_retainedCondensateApiOutputMap->setValue ( i, j, condensateApi );
      }

      if ( theNode->getSimulatorState ()->getUsablePorosity () != 0.0 ) {
         m_hcLiquidSaturation->setValue ( i, j, 
                                          theNode->getSimulatorState ()->getRetainedLiquidVolume () / theNode->getSimulatorState ()->getUsablePorosity ());
         m_hcVapourSaturation->setValue ( i, j,
                                          theNode->getSimulatorState ()->getRetainedVapourVolume () / theNode->getSimulatorState ()->getUsablePorosity ());
      } else {
         m_hcLiquidSaturation->setValue ( i, j, 0.0 );
         m_hcVapourSaturation->setValue ( i, j, 0.0 );
      }

      for (speciesIndex = 0; speciesIndex < ComponentManager::NumberOfOutputSpecies; ++speciesIndex) {
         ComponentManager::SpeciesNamesId species = ComponentManager::SpeciesNamesId ( speciesIndex );
         
         const SpeciesState* speciesState = theNode->getSimulatorState ()->GetSpeciesStateByName ( theManager.GetSpeciesName ( speciesIndex ));

         if ( adsorptionSimulator->speciesIsSimulated ( species )) {
            Genex5::SpeciesResult* result = theOutput->GetSpeciesResultByName(theManager.GetSpeciesName(speciesIndex));

            m_adsorpedOutputMaps [ species ]->setValue ( i, j, SCFpTonGasVolumeConversionFactor * result->getAdsorpedMol () / VolumeMoleMethaneAtSurfaceConditions);
#if ADSORPED_EXPELLED_MAPS
            m_expelledOutputMaps [ species ]->setValue ( i, j, SCFpTonGasVolumeConversionFactor * result->getExpelledMol () / VolumeMoleMethaneAtSurfaceConditions);
#endif

            // POTENTIAL problem here, if more than one species is considered for adsorption then this map will be overwritten.
            if ( speciesState->getAdsorptionCapacity () != 0.0 ) {
               m_fracOfAdsorptionCap->setValue ( i, j, ( result->getAdsorpedMol () / VolumeMoleMethaneAtSurfaceConditions ) / speciesState->getAdsorptionCapacity ());
            } else {
               m_fracOfAdsorptionCap->setValue ( i, j, 0.0 );
            }

            m_adsorptionCapacity->setValue ( i, j, speciesState->getAdsorptionCapacity () * SCFpTonGasVolumeConversionFactor  );
         }

         m_sourceRockExpelledOutputMaps [ species ]->setValue ( i, j, speciesState->getMassExpelledFromSourceRock ());
         m_retainedOutputMaps [ species ]->setValue ( i, j, speciesState->getRetained ());

//          if ( adsorptionSimulator->speciesRetentionSimulated ( species )) {
//             Genex5::SpeciesResult* result = theOutput->GetSpeciesResultByName(theManager.GetSpeciesName(speciesIndex));

//             m_retainedOutputMaps [ species ]->setValue ( i, j, result->getRetained () / VolumeMoleMethaneAtSurfaceConditions );
//          }

      }

   }

}

bool SourceRock::computeSnapShot ( const double previousTime,
                                   const Snapshot *theSnapshot )
{
   bool status = true;
   double time  = theSnapshot->getTime();
   if(GetRank() == 0)
   {
      cout<<"Computing SnapShot t:"<<time<<endl;
   }

   //Load VES, temperature maps 
   const GridMap *VES  = getTopSurfacePropertyGridMap("Ves",theSnapshot);
   const GridMap *Temp = getTopSurfacePropertyGridMap("Temperature", theSnapshot); 

   const GridMap *lithostaticPressure = getTopSurfacePropertyGridMap("LithoStaticPressure", theSnapshot); 
   const GridMap *hydrostaticPressure = getTopSurfacePropertyGridMap("HydroStaticPressure", theSnapshot); 
   const GridMap *porePressure = getTopSurfacePropertyGridMap("Pressure", theSnapshot); 
   const GridMap *porosity = getSurfaceFormationPropertyGridMap("Porosity", theSnapshot); 
   const GridMap *permeability = getSurfaceFormationPropertyGridMap("Permeability", theSnapshot); 
   const GridMap *vre = getTopSurfacePropertyGridMap("Vr", theSnapshot); 

   //erosion
   const GridMap *thicknessScaling = getFormationPropertyGridMap("ErosionFactor",theSnapshot );

   unsigned int depthVES = 1;
   unsigned int depthTemp = 1;
   unsigned int depthThicknessScaling = 1;

   if(VES)
   {
      VES->retrieveData();
      depthVES = VES->getDepth ();
   }

   if(Temp)
   {
      Temp->retrieveData();
      depthTemp = Temp->getDepth();
   }

   if(thicknessScaling)
   {
      thicknessScaling->retrieveData();
      depthTemp = thicknessScaling->getDepth();
   }

   if ( lithostaticPressure != 0 ) {
      lithostaticPressure->retrieveData ();
   }

   if ( hydrostaticPressure != 0 ) {
      hydrostaticPressure->retrieveData ();
   }

   if ( porePressure != 0 ) {
      porePressure->retrieveData ();
   }

   if ( porosity != 0 ) {
      porosity->retrieveData ();
   }

   if ( permeability != 0 ) {
      permeability->retrieveData ();
   }

   if ( vre != 0 ) {
      vre->retrieveData ();
   }

   if(VES && Temp)
   {
      createSnapShotOutputMaps(theSnapshot);

      bool useMaximumVes = isVESMaxEnabled();
      double maximumVes = getVESMax();
      maximumVes *= Genex5::Constants::convertMpa2Pa;

      //need to optimize..
      std::vector<Genex5::SourceRockNode*>::iterator itNode;
      for(itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++itNode)
      { 
         double in_VES  =  VES->getValue( (*itNode)->GetI(), (*itNode)->GetJ(), depthVES - 1);
         if(useMaximumVes && in_VES > maximumVes)
         {
            in_VES = maximumVes;
         }
         double in_Temp =  Temp->getValue(( *itNode)->GetI(), (*itNode)->GetJ(), depthTemp - 1);
         double in_thicknessScaling = thicknessScaling ? thicknessScaling->getValue(( *itNode)->GetI(), (*itNode)->GetJ(), depthThicknessScaling - 1 ) : 1.0;

         double nodeLithostaticPressure = 1.0e6 * lithostaticPressure->getValue ( (*itNode)->GetI(), (*itNode)->GetJ());
         double nodeHydrostaticPressure = 1.0e6 * hydrostaticPressure->getValue ( (*itNode)->GetI(), (*itNode)->GetJ());
         double nodePorePressure = 1.0e6 * porePressure->getValue ( (*itNode)->GetI(), (*itNode)->GetJ());
         double nodePorosity = 0.01 * porosity->getValue ( (*itNode)->GetI(), (*itNode)->GetJ());
         double nodePermeability = permeability->getValue ( (*itNode)->GetI(), (*itNode)->GetJ());
         double nodeVre = vre->getValue ( (*itNode)->GetI(), (*itNode)->GetJ());

         Genex5::SourceRockNodeInput *theInput = new Genex5::SourceRockNodeInput ( previousTime, time,
                                                                                   in_Temp,
                                                                                   in_Temp,
                                                                                   in_VES, 
                                                                                   nodeLithostaticPressure,
                                                                                   nodeHydrostaticPressure,
                                                                                   nodePorePressure,
                                                                                   nodePorePressure,
                                                                                   nodePorosity,
                                                                                   nodePermeability,
                                                                                   nodeVre,
                                                                                   (*itNode)->GetI (),
                                                                                   (*itNode)->GetJ (),
                                                                                   in_thicknessScaling );
         

         (*itNode)->AddInput(theInput);
         (*itNode)->RequestComputation(*m_theSimulator, true );

         updateSnapShotOutputMaps( (*itNode) );

         (*itNode)->ClearInputHistory();
         (*itNode)->ClearOutputHistory();
      }

      saveSnapShotOutputMaps(theSnapshot);

      VES->restoreData();
      Temp->restoreData();

      if(thicknessScaling)
      {
         thicknessScaling->restoreData();
      }
   } 
   else
   {
      status = false;

      if(Temp == 0)
      {    
         if(GetRank() == 0)
         {
            cout<<"!!Error!!: Missing Temperature map for the shapshot  :"<<time<<". Aborting... "<<endl;
            cout<<" ------------------------------------:"<<endl;
         }
      }
      if(VES == 0)
      {      
         if(GetRank() == 0)
         {
         cout<<"!!Error!!: Missing VES map for the shapshot  :"<<time<<". Aborting... "<<endl;
         cout<<" ------------------------------------:"<<endl;
         }
      }
   }
   return status;
}

void SourceRock::computeTimeInstance ( const double &startTime,
                                       const double &endTime,
                                       const LocalGridInterpolator* ves,
                                       const LocalGridInterpolator* temperature,
                                       const LocalGridInterpolator* thicknessScaling,
                                       const LocalGridInterpolator* lithostaticPressure,
                                       const LocalGridInterpolator* hydrostaticPressure,
                                       const LocalGridInterpolator* porePressure,
                                       const LocalGridInterpolator* porosity,
                                       const LocalGridInterpolator* permeability,
                                       const LocalGridInterpolator* vre )
{   
   bool useMaximumVes = isVESMaxEnabled();
   double maximumVes = getVESMax();
   maximumVes *= Genex5::Constants::convertMpa2Pa;

   std::vector<Genex5::SourceRockNode*>::iterator itNode;

   for(itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++itNode)
   {
      double in_VES  = ves->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );

      if(useMaximumVes && in_VES > maximumVes)
      {
	      in_VES = maximumVes;
      }

      double in_startTemp = temperature->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), startTime );
      double in_endTemp = temperature->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );

      double nodeLithostaticPressure = 1.0e6 * lithostaticPressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );
      double nodeHydrostaticPressure = 1.0e6 * hydrostaticPressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );
      double startNodePorePressure = 1.0e6 * porePressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), startTime );
      double endNodePorePressure = 1.0e6 * porePressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );
      double nodePorosity = 0.01 * porosity->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );
      double nodePermeability = permeability->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );
      double nodeVre          = vre->evaluateProperty ( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );

      double in_thicknessScaling = thicknessScaling ? thicknessScaling->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime ) : 1.0;

      Genex5::SourceRockNodeInput *theInput = new Genex5::SourceRockNodeInput ( startTime, endTime,
                                                                                in_startTemp,
                                                                                in_endTemp,
                                                                                in_VES,
                                                                                nodeLithostaticPressure,
                                                                                nodeHydrostaticPressure,
                                                                                startNodePorePressure,
                                                                                endNodePorePressure,
                                                                                nodePorosity,
                                                                                nodePermeability,
                                                                                nodeVre,
                                                                                (*itNode)->GetI (),
                                                                                (*itNode)->GetJ (),
                                                                                in_thicknessScaling );
      (*itNode)->AddInput(theInput);
      (*itNode)->RequestComputation(*m_theSimulator);
      (*itNode)->ClearInputHistory();
      (*itNode)->ClearOutputHistory();
   } 
}

GridMap *SourceRock::createSnapshotResultPropertyValueMap(const std::string &propertyName, const Snapshot *theSnapshot)
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());

   PropertyValue *thePropertyValue = m_projectHandle->createMapPropertyValue (propertyName, 
                                                                              theSnapshot, 0, 
                                                                              theFormation,0);
   GridMap *theMap = 0;
   if(thePropertyValue)
   {
      theMap = thePropertyValue->getGridMap();
   }
   return theMap;   
}

void SourceRock::addNode(Genex5::SourceRockNode* in_Node)
{
   m_theNodes.push_back(in_Node);
}
double SourceRock::getLithoDensity(const LithoType *theLitho) const
{
   Interface::LithoTypeAttributeId theId = Interface::Density;
   const AttributeValue theDensity = theLitho->getAttributeValue(theId);
   double density = theDensity.getDouble();
   return density;
}
const GridMap * SourceRock::getLithoType1PercentageMap() const
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());
   const GridMap *type1 = theFormation->getLithoType1PercentageMap();
   return type1; 
}
const GridMap * SourceRock::getLithoType2PercentageMap() const
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());
   const GridMap *type2 = theFormation->getLithoType2PercentageMap();
   return type2; 
}

const GridMap * SourceRock::getLithoType3PercentageMap() const
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());
   const GridMap *type3 = theFormation->getLithoType3PercentageMap();
   return type3; 
}
const LithoType * SourceRock::getLithoType1() const 
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());
   const LithoType *type1 = theFormation-> getLithoType1 (); 
   return type1; 
}
const LithoType * SourceRock::getLithoType2() const 
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());
   const LithoType *type2 = theFormation-> getLithoType2 (); 
   return type2; 
}
const LithoType * SourceRock::getLithoType3() const 
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());
   const LithoType *type3 = theFormation-> getLithoType3 (); 
   return type3; 
}

const GridMap * SourceRock::getInputThicknessGridMap () const
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());   
   return theFormation->getInputThicknessMap();
}

#if 0
const GridMap * SourceRock::getInputThicknessGridMap () const
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());   
   const Snapshot* snapshot = theFormation->getTopSurface ()->getSnapshot ();   

   const GridMap* topDepth = getPropertyGridMap ("Depth", snapshot, 0, 0, theFormation->getTopSurface ());
   const GridMap* bottomDepth = getPropertyGridMap ("Depth", snapshot, 0, 0, theFormation->getBottomSurface ());

   if ( topDepth != 0 and bottomDepth != 0 ) {
      Interface::SubtractionFunctor minus;

      // The thickness map is the bottomDepth - topDepth.
      return m_projectHandle->getFactory()->produceGridMap ( 0, 0, bottomDepth, topDepth, minus );
   } else {
      return theFormation->getInputThicknessMap();
   }

}
#endif

const GridMap * SourceRock::getTopSurfacePropertyGridMap (const string & propertyName, const Interface::Snapshot * snapshot) const
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());   
   return getPropertyGridMap (propertyName, snapshot, 0, 0, theFormation->getTopSurface ());
}
const GridMap * SourceRock::getFormationPropertyGridMap (const string & propertyName,const Interface::Snapshot * snapshot) const
{
  return getPropertyGridMap (propertyName, snapshot, 0, m_projectHandle->findFormation (this->getLayerName()), 0);
}

const GridMap * SourceRock::getSurfaceFormationPropertyGridMap (const string & propertyName,const Interface::Snapshot * snapshot) const
{
   const Formation *theFormation = m_projectHandle->findFormation (this->getLayerName());   

   const GridMap * result;

//    cout << " getSurfaceFormationPropertyGridMap " 
//         << propertyName << "  " 
//         << theFormation->getName () << "  "
//         << snapshot->getTime () << "  "
//         << ( theFormation->getTopSurface () == 0 ? " NULL SURFACE " : theFormation->getTopSurface ()->getName ())
//         << endl;

   

   result =  getPropertyGridMap (propertyName, snapshot, 0, theFormation, theFormation->getTopSurface ());

//    cout << "result = " << result  << endl;
   return result;

//    return getPropertyGridMap (propertyName, snapshot, 0, theFormation, theFormation->getTopSurface ());
}
const GridMap * SourceRock::getPropertyGridMap (const string & propertyName,
      const Interface::Snapshot * snapshot,
                                                const Interface::Reservoir * reservoir,
                                                const Formation * formation,
                                                const Surface * surface) const
{
   int selectionFlags = 0;

   if (reservoir) selectionFlags |= Interface::RESERVOIR;
   if (formation && !surface) selectionFlags |= Interface::FORMATION;
   if (surface && !formation) selectionFlags |= Interface::SURFACE;
   if (formation && surface) selectionFlags |= Interface::FORMATIONSURFACE;

   PropertyValueList * propertyValues = m_projectHandle->getPropertyValues (  selectionFlags,
                                                                              m_projectHandle->findProperty (propertyName),
	                                                                           snapshot, 
                                                                              reservoir, 
                                                                              formation, 
                                                                              surface,
	                                                                           Interface::MAP);     
	    
   if (propertyValues->size () != 1)
   {

//       int i;


//       for ( i = 0; i < propertyValues->size (); ++i ) {
//          cout << " propertyvalue: " << i << endl;
//          (*propertyValues)[ i ]->printOn ( cout );
//          cout << endl;
//          cout << endl;
//       }


      return 0;
   }

   const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

   delete propertyValues;
   return gridMap;
}
void SourceRock::computeSnapshotIntervals ()
{
   m_depositionTime = getDepositionTime ();

   Interface::SnapshotList * snapshots = m_projectHandle->getSnapshots (Interface::MINOR | Interface::MAJOR);

   Interface::SnapshotList::reverse_iterator snapshotIter;

   const Interface::Snapshot * start;
   const Interface::Snapshot * end;

   if (snapshots->size () >= 1)
   {
      end = 0;
      for (snapshotIter = snapshots->rbegin (); snapshotIter != snapshots->rend () - 1; ++snapshotIter)
      {

         start = (*snapshotIter);
         end = 0;

         if ((m_depositionTime > start->getTime ())
             || (fabs (m_depositionTime - start->getTime ()) < Genex5::Constants::ZERO))
         {
            start = *snapshotIter;
            end = *(snapshotIter + 1);
            SnapshotInterval *theInterval = new SnapshotInterval (start, end);

            m_theIntervals.push_back (theInterval);
         }
      }
   }
   else
   {
      //throw
   }
   delete snapshots;
}

void SourceRock::computeHcVolumes ( Genex5::SourceRockNode& node,
                                    double& gasVolume,
                                    double& oilVolume,
                                    double& gasExpansionRatio,
                                    double& gor,
                                    double& cgr,
                                    double& oilApi,
                                    double& condensateApi ) const {

   // Standard conditions.
   double StandardTemperature = 15.5555556 + Genex5::Constants::s_TCabs; //Kelvin
   double StandardPressure    = 101325.353; //Pa

   const Genex5::SourceRockNodeInput* lastInput = node.getTheLastInput ();

   Genex5::SimulatorState* state = node.getSimulatorState ();

   Genex5::PVTComponentMasses phaseMasses;
   Genex5::PVTPhaseValues     densities;
   Genex5::PVTPhaseValues     viscosities;

   Genex5::PVTComponents masses;
   double reservoirGasVolume = 0.0;

   double freeGasVolume;
   double condensateVolume;
   double solutionGasVolume;
   double liquidOilVolume; // This is the volume of oil in the liquid phase.

   masses = state->getVapourComponents ();

   if ( lastInput != 0 ) {
      // Calculate reservoir gas volume
      masses ( pvtFlash::COX ) = 0.0;
      Genex5::PVTCalc::getInstance ().compute ( lastInput->GetTemperatureKelvin (), lastInput->getPorePressure (), masses, phaseMasses, densities, viscosities );
      reservoirGasVolume = phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) / densities ( pvtFlash::VAPOUR_PHASE );
   }

   // Calculate surface volumes from vapour components.
   masses ( pvtFlash::COX ) = 0.0;
   Genex5::PVTCalc::getInstance ().compute ( StandardTemperature, StandardPressure, masses, phaseMasses, densities, viscosities );

   freeGasVolume = phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) / densities ( pvtFlash::VAPOUR_PHASE );
   gasVolume = freeGasVolume;

   condensateVolume = phaseMasses.sum ( pvtFlash::LIQUID_PHASE ) / densities ( pvtFlash::LIQUID_PHASE );
   oilVolume = condensateVolume;

   if ( densities ( pvtFlash::LIQUID_PHASE ) != 1000.0 ) {
      condensateApi = 141.5 / densities ( pvtFlash::LIQUID_PHASE ) * 1000.0 - 131.5;

      if ( condensateApi < 1.99 ) {
         condensateApi = 99999.0;
      }

   } else {
      condensateApi = 99999.0;
   }

   if ( freeGasVolume > 0.0 and densities ( pvtFlash::VAPOUR_PHASE ) != 1000.0 ) {
      cgr = condensateVolume / freeGasVolume;
   } else {
      cgr = 99999.0;
   }

   if ( lastInput != 0 and gasVolume > 0.0 ) {
      gasExpansionRatio = reservoirGasVolume / gasVolume;
   } else {
      gasExpansionRatio = 99999.0;
   }

   // Calculate surface volumes from liquid components.
   masses = state->getLiquidComponents ();
   masses ( pvtFlash::COX ) = 0.0;
   Genex5::PVTCalc::getInstance ().compute ( StandardTemperature, StandardPressure, masses, phaseMasses, densities, viscosities );

   solutionGasVolume = phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) / densities ( pvtFlash::VAPOUR_PHASE );
   liquidOilVolume = phaseMasses.sum ( pvtFlash::LIQUID_PHASE ) / densities ( pvtFlash::LIQUID_PHASE );

   gasVolume += solutionGasVolume;
   oilVolume += liquidOilVolume;

   if ( densities ( pvtFlash::LIQUID_PHASE ) != 1000.0 ) {
      oilApi = 141.5 / densities ( pvtFlash::LIQUID_PHASE ) * 1000.0 - 131.5;

      if ( oilApi < 1.99 ) {
         oilApi = 99999.0;
      }

   } else {
      oilApi = 99999.0;
   }

   if ( liquidOilVolume > 0.0 and densities ( pvtFlash::LIQUID_PHASE ) != 1000.0 ) {
      gor = solutionGasVolume / liquidOilVolume;
   } else {
      gor = 99999.0;
   }

}


}//namespace GenexSimulation



