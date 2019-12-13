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

class GenexBaseSourceRock
{
public:
  GenexBaseSourceRock ();
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
  virtual bool setFormationData ( const DataAccess::Interface::Formation * aFormation ) = 0;

  /// Constructs the m_theSimulator, validates the chemical model
  virtual bool initialize ( const bool printInitialisationDetails = true ) = 0;

  /// \brief Add history objects to the nodes.
  virtual bool addHistoryToNodes () = 0;

  /// \brief Save data for all items on the history list.
  void saveSourceRockNodeAdsorptionHistory ();

  /// \brief Clear the history list.
  void clearSourceRockNodeAdsorptionHistory ();

protected:
  /// \brief Clears the base variables ...
  void clearBase ();

  /// \brief Clears the simulators and checmical models frmo the source rock.
  void clearSimulatorBase();

  /// Construct the valid source rock node set, the valid snapshot intervals
  virtual bool preprocess () = 0;

  /// Main processing functionality
  virtual bool process() = 0;

  void computeSnapshotIntervals(const DataAccess::Interface::SnapshotList & snapshots);

  void clearSnapshotIntervals();

  double getDepositionTime() const;

  double getMaximumTimeStepSize ( const double depositionTime ) const;

  int getRunType(const double in_SC) const;

  char * getGenexEnvironment(const double in_SC) const;

  /// The simulator associated with the source rock
  Genex6::Simulator *m_theSimulator;

  /// The chemical model associated with the source rock with bigger number of species.
  /// (to access SpeciesManager)
  Genex6::ChemicalModel *m_theChemicalModel;

  /// The snapshot intervals related to the source rock
  std::vector <SnapshotInterval*> m_theIntervals;

  ///The deposition time of the source rock
  double m_depositionTime;

  /// \brief List of all adsorption-history objects.
  Genex6::SourceRockAdsorptionHistoryList m_sourceRockNodeAdsorptionHistory;

  const DataAccess::Interface::Formation * m_formation;
  /// if Sulphur is included
  bool m_isSulphur;

private:
  void clearBaseAll();

  static const double conversionCoeffs [ 8 ];
};

inline bool GenexBaseSourceRock::isSulphur() const {
  return m_isSulphur;
}

} // namespace Genex6
