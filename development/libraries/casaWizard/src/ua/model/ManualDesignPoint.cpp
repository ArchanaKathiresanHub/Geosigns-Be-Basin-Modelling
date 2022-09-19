//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "ManualDesignPoint.h"
#include "model/logger.h"

#include "assert.h"

namespace casaWizard
{

namespace ua
{

ManualDesignPoint::ManualDesignPoint():
   m_hidden(false),
   m_simulationState(SimulationState::NotRun)
{}

ManualDesignPoint::ManualDesignPoint(const QVector<double>& influentialParameterValues):
   m_influentialParameterValues(influentialParameterValues),
   m_hidden(false),
   m_simulationState(SimulationState::NotRun)
{}

ManualDesignPoint::SimulationState ManualDesignPoint::stateFromString(const QString& stateString)
{
   if (stateString == "Completed") return SimulationState::Completed;
   if (stateString == "Failed") return SimulationState::Failed;
   if (stateString == "NotRun") return SimulationState::NotRun;
   throw std::runtime_error("Unknown manual design point state string.");
}

QString ManualDesignPoint::stringFromState(SimulationState state)
{
   switch (state)
   {
   case SimulationState::Completed: return "Completed";
   case SimulationState::Failed: return "Failed";
   case SimulationState::NotRun: return "NotRun";
   }
   assert(false); //Update the function with added states if this assert is reached.
   return "";
}

void ManualDesignPoint::addInfluentialParameter(double parameter)
{
   m_influentialParameterValues.push_back(parameter);
}

void ManualDesignPoint::hide()
{
   m_hidden = true;
}

void ManualDesignPoint::removeParameter(int index)
{
   if (index < m_influentialParameterValues.size() && index >= 0)
   {
      m_influentialParameterValues.remove(index);
   }
}

void ManualDesignPoint::setParameterValue(int index, double value)
{
   if (m_simulationState == SimulationState::NotRun && index < m_influentialParameterValues.size() && index >= 0)
   {
      m_influentialParameterValues[index] = value;
   }
}

void ManualDesignPoint::setSimulationState(SimulationState state)
{
   m_simulationState =state;
}

void ManualDesignPoint::setSimulationState(bool completionState)
{
   if (m_simulationState == SimulationState::Completed && completionState == false)
   {
      Logger::log() << "Warning changing manual design point completion state from true to false. Is this correct?" << Logger::endl();
   }

   if (completionState == true)
   {
      m_simulationState = SimulationState::Completed;
   }
   else
   {
      m_simulationState = SimulationState::Failed;
   }
}

const QVector<double>& ManualDesignPoint::influentialParameterValues() const
{
   return m_influentialParameterValues;
}

bool ManualDesignPoint::hidden() const
{
   return m_hidden;
}

ManualDesignPoint::SimulationState ManualDesignPoint::simulationState() const
{
   return m_simulationState;
}

QStringList ManualDesignPoint::write() const
{
   QStringList out;
   out << scenarioIO::vectorToWrite(m_influentialParameterValues)
       << (m_hidden ? "1":"0")
       << stringFromState(m_simulationState);
   return out;
}

ManualDesignPoint ManualDesignPoint::read(const int version, const QStringList& p)
{
   if (p.size() != 3)
   {
      return ManualDesignPoint();
   }
   ManualDesignPoint manualDesignPoint(scenarioIO::vectorFromRead(p[0]));
   if (p[1]== "1")
   {
      manualDesignPoint.hide();
   }
   manualDesignPoint.setSimulationState(stateFromString(p[2]));
   return manualDesignPoint;
}

int ManualDesignPoint::version() const
{
   return 0;
}

} // namespace ua

} // namespace casaWizard
