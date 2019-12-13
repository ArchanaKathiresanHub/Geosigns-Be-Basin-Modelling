//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "GenexBaseSourceRock.h"

#include "SourceRock.h"
#include "Formation.h"
#include "GridMap.h"

#include "AdsorptionSimulator.h"
#include "SpeciesManager.h"

#include "SourceRockAdsorptionHistory.h"

#include <string>
#include <vector>
#include <map>

namespace AbstractDerivedProperties
{
class AbstractPropertyManager;
}

namespace database
{
class Record;
class Table;
}

namespace DataAccess
{
namespace Interface
{
class Formation;
class GridMap;
class Grid;
class LithoType;
class ProjectHandle;
class Snapshot;
}
}

namespace Genex6
{

class LinearGridInterpolator;
class LocalGridInterpolator;
class PropertyManager;
class Simulator;
class SnapshotInterval;
class SourceRockNode;

class GenexSourceRock : public DataAccess::Interface::SourceRock, public GenexBaseSourceRock
{
public:
  GenexSourceRock (DataAccess::Interface::ProjectHandle& projectHandle, database::Record * record);
  virtual ~GenexSourceRock ();

  void initializeSnapshotOutputMaps ( const std::vector<std::string> & requiredPropertyNames,
                                      const std::vector<std::string> & theRequestedPropertyNames );

  ///  set second SR type, mixing parameter, check Sulphur
  bool setFormationData ( const DataAccess::Interface::Formation * aFormation ) final;
  void setPropertyManager ( AbstractDerivedProperties::AbstractPropertyManager * aPropertyManager );

  /// Whether to perform adsorption
  bool doOutputAdsorptionProperties (void) const;

  /// Construct the valid source rock node set, the valid snapshot intervals
  bool preprocess ( const DataAccess::Interface::GridMap* validityMap,
                    const DataAccess::Interface::GridMap* vreMap = 0,
                    const bool printInitialisationDetails = true );

  /// \brief Clears the source-rock of any nodes, ...
  void clear ();

  /// \brief Clears the simulators and checmical models frmo the source rock.
  void clearSimulator();

  // After each computeTimeInstance call this must be called to clear the output data buffers.
  void clearOutputHistory ();

  /// Constructs the m_theSimulator, validates the chemical model
  bool initialize ( const bool printInitialisationDetails = true ) final;

  /// Initialises all source-rock nodes.
  void initialiseNodes ();

  /// \brief Get the species-manager from chemical-model.
  const SpeciesManager& getSpeciesManager () const;

  /// \brief Add history objects to the nodes.
  bool addHistoryToNodes () final;

  /// \brief Collect the history data from any nodes selected.
  void collectSourceRockNodeHistory ();

  /// \brief Sets variable that indicates whether output is desired also at minor snapshots
  void setMinor (const bool minor);

  /// \brief Gets variable that indicates whether output is desired also at minor snapshots
  bool getMinor (void) const;

protected:

  /// Construct the valid source rock node set, the valid snapshot intervals
  bool preprocess () final;

  /// Main processing functionality
  bool process() final;

  void clearSourceRockNodes();

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
  bool computeSnapshot ( const double previousTime,
                         const DataAccess::Interface::Snapshot *theSnapshot );

  DataAccess::Interface::GridMap *createSnapshotResultPropertyValueMap(const std::string &propertyName,
                                                                       const DataAccess::Interface::Snapshot *theSnapshot);

  bool isNodeActive ( const double VreAtPresentDay,
                      const double in_thickness,
                      const double in_TOC,
                      const double inorganicDensity,
                      const double temperatureAtPresentDay ) const;

  bool isNodeValid( const double temperatureAtPresentDay,
                    const double VreAtPresentDay,
                    const double thickness,
                    const double TOC,
                    const double inorganicDensity,
                    const double mapUndefinedValue) const;

  void addNode(Genex6::SourceRockNode *in_Node);

  double getLithoDensity(const DataAccess::Interface::LithoType *theLitho) const;

  const DataAccess::Interface::GridMap *getLithoType1PercentageMap()  const;
  const DataAccess::Interface::GridMap *getLithoType2PercentageMap()  const;
  const DataAccess::Interface::GridMap *getLithoType3PercentageMap()  const;
  const DataAccess::Interface::GridMap *getInputThicknessGridMap()    const;

  const DataAccess::Interface::LithoType *getLithoType1() const;
  const DataAccess::Interface::LithoType *getLithoType2() const;
  const DataAccess::Interface::LithoType *getLithoType3() const;

  const DataAccess::Interface::GridMap *getTopSurfacePropertyGridMap (const std::string &propertyName,
                                                                      const DataAccess::Interface::Snapshot *snapshot) const;

  const DataAccess::Interface::GridMap *getPropertyGridMap (const std::string &propertyName,
                                                            const DataAccess::Interface::Snapshot *snapshot) const;

