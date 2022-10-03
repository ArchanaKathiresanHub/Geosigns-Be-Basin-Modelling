#include "optimizedLithofraction.h"

#include "model/scenarioIO.h"

#include <QStringList>

namespace casaWizard
{

namespace sac
{

OptimizedLithofraction::OptimizedLithofraction(const int wellId,
                                               const int lithofractionId,
                                               const double optimizedPercentageFirstComponent,
                                               const double optimizedFractionSecondComponent) :
  wellId_{wellId},
  lithofractionId_{lithofractionId},
  optimizedPercentageFirstComponent_{optimizedPercentageFirstComponent},
  optimizedFractionSecondComponent_{optimizedFractionSecondComponent}
{
}

int OptimizedLithofraction::version() const
{
  return 0;
}

OptimizedLithofraction  OptimizedLithofraction::read(const int /*version*/, const QStringList& p)
{
  if (p.size() != 4)
  {
    return OptimizedLithofraction{0, 0, 0.0, 0.0};
  }
  return OptimizedLithofraction
  {
    p[0].toInt(),
    p[1].toInt(),
    p[2].toDouble(),
    p[3].toDouble()
  };
}

QStringList OptimizedLithofraction::write() const
{
  QStringList out;
  out << QString::number(wellId_)
      << QString::number(lithofractionId_)
      << scenarioIO::doubleToQString(optimizedPercentageFirstComponent_)
      << scenarioIO::doubleToQString(optimizedFractionSecondComponent_);
  return out;
}

int OptimizedLithofraction::wellId() const
{
 return wellId_;
}

void OptimizedLithofraction::setLithofractionId( int lithofractionId)
{
  lithofractionId_ = lithofractionId;
}

int OptimizedLithofraction::lithofractionId() const
{
  return lithofractionId_;
}

double OptimizedLithofraction::optimizedPercentageFirstComponent() const
{
  return optimizedPercentageFirstComponent_;
}

double OptimizedLithofraction::optimizedFractionSecondComponent() const
{
  return optimizedFractionSecondComponent_;
}

double OptimizedLithofraction::optimizedPercentageSecondComponent() const
{
  return (100.0 - optimizedPercentageFirstComponent_) * optimizedFractionSecondComponent_;
}

double OptimizedLithofraction::optimizedPercentageThirdComponent() const
{
  return 100.0 - optimizedPercentageFirstComponent() - optimizedPercentageSecondComponent();
}

} // namespace sac

} // namespace casaWizard
