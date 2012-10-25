#ifndef _GENEXSIMULATION_SOURCEROCK_H_
#define _GENEXSIMULATION_SOURCEROCK_H_

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace database
{
   class Record;
   class Table;
}

#include "Interface/SourceRock.h"
#include "ComponentManager.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class Formation;
      class Snapshot;
      class GridMap;
      class Grid;
      class LithoType;
   }
}
using namespace DataAccess;

namespace Genex5
{
   class Simulator;
   class SourceRockNode;
}

#include "GenexSimulation.h"
#include "AdsorptionProjectHandle.h"

#include "PointAdsorptionHistory.h"
#include "SourceRockAdsorptionHistory.h"

namespace GenexSimulation
{

class LocalGridInterpolator;
class SnapshotInterval;

class SourceRock : public Interface::SourceRock
{
public:
   SourceRock (Interface::ProjectHandle * projectHandle, database::Record * record);
   virtual ~SourceRock (void);

   void setAdsorptionProjectHandle ( AdsorptionProjectHandle* adsorptionProjectHandle );

   /// Computes generation and expulsion of the source rock
   bool compute();
  
protected:
   /// Construct the valid source rock node set, the valid snapshot intervals 
   bool preprocess();
   /// Constructs the m_theSimulator, validates the chemical model
   bool initialize();
   /// Main processing functionality
   bool process();

   void computeSnapshotIntervals();
   void clearSnapshotIntervals();
   void clearSimulator();
   void clearSourceRockNodes();
   double getDepositionTime() const;

   /// Compute the new state at a time instance for all the valid source rock nodes
//    void computeTimeInstance(  const double &t, 
//                               const LocalGridInterpolator *VES,  
//                               const LocalGridInterpolator *Temp, 
//                               const LocalGridInterpolator *thicknessScaling);

   /// Compute the new state at a time instance for all the valid source rock nodes
   void computeTimeInstance ( const double &startTime,
                              const double &endTime,
                              const LocalGridInterpolator* ves,
                              const LocalGridInterpolator* temperature,
                              const LocalGridInterpolator* thicknessScaling,
                              const LocalGridInterpolator* lithostaticPressure,
                              const LocalGridInterpolator* hydrostaticPressure,
                              const LocalGridInterpolator* porePressure,
                              const LocalGridInterpolator* porosity,
                              const LocalGridInterpolator* permeability,
                              const LocalGridInterpolator* vre );


   /// Compute the new state and the results at a snapshot for all the valid source rock nodes 
   bool computeSnapShot ( const double previousTime,
                          const Interface::Snapshot *theSnapshot);
   
   Interface::GridMap *createSnapshotResultPropertyValueMap(const std::string &propertyName, const Interface::Snapshot *theSnapshot);

   bool isNodeActive(const double &VreAtPresentDay, const double &in_thickness, const double &in_TOC, const double &inorganicDensity) const;

   bool isNodeValid( const double & temperatureAtPresentDay, 
                     const double &VreAtPresentDay, 
                     const double &thickness, 
                     const double &TOC, 
                     const double &inorganicDensity, 
                     const double &mapUndefinedValue) const;

   void addNode(Genex5::SourceRockNode *in_Node);
   
   double getLithoDensity(const Interface::LithoType *theLitho) const;

   const Interface::GridMap *getLithoType1PercentageMap()  const;
   const Interface::GridMap *getLithoType2PercentageMap()  const;
   const Interface::GridMap *getLithoType3PercentageMap()  const;
   const Interface::GridMap *getInputThicknessGridMap()    const;

   const Interface::LithoType *getLithoType1() const;
   const Interface::LithoType *getLithoType2() const; 
   const Interface::LithoType *getLithoType3() const; 

   const Interface::GridMap *getTopSurfacePropertyGridMap (const string &propertyName, const Interface::Snapshot *snapshot) const;

   const Interface::GridMap *getPropertyGridMap (const string &propertyName, const Interface::Snapshot *snapshot) const;

   const Interface::GridMap *getPropertyGridMap (const string &propertyName,
                                                      const Interface::Snapshot *snapshot,
                                                      const Interface::Reservoir *reservoir, 
                                                      const Interface::Formation *formation, 
                                                      const Interface::Surface *surface) const;

	const Interface::GridMap *getFormationPropertyGridMap (const string & propertyName,const Interface::Snapshot * snapshot) const;

   const Interface::GridMap * getSurfaceFormationPropertyGridMap (const string & propertyName,const Interface::Snapshot * snapshot) const;


   void initializeSnapShotOutputMaps();

   void createSnapShotOutputMaps(const Interface::Snapshot *theSnapshot);

   void updateSnapShotOutputMaps(Genex5::SourceRockNode *theNode);

   void saveSnapShotOutputMaps(const Interface::Snapshot *theSnapshot);

   bool validateGuiValue(const double & GuiValue, const double & LowerBound, const double & UpperBound);

   const string & determineConfigurationFileName(const string & SourceRockType);

   void findNodeHistoryObjects ( Genex5::SourceRockNode * node );

   void saveSourceRockNodeAdsorptionHistory ();


private: 


   /// Computes the volume of oil and gas per volume of rock.
   void computeHcVolumes ( Genex5::SourceRockNode& node,
                           double& gasVolume,
                           double& oilVolume,
                           double& gasExpansionRatio,
                           double& gor,
                           double& cgr,
                           double& oilApi,
                           double& condensateApi ) const;

   ///The deposition time of the source rock
   double m_depositionTime;

   /// The snapshot intervals related to the source rock
   std::vector <SnapshotInterval*> m_theIntervals;

   /// The simulator associated with the source rock
   Genex5::Simulator *m_theSimulator;

   AdsorptionProjectHandle* m_adsorptionProjectHandle;

   /// The valid nodes of the source rock
   std::vector<Genex5::SourceRockNode*> m_theNodes;

   std::map<std::string, Interface::GridMap*> m_theSnapShotOutputMaps;  

   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_adsorpedOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_expelledOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_sourceRockExpelledOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_freeOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_retainedOutputMaps;

   Interface::GridMap* m_hcSaturationOutputMap;
   Interface::GridMap* m_irreducibleWaterSaturationOutputMap;

   Interface::GridMap* m_gasVolumeOutputMap;
   Interface::GridMap* m_oilVolumeOutputMap;
   Interface::GridMap* m_gasExpansionRatio;
   Interface::GridMap* m_gasGeneratedFromOtgc;
   Interface::GridMap* m_totalGasGenerated;

   Interface::GridMap* m_fracOfAdsorptionCap;

   Interface::GridMap* m_hcVapourSaturation;
   Interface::GridMap* m_hcLiquidSaturation;

   Interface::GridMap* m_adsorptionCapacity;

   Interface::GridMap* m_retainedOilApiOutputMap;
   Interface::GridMap* m_retainedCondensateApiOutputMap;

   Interface::GridMap* m_retainedGor;
   Interface::GridMap* m_retainedCgr;

   static std::map<std::string, std::string> s_CfgFileNameBySRType;
   static void initializeCfgFileNameBySRType();

   SourceRockAdsorptionHistoryList m_sourceRockNodeAdsorptionHistory;

};
}

#endif 

