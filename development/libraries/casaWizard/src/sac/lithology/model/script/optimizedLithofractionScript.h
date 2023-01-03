// Script to run casa to extract the optimized lithofraction for each case
#pragma once

#include "model/script/OptimalValueExtractionScript.h"

#include <QStringList>
namespace casaWizard
{

namespace sac
{

namespace lithology
{

class OptimizedLithofractionScript : public OptimalValueExtractionScript
{
public:
   explicit OptimizedLithofractionScript(const QString& baseDirectory, QString scriptFileName = "obtainLithofractions.casa");
private:
   void writeScript(QFile* file) override;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
