// Struct for storing the optimized lithofraction values per well
#pragma once

#include <QStringList>
#include "ConstantsNumerical.h"
namespace casaWizard
{

namespace sac
{

namespace thermal
{

class OptimizedTCHP
{
public:
   OptimizedTCHP() = default;
   OptimizedTCHP(const int wellId,
                 const double optimizedHeatProduction = Utilities::Numerical::CauldronNoDataValueInt);

   int version() const;
   static OptimizedTCHP read(const int version, const QStringList& p);
   QStringList write() const;
   int wellId() const;
   double optimizedHeatProduction() const;

private:
   int m_wellId;
   double m_optimizedHeatProduction;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
