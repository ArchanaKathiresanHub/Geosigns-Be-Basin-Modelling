#include "targetQC.h"

#include "model/scenarioIO.h"
#include "model/targetParameterMapCreator.h"

namespace casaWizard
{

namespace ua
{

TargetQC::TargetQC(const int id,
                   const QString& property,
                   const QString& name,
                   const QString& identifier,
                   const bool calibration,
                   const double value,
                   const double standardDeviation,
                   const double R2,
                   const double R2Adj,
                   const double Q2,
                   const QVector<double>& y,
                   const QVector<double>& yProxy,
                   const double yOptimalSim) :
   id_{id},
   property_{property},
   name_{name},
   identifier_{identifier},
   calibration_{calibration},
   value_{value},
   standardDeviation_{standardDeviation},
   R2_{R2},
   R2Adj_{R2Adj},
   Q2_{Q2},
   y_{y},
   yProxy_{yProxy},
   m_yBase{double(NAN)},
   m_yBaseProxy{double(NAN)},
   m_yOptimalSim{yOptimalSim}
{
}

int TargetQC::version() const
{
   return 2;
}

TargetQC TargetQC::read(const int version, const QStringList& p)
{
   if (version == 0 && p.size() == 11)
   {
      TargetQC target
      {
         p[0].toInt(),
               p[1],
               p[2],
               "",
               p[3]=="1",
               p[4].toDouble(),
               p[5].toDouble(),
               p[6].toDouble(),
               p[7].toDouble(),
               p[8].toDouble(),
               scenarioIO::vectorFromRead(p[9]),
               scenarioIO::vectorFromRead(p[10])
      };
      return target;
   }
   else
   {
      TargetQC target
      {
         p[0].toInt(),
               p[1],
               p[2],
               p[3],
               p[4]=="1",
               p[5].toDouble(),
               p[6].toDouble(),
               p[7].toDouble(),
               p[8].toDouble(),
               p[9].toDouble(),
               scenarioIO::vectorFromRead(p[10]),
               scenarioIO::vectorFromRead(p[11]),
      };
      target.setValOptimalSim(p[12].toDouble());

      if (version >= 2)
      {
         target.setValBaseSim(p[13].toDouble());
         target.setValBaseProxy(p[14].toDouble());
      }
      return target;
   }
}

QStringList TargetQC::write() const
{
   QStringList out;
   out << QString::number(id_)
       << property_
       << name_
       << identifier_
       << (calibration_?"1":"0")
       << scenarioIO::doubleToQString(value_)
       << scenarioIO::doubleToQString(standardDeviation_)
       << scenarioIO::doubleToQString(R2_)
       << scenarioIO::doubleToQString(R2Adj_)
       << scenarioIO::doubleToQString(Q2_)
       << scenarioIO::vectorToWrite(y_)
       << scenarioIO::vectorToWrite(yProxy_)
       << scenarioIO::doubleToQString(m_yBase)
       << scenarioIO::doubleToQString(m_yBaseProxy)
       << scenarioIO::doubleToQString(m_yOptimalSim);
   return out;
}

void TargetQC::setValOptimalSim(double vOptimalSim)
{
   m_yOptimalSim = vOptimalSim;
}

void TargetQC::setValBaseSim(double vBaseSim)
{
   m_yBase = vBaseSim;
}

void TargetQC::setValBaseProxy(double vBaseProxy)
{
   m_yBaseProxy = vBaseProxy;
}

double TargetQC::yOptimalSim() const
{
   return m_yOptimalSim;
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

QString TargetQC::identifier() const
{
   return identifier_;
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

double TargetQC::yBase() const
{
   return m_yBase;
}

double TargetQC::yBaseProxy() const
{
   return m_yBaseProxy;
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
