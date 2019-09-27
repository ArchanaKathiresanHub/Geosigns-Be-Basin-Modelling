// Manager for the lithofractions in the SAC
#pragma once

#include "lithofraction.h"
#include "model/writable.h"
#include "optimizedLithofraction.h"
#include "model/writable.h"

#include <QVector>

class QString;

namespace casaWizard
{

namespace sac
{

class LithofractionManager : public Writable
{
public:
  LithofractionManager();

  const QVector<Lithofraction>& lithofractions() const;
  void addLithofraction(const QString& name);
  void removeLithofraction(int index);

  void setLithofractionLayerName(int index, const QString& name);
  void setLithofractionFirstComponent(int index, int percent);
  void setLithofractionFirstMinPercentage(int index, double value);
  void setLithofractionFirstMaxPercentage(int index, double value);
  void setLithofractionSecondComponent(int index, int percent);
  void setLithofractionSecondMinFraction(int index, double value);
  void setLithofractionSecondMaxFraction(int index, double value);

  const QVector<OptimizedLithofraction>& optimizedLithofractions() const;
  QVector<OptimizedLithofraction> optimizedInWell(const int wellIndex) const;
  void addOptimizedLithofraction(const OptimizedLithofraction& optimal);
  void clearOptimizedLithofractions();

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  LithofractionManager(const LithofractionManager&) = delete;
  LithofractionManager& operator=(LithofractionManager) = delete;

  QVector<Lithofraction> lithofractions_;
  QVector<OptimizedLithofraction> optimizedLithofractions_;

  void checkLithofractionPercentWarning(int index) const;
};

}  // namespace sac

}  // namespace casaWizard
