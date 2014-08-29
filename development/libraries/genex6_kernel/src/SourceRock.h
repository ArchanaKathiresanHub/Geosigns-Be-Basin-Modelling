#ifndef _GENEXSIMULATION_SOURCEROCK_H_
#define _GENEXSIMULATION_SOURCEROCK_H_

namespace database
{
   class Record;
   class Table;
}

#include "Interface/SourceRock.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"

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

#include "AdsorptionSimulator.h"
#include "SpeciesManager.h"

namespace Genex6
{
   class Simulator;
   class SourceRockNode;
}

#include "SourceRockAdsorptionHistory.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace Genex6
{

class LocalGridInterpolator;
class SnapshotInterval;

class SourceRock : public Interface::SourceRock
{
public:
   SourceRock (Interface::ProjectHandle * projectHandle, database::Record * record);
   virtual ~SourceRock (void);

   /// Computes generation and expulsion of the source rock
   bool compute();

   void initializeSnapShotOutputMaps ( const vector<string> & requiredPropertyNames,
                                       const vector<string> & theRequestedPropertyNames );

   ///if SourceRock contains Sulphur
   bool isSulphur() const;

   ///  set second SR type, mixing parameter, check Sulphur
   bool setFormationData ( const Interface::Formation * aFormation );

   /// Whether to perform adsorption 
   bool doOutputAdsorptionProperties (void) const;

   /// Construct the valid source rock node set, the valid snapshot intervals 
   bool preprocess ( const Interface::GridMap* validityMap,
                     const Interface::GridMap* vreMap = 0,
                     const bool printInitialisationDetails = true );

   /// \brief Clears the source-rock of any nodes, ...
   void clear ();

   /// \brief Clears the simulators and checmical models frmo the source rock.
   void clearSimulator();

   // After each computeTimeInstance call this must be called to clear the output data buffers.
   void clearOutputHistory ();

   /// Constructs the m_theSimulator, validates the chemical model
   bool initialize ( const bool printInitialisationDetails = true );

   /// Initialises all source-rock nodes.
   void initialiseNodes ();

   /// \brief Get the species-manager from chemical-model.
   const SpeciesManager& getSpeciesManager () const;

   /// \brief Add history objects to the nodes.
   bool addHistoryToNodes ();

   /// \brief Save data for all items on the history list.
   void saveSourceRockNodeAdsorptionHistory ();

   /// \brief Clear the history list.
   void clearSourceRockNodeAdsorptionHistory ();

   /// \brief Collect the history data from any nodes selected.
   void collectSourceRockNodeHistory ();

protected:

   /// Construct the valid source rock node set, the valid snapshot intervals 
   bool preprocess ();

   /// Main processing functionality
   bool process();

   void computeSnapshotIntervals();

   void clearSnapshotIntervals();
   void clearSourceRockNodes();

   double getDepositionTime() const;

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
   // void computeTimeInstance(  const double t, 
   //                            const LocalGridInterpolator *VES,  
   //                            const LocalGridInterpolator *Temp, 
   //                            const LocalGridInterpolator *thicknessScaling);

   /// Compute the new state and the results at a snapshot for all the valid source rock nodes 
   bool computeSnapShot ( const double previousTime,
                          const Interface::Snapshot *theSnapshot );
   
   Interface::GridMap *createSnapshotResultPropertyValueMap(const std::string &propertyName, 
                                                                 const Interface::Snapshot *theSnapshot);

   bool isNodeActive ( const double VreAtPresentDay,
                       const double in_thickness,
                       const double in_TOC,
                       const double inorganicDensity ) const;

   bool isNodeValid( const double temperatureAtPresentDay, 
                     const double VreAtPresentDay, 
                     const double thickness, 
                     const double TOC, 
                     const double inorganicDensity, 
                     const double mapUndefinedValue) const;

   void addNode(Genex6::SourceRockNode *in_Node);
   
   double getLithoDensity(const Interface::LithoType *theLitho) const;

   const Interface::GridMap *getLithoType1PercentageMap()  const;
   const Interface::GridMap *getLithoType2PercentageMap()  const;
   const Interface::GridMap *getLithoType3PercentageMap()  const;
   const Interface::GridMap *getInputThicknessGridMap()    const;

   const Interface::LithoType *getLithoType1() const;
   const Interface::LithoType *getLithoType2() const; 
   const Interface::LithoType *getLithoType3() const; 

   const Interface::GridMap *getTopSurfacePropertyGridMap (const string &propertyName, 
                                                                const Interface::Snapshot *snapshot) const;

   const Interface::GridMap *getPropertyGridMap (const string &propertyName, 
                                                      const Interface::Snapshot *snapshot) const;

   const Interface::GridMap *getPropertyGridMap (const string &propertyName,
                                                      const Interface::Snapshot *snapshot,
                                                      const Interface::Reservoir *reservoir, 
                                                      const Interface::Formation *formation, 
                                                      const Interface::Surface *surface) const;

   const Interface::GridMap *getFormationPropertyGridMap (const string & propertyName,
                                                               const Interface::Snapshot * snapshot) const;

   const Interface::GridMap * getSurfaceFormationPropertyGridMap (const string & propertyName,
                                                                       const Interface::Snapshot * snapshot) const;

   void createSnapShotOutputMaps(const Interface::Snapshot *theSnapshot);

   void updateSnapShotOutputMaps(Genex6::SourceRockNode *theNode);

   void saveSnapShotOutputMaps(const Interface::Snapshot *theSnapshot);

   bool validateGuiValue(const double GuiValue, const double LowerBound, const double UpperBound);

   const string & determineConfigurationFileName(const string & SourceRockType);

   double getMaximumTimeStepSize ( const double depositionTime ) const;

   void zeroTimeStepAccumulations ();

   const Simulator& getSimulator () const;

   // \brief Convert HC value to HI 
   double convertHCtoHI ( double aHI );

   // \brief Convert HI value to HC 
   double convertHItoHC ( double aHI );
    
   void getHIBounds( double &HILower, double &HIUpper );
   /// The valid nodes of the source rock
   std::vector<Genex6::SourceRockNode*> m_theNodes;

private: 
   Genex6::ChemicalModel * loadChemicalModel( const Interface::SourceRock * sr,
                                              const bool printInitialisationDetails = true );

   void setLayerName( const string & layerName );

   Genex6::AdsorptionSimulator * getAdsorptionSimulator() const;
   
   const Interface::Formation * m_formation;
   std::string m_layerName;

   ///The deposition time of the source rock
   double m_depositionTime;

   double m_runtime;
   double m_time;

   /// if Sulphur is included
   bool m_isSulphur;

   /// Apply SR mixing flag
   bool m_applySRMixing;

   /// The snapshot intervals related to the source rock
   std::vector <SnapshotInterval*> m_theIntervals;

   /// The simulator associated with the source rock
   Genex6::Simulator *m_theSimulator;

   /// The chemical model associated with the source rock with bigger number of species.
   /// (to access SpeciesManager)
   Genex6::ChemicalModel *m_theChemicalModel;

   /// The chemical model associated with the source rock1
   Genex6::ChemicalModel *m_theChemicalModel1;

   /// The chemical model associated with the source rock2
   Genex6::ChemicalModel *m_theChemicalModel2;

   /// \brief The simulator for adsorption processes.
   Genex6::AdsorptionSimulator* m_adsorptionSimulator;
   Genex6::AdsorptionSimulator* m_adsorptionSimulator2;

   /// \brief List of all adsorption-history objects.
   Genex6::SourceRockAdsorptionHistoryList m_sourceRockNodeAdsorptionHistory;


   std::map<std::string, Interface::GridMap*> m_theSnapShotOutputMaps;
   static std::map<std::string, std::string> s_CfgFileNameBySRType;
   static void initializeCfgFileNameBySRType();


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

   Interface::GridMap* m_overChargeFactor;
   Interface::GridMap* m_porosityLossDueToPyrobitumen;
   Interface::GridMap* m_h2sRisk;
   Interface::GridMap* m_tocOutputMap;

   Interface::GridMap* m_sourceRockEndMember1;
   Interface::GridMap* m_sourceRockEndMember2;

   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_adsorpedOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_expelledOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_sourceRockExpelledOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_freeOutputMaps;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* > m_retainedOutputMaps;

   static const double conversionCoeffs [ 8 ]; 
};

inline void SourceRock::setLayerName( const string & aLayerName ) {
   m_layerName = aLayerName;
}

inline bool SourceRock::isSulphur() const {
   return m_isSulphur;
}

inline const SpeciesManager& SourceRock::getSpeciesManager () const {
   return m_theChemicalModel->getSpeciesManager ();
}

}



#endif 

