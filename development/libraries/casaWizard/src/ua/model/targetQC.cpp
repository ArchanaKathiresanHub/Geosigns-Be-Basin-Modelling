#include "targetQC.h"

#include "model/scenarioIO.h"
#include "model/targetParameterMapCreator.h"

namespace casaWizard
{

namespace ua
{

TargetQC::TargetQC(const double id,
                   const QString& property,
                   const QString& name,
                   const bool calibration,
                   const double value,
                   const double standardDeviation,
                   const double R2,
                   const double R2Adj,
                   const double Q2,
                   const QVector<double>& y,
                   const QVector<double>& yProxy) :
  id_{id},
  property_{property},
  name_{name},
  calibration_{calibration},
  value_{value},
  standardDeviation_{standardDeviation},
  R2_{R2},
  R2Adj_{R2Adj},
  Q2_{Q2},
  y_{y},
  yProxy_{yProxy}
{
}

int TargetQC::version() const
{
  return 0;
}

TargetQC TargetQC::read(const int /*version*/, const QStringList& p)
{
  return TargetQC
  {
    p[0].toInt(),
    p[1],
    p[2],
    p[3]=="1",
    p[4].toDouble(),
    p[5].toDouble(),
    p[6].toDouble(),
    p[7].toDouble(),
    p[8].toDouble(),
    scenarioIO::vectorFromRead(p[9]),
    scenarioIO::vectorFromRead(p[10])
  };
}

QStringList TargetQC::write() const
{
  QStringList out;
  out << QString::number(id_)
      << property_
      << name_
      << (calibration_?"1":"0")
      << scenarioIO::doubleToQString(value_)
      << scenarioIO::doubleToQString(standardDeviation_)
      << scenarioIO::doubleToQString(R2_)
      << scenarioIO::doubleToQString(R2Adj_)
      << scenarioIO::doubleToQString(Q2_)
      << scenarioIO::vectorToWrite(y_)
      << scenarioIO::vectorToWrite(yProxy_);
  return out;
}

int TargetQC::id() const
{
  return id_;
}

QString TargetQC::property() const
{
  return property_;
}

QString TargetQC::name() const
{
  return name_;
}

bool TargetQC::calibration() const
{
  return calibration_;
}

double TargetQC::value() const
{
  return value_;
}

double TargetQC::standardDeviation() const
{
  return standardDeviation_;
}

double TargetQC::R2() const
{
  return R2_;
}

double TargetQC::R2Adj() const
{
  return R2Adj_;
}

QVector<double> TargetQC::y() const
{
  return y_;
}

QVector<double> TargetQC::yProxy() const
{
  return yProxy_;
}

QString TargetQC::unitSI() const
{
  return targetParameterMapCreator::lookupSIUnit(property_);
}

double TargetQC::Q2() const
{
  return Q2_;
}

} // namespace ua

} // namespace casaWizard
