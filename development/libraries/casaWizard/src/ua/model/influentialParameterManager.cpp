#include "influentialParameterManager.h"

#include "influentialParameter.h"
#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace ua
{

InfluentialParameterManager::InfluentialParameterManager(const ProjectReader& projectReader) :
  options_{InfluentialParameter::list()},
  influentialParameters_(),
  isUsedInCorrelation_{},
  projectReader_{projectReader}
{
}

InfluentialParameterManager::~InfluentialParameterManager()
{
  for (InfluentialParameter* influentialParameter : influentialParameters_)
  {
    delete influentialParameter;
  }
}

QVector<InfluentialParameter*> InfluentialParameterManager::influentialParameters() const
{
  return influentialParameters_;
}

int InfluentialParameterManager::totalNumberOfInfluentialParameters() const
{
  int nInfluentialParameters = 0;

  for (const InfluentialParameter* influentialParameter : influentialParameters_)
  {
    nInfluentialParameters += influentialParameter->totalNumberOfParameters();
  }

  return nInfluentialParameters;
}

QStringList InfluentialParameterManager::options() const
{
  return options_;
}

void InfluentialParameterManager::set(const int row, const int selected)
{
  delete influentialParameters_[row];
  influentialParameters_[row] = InfluentialParameter::createFromIndex(selected);
  influentialParameters_[row]->fillArguments(projectReader_);
}

void InfluentialParameterManager::setArguments(int index, const QVector<double>& valueArguments, const QStringList& listArguments)
{
  influentialParameters_[index]->arguments().setDoubleArguments(valueArguments);
  influentialParameters_[index]->arguments().setListValues(listArguments);
}

void InfluentialParameterManager::add(const int index)
{
  InfluentialParameter* newInfluentialParameter = InfluentialParameter::createFromIndex(index);
  newInfluentialParameter->fillArguments(projectReader_);
  influentialParameters_.append(newInfluentialParameter);

  isUsedInCorrelation_.push_back(true);
}

void InfluentialParameterManager::remove(int index)
{
  influentialParameters_.remove(index);
  isUsedInCorrelation_.remove(index);
}


QVector<bool> InfluentialParameterManager::isUsedInCorrelation() const
{
  return isUsedInCorrelation_;
}

void InfluentialParameterManager::setIsUsedInCorrelation(const QVector<bool>& isInfluentialParameterUsedInCorrelation)
{
  isUsedInCorrelation_ = isInfluentialParameterUsedInCorrelation;
}

QStringList InfluentialParameterManager::nameList() const
{
  QStringList names;
  for (const InfluentialParameter* influentialParameter : influentialParameters_)
  {
    names << influentialParameter->labelName();
  }
  return names;
}

void InfluentialParameterManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("InfluentialParameterManagerVersion", 0);
  writer.writeValue("influentialParameter", influentialParameters_);
}

void InfluentialParameterManager::readFromFile(const ScenarioReader& reader)
{
  clear();
  influentialParameters_ = reader.readAndCreateVector<InfluentialParameter>("influentialParameter");
  isUsedInCorrelation_ = QVector<bool>(influentialParameters_.size(), true);
}

void InfluentialParameterManager::clear()
{
  for (InfluentialParameter* influentialParameter : influentialParameters_)
  {
    delete influentialParameter;
  }
  influentialParameters_.clear();
  isUsedInCorrelation_.clear();
}

} // namespace ua

} // namespace casaWizard
