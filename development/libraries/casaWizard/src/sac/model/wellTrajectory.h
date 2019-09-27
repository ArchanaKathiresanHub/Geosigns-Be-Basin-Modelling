// Data struct for well trajectories, written to file
#pragma once

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace sac
{

class WellTrajectory
{
public:
  WellTrajectory() = default;
  explicit WellTrajectory(const int trajectoryIndex, const int wellIndex, const QString& property, const QVector<double>& depth, const QVector<double>& value);
  int version() const;
  static WellTrajectory read(const int version, const QStringList& p);

  QString name() const;
  QStringList write() const;

  int trajectoryIndex() const;
  int wellIndex() const;
  QString property() const;

  QVector<double> depth() const;
  void setDepth(const QVector<double>& depth);

  QVector<double> value() const;
  void setValue(const QVector<double>& value);

private:
  int trajectoryIndex_;
  int wellIndex_;
  QString property_;
  QVector<double> depth_;
  QVector<double> value_;
};

} // namespace sac

} // namespace casaWizard
