#include "stagesUA.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace ua
{

StageCompletionUA::StageCompletionUA() :
  Writable(),
  isComplete_{QVector<bool>(static_cast<int>(StageTypesUA::Count), false)}
{
}

void StageCompletionUA::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("isStageComplete", isComplete_);
}

void StageCompletionUA::readFromFile(const ScenarioReader& reader)
{
  isComplete_ = reader.readVector<bool>("isStageComplete");
}

void StageCompletionUA::clear()
{
  std::fill(isComplete_.begin(), isComplete_.end(), false);
}

void StageCompletionUA::setAllToCompleted()
{
  std::fill(isComplete_.begin(), isComplete_.end(), true);
}

bool StageCompletionUA::isComplete(const StageTypesUA& stageType) const
{
  return isComplete_[static_cast<int>(stageType)];
}

void StageCompletionUA::setStageIsComplete(const StageTypesUA& stageType, const bool isComplete)
{
  isComplete_[static_cast<int>(stageType)] = isComplete;
}

} // namespace ua

} // namespace casaWizard
