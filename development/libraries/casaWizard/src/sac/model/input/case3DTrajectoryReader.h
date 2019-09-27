// Reader for data extracted using Track1D from a 3D run
#pragma once

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace sac
{

class WellTrajectoryManager;

class Case3DTrajectoryReader
{
public:
  Case3DTrajectoryReader(const QString filename);
  void read();

  QStringList properties() const;
  QVector<double> x() const;
  QVector<double> y() const;
  QVector<double> depth() const;
  QVector<double> readProperty(const QString property) const;

private:
  void extractProperties(const QString header);

  QString filename_;
  QStringList properties_;

  QVector<double> x_;
  QVector<double> y_;
  QVector<double> depth_;
  QVector<QVector<double>> propertyValues_;
};

}  // namespace sac

}  // namespace casaWizard
