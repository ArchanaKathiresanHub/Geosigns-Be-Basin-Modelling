// Struct storing the Quality Check data for a single target
#pragma once

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace ua
{

struct TargetQC
{
public:
  TargetQC() = default;
  explicit TargetQC(const double id, const QString& property, const QString& name, const bool calibration, const double value, const double standardDeviation,
                    const double R2, const double R2Adj, const double Q, const QVector<double>& y, const QVector<double>& yProxy);
  int version() const;
  static TargetQC read(const int version, const QStringList& p);

  QStringList write() const;

  int id() const;
  QString property() const;
  QString name() const;
  bool calibration() const;
  double value() const;
  double standardDeviation() const;
  double R2() const;
  double R2Adj() const;
  QVector<double> y() const;
  QVector<double> yProxy() const;
  QString unitSI() const;
  double Q2() const;

private:
  int id_;
  QString property_;
  QString name_;
  bool calibration_;
  double value_;
  double standardDeviation_;
  double R2_;
  double R2Adj_;
  double Q2_;
  QVector<double> y_;
  QVector<double> yProxy_;
};

} // namespace ua

} // namespace casaWizard
