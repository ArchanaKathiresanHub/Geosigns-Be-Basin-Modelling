#include "well.h"

#include "logger.h"
#include "model/scenarioIO.h"
#include "objectiveFunctionManager.h"
#include "scenarioReader.h"
#include "scenarioWriter.h"


#include <QStringList>

#include <cmath>

namespace casaWizard
{

Well::Well(const int id, const QString& name, const double x, const double y, const bool isActive, const bool isExcluded, const QVector<CalibrationTarget> calibrationTargets) :
  id_{id},
  name_{name},
  x_{x},
  y_{y},
  isActive_{isActive},
  isExcluded_{isExcluded},
  isInvalid_{false},
  calibrationTargets_{calibrationTargets},
  hasDataInLayer_{}
{
}

int Well::version() const
{
  return 3;
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
  writer.writeValue(wellName + "isInvalid", isInvalid_);
  writer.writeValue(wellName + "targets", calibrationTargets_);
  writer.writeValue(wellName + "hasDataInLayer", hasDataInLayer_);
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
  if (version > 2)
  {
    hasDataInLayer_ = reader.readVector<bool>(wellName + "hasDataInLayer");
  }
  if (version > 1)
  {
    isInvalid_ = reader.readBool(wellName + "isInvalid");
  }
  else if (version > 0)
  {
    isInvalid_ = reader.readBool(wellName + "isOutOfBasin");
  }
  else
  {
    isInvalid_ = false;
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

void Well::setId(const int id)
{
  id_ = id;
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

bool Well::isInvalid() const
{
  return isInvalid_;
}

void Well::setIsInvalid(const bool isInvalid)
{
  isInvalid_ = isInvalid;
}

QVector<bool> Well::hasDataInLayer() const
{
  return hasDataInLayer_;
}

void Well::setHasDataInLayer(QVector<bool> hasDataInLayer)
{
  hasDataInLayer_ = hasDataInLayer;
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

void Well::addCalibrationTarget(const QString& name, const QString& propertyUserName,
                                const double z, const double value)
{
  const double standardDeviation = 1;
  const double uaWeight = 1;

  CalibrationTarget newTarget{name, propertyUserName, z, value, standardDeviation, uaWeight};
  calibrationTargets_.append(newTarget);
}

void Well::applyObjectiveFunction(const ObjectiveFunctionManager& objectiveFunction)
{
  for (CalibrationTarget& target : calibrationTargets_)
  {
    const int index = objectiveFunction.indexOfUserName(target.propertyUserName());
    if (index>-1)
    {
      double standardDeviation = objectiveFunction.absoluteError(index) +
                                 objectiveFunction.relativeError(index)*target.value();
      target.setStandardDeviation(standardDeviation);
      target.setUaWeight(objectiveFunction.weight(index));
    }
  }
}

void Well::shift(const double xShift)
{
  x_+=xShift;
}

void Well::renameUserPropertyName(const QString& oldName, const QString& newName)
{
  for (CalibrationTarget& target : calibrationTargets_)
  {
    if (target.propertyUserName() == oldName)
    {
      target.setPropertyUserName(newName);
    }
  }
}

void Well::removeCalibrationTargetsWithPropertyUserName(const QString& propertyUserName)
{
  for (int i = calibrationTargets_.size() - 1; i >= 0  ; i--)
  {
    if (calibrationTargets_[i].propertyUserName() == propertyUserName)
    {
      calibrationTargets_.remove(i);
    }
  }
}

bool Well::removeDataBelowDepth(const double depth)
{
  bool hasRemovedData = false;
  for (int i = calibrationTargets_.size() - 1; i >= 0; i--)
  {
    if (calibrationTargets_[i].z() > depth)
    {
      calibrationTargets_.remove(i);
      hasRemovedData = true;
    }
  }

  return hasRemovedData;
}

bool Well::removeDataAboveDepth(const double depth)
{
  bool hasRemovedData = false;
  for (int i = calibrationTargets_.size() - 1; i >= 0; i--)
  {
    if (calibrationTargets_[i].z() < depth)
    {
      calibrationTargets_.remove(i);
      hasRemovedData = true;
    }
  }

  return hasRemovedData;
}

} // namespace casaWizard