  const DataAccess::Interface::GridMap *getPropertyGridMap (const std::string &propertyName,
                                                            const DataAccess::Interface::Snapshot *snapshot,
                                                            const DataAccess::Interface::Reservoir *reservoir,
                                                            const DataAccess::Interface::Formation *formation,
                                                            const DataAccess::Interface::Surface *surface) const;

  const DataAccess::Interface::GridMap * getSurfaceFormationPropertyGridMap (const std::string & propertyName,
                                                                             const DataAccess::Interface::Snapshot * snapshot) const;

  void createSnapShotOutputMaps(const DataAccess::Interface::Snapshot *theSnapshot);

  void updateSnapShotOutputMaps(Genex6::SourceRockNode *theNode);

  void saveSnapShotOutputMaps();

  bool validateGuiValue(const double GuiValue, const double LowerBound, const double UpperBound);

  const std::string & determineConfigurationFileName(const std::string & SourceRockType);

  void zeroTimeStepAccumulations ();

  Genex6::ChemicalModel * loadChemicalModel( const DataAccess::Interface::SourceRock * sr,
                                             const bool printInitialisationDetails = true );

  /// The valid nodes of the source rock
  std::vector<Genex6::SourceRockNode*> m_theNodes;

  double m_runtime;
  double m_time;

private: 
  Genex6::AdsorptionSimulator * getAdsorptionSimulator() const;

  /// Apply SR mixing flag
  bool m_applySRMixing;

  /// Output results also at minor snapshots
  bool m_minorOutput;

  /// The chemical model associated with the source rock1
  Genex6::ChemicalModel *m_theChemicalModel1;

  /// The chemical model associated with the source rock2
  Genex6::ChemicalModel *m_theChemicalModel2;

  /// \brief The simulator for adsorption processes.
  Genex6::AdsorptionSimulator* m_adsorptionSimulator;
  Genex6::AdsorptionSimulator* m_adsorptionSimulator2;

  AbstractDerivedProperties::AbstractPropertyManager * m_propertyManager;

  std::map<std::string, DataAccess::Interface::GridMap*> m_theSnapShotOutputMaps;
  /// \brief Mapping between source rock name and source rock type as written in genex configuration file
  static std::map<std::string, std::string> s_CfgFileNameBySRType;
  static void initializeCfgFileNameBySRType();


  DataAccess::Interface::GridMap* m_hcSaturationOutputMap;
  DataAccess::Interface::GridMap* m_irreducibleWaterSaturationOutputMap;

  DataAccess::Interface::GridMap* m_gasVolumeOutputMap;
  DataAccess::Interface::GridMap* m_oilVolumeOutputMap;
  DataAccess::Interface::GridMap* m_gasExpansionRatio;
  DataAccess::Interface::GridMap* m_gasGeneratedFromOtgc;
  DataAccess::Interface::GridMap* m_totalGasGenerated;

  DataAccess::Interface::GridMap* m_fracOfAdsorptionCap;

  DataAccess::Interface::GridMap* m_hcVapourSaturation;
  DataAccess::Interface::GridMap* m_hcLiquidSaturation;

  DataAccess::Interface::GridMap* m_adsorptionCapacity;

  DataAccess::Interface::GridMap* m_retainedOilApiOutputMap;
  DataAccess::Interface::GridMap* m_retainedCondensateApiOutputMap;

  DataAccess::Interface::GridMap* m_retainedGor;
  DataAccess::Interface::GridMap* m_retainedCgr;

  DataAccess::Interface::GridMap* m_overChargeFactor;
  DataAccess::Interface::GridMap* m_porosityLossDueToPyrobitumen;
  DataAccess::Interface::GridMap* m_h2sRisk;
  DataAccess::Interface::GridMap* m_tocOutputMap;

  DataAccess::Interface::GridMap* m_sourceRockEndMember1;
  DataAccess::Interface::GridMap* m_sourceRockEndMember2;

  std::map < CBMGenerics::ComponentManager::SpeciesNamesId, DataAccess::Interface::GridMap* > m_adsorpedOutputMaps;
  std::map < CBMGenerics::ComponentManager::SpeciesNamesId, DataAccess::Interface::GridMap* > m_expelledOutputMaps;
  std::map < CBMGenerics::ComponentManager::SpeciesNamesId, DataAccess::Interface::GridMap* > m_sourceRockExpelledOutputMaps;
  std::map < CBMGenerics::ComponentManager::SpeciesNamesId, DataAccess::Interface::GridMap* > m_freeOutputMaps;
  std::map < CBMGenerics::ComponentManager::SpeciesNamesId, DataAccess::Interface::GridMap* > m_retainedOutputMaps;
};

inline const SpeciesManager& GenexSourceRock::getSpeciesManager () const {
  return m_theChemicalModel->getSpeciesManager ();
}

inline void GenexSourceRock::setMinor( const bool minor) {
  m_minorOutput = minor;
}

inline bool GenexSourceRock::getMinor(void) const {
  return m_minorOutput;
}

} // namespace Genex6
