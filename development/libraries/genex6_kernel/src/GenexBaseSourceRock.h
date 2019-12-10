//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "SourceRock.h"

#include "SourceRockAdsorptionHistory.h"

#include <string>
#include <vector>
#include <map>

namespace database
{
class Record;
class Table;
}

namespace DataAccess
{
namespace Interface
{
class ProjectHandle;
class Formation;
class Snapshot;
}
}

namespace Genex6
{
class Simulator;
class SourceRockNode;
class ChemicalModel;
}

namespace Genex6
{

class SnapshotInterval;

class GenexBaseSourceRock : public DataAccess::Interface::SourceRock
{
public:
  GenexBaseSourceRock (DataAccess::Interface::ProjectHandle& projectHandle, database::Record * record);
  virtual ~GenexBaseSourceRock ();

  /// \brief Convert H/C value to HI
  static double convertHCtoHI( double aHI );

  /// \brief Convert HI value to H/C
  static double convertHItoHC( double aHI );

  /// \brief Get min/max range value for HI
  static void getHIBounds( double &HILower, double &HIUpper );

  /// Computes generation and expulsion of the source rock
  bool compute();

  ///if SourceRock contains Sulphur
  bool isSulphur() const;

  ///  set second SR type, mixing parameter, check Sulphur
  virtual bool setFormationData ( const DataAccess::Interface::Formation * aFormation );

  /// Whether to perform adsorption
  bool doOutputAdsorptionProperties (void) const;

  /// \brief Clears the source-rock of any nodes, ...
  void clear ();

  /// \brief Clears the simulators and checmical models frmo the source rock.
  void clearSimulator();

  /// Constructs the m_theSimulator, validates the chemical model
  virtual bool initialize ( const bool printInitialisationDetails = true ) = 0;

  /// \brief Add history objects to the nodes.
  virtual bool addHistoryToNodes () = 0;

  /// \brief Save data for all items on the history list.
  void saveSourceRockNodeAdsorptionHistory ();

  /// \brief Clear the history list.
  void clearSourceRockNodeAdsorptionHistory ();

  /// \brief Sets variable that indicates whether output is desired also at minor snapshots
  void setMinor (const bool minor);

  /// \brief Gets variable that indicates whether output is desired also at minor snapshots
  bool getMinor (void) const;

protected:

  /// Construct the valid source rock node set, the valid snapshot intervals
  virtual bool preprocess () = 0;

  /// Main processing functionality
  virtual bool process() = 0;

  void computeSnapshotIntervals( const int snapshotType );

  void clearSnapshotIntervals();

  double getDepositionTime() const;

  double getMaximumTimeStepSize ( const double depositionTime ) const;

  const Simulator& getSimulator () const;

  void setLayerName( const std::string & layerName );

  /// The simulator associated with the source rock
  Genex6::Simulator *m_theSimulator;

  /// The chemical model associated with the source rock with bigger number of species.
  /// (to access SpeciesManager)
  Genex6::ChemicalModel *m_theChemicalModel;

  /// The chemical model associated with the source rock1
  Genex6::ChemicalModel *m_theChemicalModel1;

  /// The chemical model associated with the source rock2
  Genex6::ChemicalModel *m_theChemicalModel2;

  /// The snapshot intervals related to the source rock
  std::vector <SnapshotInterval*> m_theIntervals;

  ///The deposition time of the source rock
  double m_depositionTime;

  /// \brief List of all adsorption-history objects.
  Genex6::SourceRockAdsorptionHistoryList m_sourceRockNodeAdsorptionHistory;

  std::string m_layerName;
  const DataAccess::Interface::Formation * m_formation;
  /// if Sulphur is included
  bool m_isSulphur;

private: 
  /// Apply SR mixing flag
  bool m_applySRMixing;

  /// Output results also at minor snapshots
  bool m_minorOutput;

  static const double conversionCoeffs [ 8 ];
};

inline void GenexBaseSourceRock::setLayerName( const std::string & aLayerName ) {
  m_layerName = aLayerName;
}

inline bool GenexBaseSourceRock::isSulphur() const {
  return m_isSulphur;
}

inline void GenexBaseSourceRock::setMinor( const bool minor) {
  m_minorOutput = minor;
}

inline bool GenexBaseSourceRock::getMinor(void) const {
  return m_minorOutput;
}

} // namespace Genex6
