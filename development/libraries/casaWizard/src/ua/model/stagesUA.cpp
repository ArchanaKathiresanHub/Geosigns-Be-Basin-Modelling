#include "stagesUA.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace ua
{

StageStateUA::StageStateUA(const QString stateName, bool state) :
  Writable(),
  m_stageStates{QVector<bool>(static_cast<int>(StageTypesUA::Count), state)},
  m_stateName(stateName)
{
}

void StageStateUA::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue(m_stateName, m_stageStates);
}

void StageStateUA::readFromFile(const ScenarioReader& reader)
{
  m_stageStates = reader.readVector<bool>(m_stateName);
  int nStages = static_cast<int>(StageTypesUA::Count);
  if (m_stageStates.size() != nStages)
  {
     m_stageStates = QVector<bool>(nStages, false);
  }
}

void StageStateUA::clear()
{
  std::fill(m_stageStates.begin(), m_stageStates.end(), false);
}

void StageStateUA::setAllToTrue()
{
  std::fill(m_stageStates.begin(), m_stageStates.end(), true);
}

bool StageStateUA::isTrue(const StageTypesUA& stageType) const
{
  return m_stageStates[static_cast<int>(stageType)];
}

void StageStateUA::setStageState(const StageTypesUA& stageType, const bool isComplete)
{
  m_stageStates[static_cast<int>(stageType)] = isComplete;
}

} // namespace ua

} // namespace casaWizard
