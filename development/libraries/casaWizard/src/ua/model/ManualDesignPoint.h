//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVector>
#include "model/writable.h"

namespace casaWizard
{

namespace ua
{

class ManualDesignPoint
{
public:
   ManualDesignPoint();
   ManualDesignPoint(const QVector<double>& influentialParameterValues);

   enum class SimulationState
   {
      Completed,
      Failed,
      NotRun
   };

   void addInfluentialParameter(double parameter);
   void hide();
   void removeParameter(int index);
   void setParameterValue(int index, double value);
   void setSimulationState(bool simState);
   void setSimulationState(SimulationState state);


   const QVector<double>& influentialParameterValues() const;
   bool hidden() const;
   SimulationState simulationState() const;

   int version() const;
   QStringList write() const;
   static ManualDesignPoint read(const int version, const QStringList& p);

private:
   static SimulationState stateFromString(const QString& stateString);
   static QString stringFromState(SimulationState state);

   QVector<double> m_influentialParameterValues;
   bool m_hidden;
   SimulationState m_simulationState;
};

} // namespace ua

} // namespace casaWizard
