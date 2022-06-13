// Abstract base class for prediction target
#pragma once

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace ua
{

class PredictionTarget
{
public:
  PredictionTarget() = default;
  PredictionTarget(const QVector<QString>& properties, const double x, const double y, const double age, const QString& locationName, const double z = 0.0);
  static PredictionTarget* createFromList(const int version, const QStringList& list);
  virtual ~PredictionTarget() = default;

  virtual int version() const = 0;
  virtual QStringList write() const = 0;
  virtual QString name(const QString& property) const = 0;
  virtual QString identifier(const QString& property) const = 0;
  virtual QString nameWithoutAge() const = 0;
  virtual QString variable() const = 0;
  virtual QString typeName() const = 0;
  virtual QString surfaceName() const;
  virtual QString casaCommand(const QString& property) const = 0;

  static QString casaCommandFromStrVec(std::vector<std::string> stringVec);

  virtual PredictionTarget* createCopy() const = 0;

  QVector<QString> properties() const;
  void setProperties(const QVector<QString>& properties);
  void setPropertyActive(const bool active, const QString& property);

  double x() const;
  virtual void setX(double x);

  double y() const;
  virtual void setY(double y);

  double z() const;
  virtual void setZ(double z);

  double age() const;
  void setAge(double age);

  QString locationName() const;
  void setLocationName(const QString& locationName);

private:
  QVector<QString> m_properties;
  double m_x;
  double m_y;
  double m_z;
  double m_age;
  QString m_locationName;
};

} // namespace ua

} // namespace casaWizard
