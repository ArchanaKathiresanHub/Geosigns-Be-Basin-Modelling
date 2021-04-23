#include "well.h"

#include "model/scenarioIO.h"
#include "objectiveFunctionManager.h"
#include "scenarioReader.h"
#include "scenarioWriter.h"
#include <QStringList>

namespace casaWizard
{

Well::Well(const int id, const QString& name, const double x, const double y, const bool isActive, const bool isExcluded, const QVector<CalibrationTarget> calibrationTargets) :
  id_{id},
  name_{name},
  x_{x},
  y_{y},
  isActive_{isActive},
  isExcluded_{isExcluded},
  isOutOfBasin_{false},
  calibrationTargets_{calibrationTargets}
{
}

int Well::version() const
{
  return 1;
}

void Well::writeToFile(ScenarioWriter& writer) const
{
  QString wellName{"well" + QString::number(id_)};
  writer.writeValue(wellName + "version", version());
  writer.writeValue(wellName + "id", id_);
  writer.writeValue(wellName + "name", name_);
  writer.writeValue(wellName + "x", x_);
  writer.writeValue(wellName + "y", y_);
  writer.writeValue(wellName + "isActive", isActive_);
  writer.writeValue(wellName + "isExcluded", isExcluded_);
  writer.writeValue(wellName + "isOutOfBasin", isOutOfBasin_);
  writer.writeValue(wellName + "targets", calibrationTargets_);
}

void Well::readFromFile(const ScenarioReader& reader)
{
  QString wellName{"well" + QString::number(id_)};
  id_ = reader.readInt(wellName + "id");
  name_ = reader.readString(wellName + "name");
  x_ = reader.readDouble(wellName + "x");
  y_ = reader.readDouble(wellName + "y");
  isActive_ = reader.readBool(wellName + "isActive");
  isExcluded_ = reader.readBool(wellName+ "isExcluded");
  calibrationTargets_ = reader.readVector<CalibrationTarget>(wellName + "targets");

  const int version = reader.readInt(wellName + "version");
  if (version > 0)
  {
    isOutOfBasin_ = reader.readBool(wellName + "isOutOfBasin");
  }
  else
  {
    isOutOfBasin_ = false;
  }
}

void Well::readFromFile(const ScenarioReader& reader, const int wellIndex)
{
  id_ = wellIndex;
  readFromFile(reader);
}

void Well::clear()
{
  calibrationTargets_.clear();
}

int Well::id() const
{
  return id_;
}

QString Well::name() const
{
  return name_;
}

double Well::x() const
{
  return x_;
}

double Well::y() const
{
  return y_;
}

bool Well::isActive() const
{
  return isActive_;
}

void Well::setIsActive(const bool isActive)
{
  isActive_ = isActive;
}

bool Well::isExcluded() const
{
  return isExcluded_;
}

void Well::setIsExcluded(const bool isExcluded)
{
  isExcluded_ = isExcluded;
}

bool Well::isOutOfBasin() const
{
  return isOutOfBasin_;
}

void Well::setIsOutOfBasin(const bool isOutOfAOI)
{
  isOutOfBasin_ = isOutOfAOI;
}

QVector<const CalibrationTarget*> Well::calibrationTargets() const
{
  QVector<const CalibrationTarget*> targets;
  for (const CalibrationTarget& target: calibrationTargets_)
  {
    targets.push_back(&target);
  }
  return targets;
}

void Well::setCalibrationTargets(const QVector<CalibrationTarget>& calibrationTargets)
{
  calibrationTargets_ = calibrationTargets;
}

void Well::addCalibrationTarget(const QString& name, const QString& property,
                                const double z, const double value)
{
  const double standardDeviation = 1;
  const double uaWeight = 1;

  CalibrationTarget newTarget{name, property, z, value, standardDeviation, uaWeight};
  calibrationTargets_.append(newTarget);
}

void Well::applyObjectiveFunction(const ObjectiveFunctionManager& objectiveFunction)
{
  for (CalibrationTarget& target : calibrationTargets_)
  {
    const int index = objectiveFunction.indexOf(target.property());
    if (index>-1)
    {
      double standardDeviation = objectiveFunction.absoluteError(index) +
                                 objectiveFunction.relativeError(index)*target.value();
      target.setStandardDeviation(standardDeviation);
      target.setUaWeight(objectiveFunction.weight(index));
    }
  }
}

} // namespace casaWizard
