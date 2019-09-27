// Manager for the influential parameter data
#pragma once

#include "model/writable.h"

#include <QStringList>
#include <QVector>

namespace casaWizard
{

class ProjectReader;

namespace ua
{

class InfluentialParameter;

class InfluentialParameterManager : public Writable
{
public:
  explicit InfluentialParameterManager(const ProjectReader& projectReader);
  ~InfluentialParameterManager();

  QVector<InfluentialParameter*> influentialParameters() const;
  void set(const int row, const int selected);
  void setArguments(int index, const QVector<double>& valueArguments, const QStringList& listArguments);
  void add(const int index = 0);
  void remove(int index);

  QStringList options() const;
  int totalNumberOfInfluentialParameters() const;
  QVector<bool> isUsedInCorrelation() const;
  void setIsUsedInCorrelation(const QVector<bool>& isUsedInCorrelation);

  QStringList nameList() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  InfluentialParameterManager(const InfluentialParameterManager&) = delete;
  InfluentialParameterManager& operator=(InfluentialParameterManager) = delete;

  QStringList options_;
  QVector<InfluentialParameter*> influentialParameters_;
  QVector<bool> isUsedInCorrelation_;
  const ProjectReader& projectReader_;
};

} // namespace ua

} // namespace casaWizard
