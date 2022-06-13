// Struct storing the Quality Check data for a single target
#pragma once

#include <QStringList>
#include <QVector>

#include <math.h>

namespace casaWizard
{

namespace ua
{

struct TargetQC
{
public:
  TargetQC() = default;
  explicit TargetQC(const int id,
                    const QString& property,
                    const QString& name,
                    const QString& identifier,
                    const bool calibration,
                    const double value,
                    const double standardDeviation,
                    const double R2,
                    const double R2Adj,
                    const double Q,
                    const QVector<double>& y,
                    const QVector<double>& yProxy,
                    const double yOptimalSim = double(NAN));

  int version() const;
  static TargetQC read(const int version, const QStringList& p);

  QStringList write() const;

  int id() const;
  QString property() const;
  QString name() const;
  QString identifier() const;
  bool calibration() const;
  double value() const;
  double standardDeviation() const;
  double R2() const;
  double R2Adj() const;
  QVector<double> y() const;
  QVector<double> yProxy() const;
  double yOptimalSim() const;
  QString unitSI() const;
  double Q2() const;

  void setValOptimalSim(double vOptimalSim);

private:
  int id_;
  QString property_;
  QString name_;
  QString identifier_;
  bool calibration_;
  double value_;
  double standardDeviation_;
  double R2_;
  double R2Adj_;
  double Q2_;
  QVector<double> y_;
  QVector<double> yProxy_;
  double m_yOptimalSim;
};

} // namespace ua

} // namespace casaWizard
