#include "OptimizedTCHP.h"

#include "model/scenarioIO.h"

#include <QStringList>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

OptimizedTCHP::OptimizedTCHP(const int wellId,
                             const double optimizedValue) :
   m_wellId{wellId},
   m_optimizedHeatProduction{optimizedValue}
{
}

int OptimizedTCHP::version() const
{
   return 0;
}

OptimizedTCHP OptimizedTCHP::read(const int /*version*/, const QStringList& p)
{
   if (p.size() != 2)
   {
      return OptimizedTCHP{0, 0.0};
   }
   return OptimizedTCHP{p[0].toInt(), p[1].toDouble()};
}

QStringList OptimizedTCHP::write() const
{
   QStringList out;
   out << QString::number(m_wellId)
       << scenarioIO::doubleToQString(m_optimizedHeatProduction);
   return out;
}

int OptimizedTCHP::wellId() const
{
   return m_wellId;
}

double OptimizedTCHP::optimizedHeatProduction() const
{
   return m_optimizedHeatProduction;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
