// Script to run casa to extract the optimized lithofraction for each case
#pragma once

#include "model/script/OptimalValueExtractionScript.h"

#include <QStringList>
namespace casaWizard
{

namespace sac
{

namespace thermal
{

class OptimizedTCHPScript : public OptimalValueExtractionScript
{
public:
   explicit OptimizedTCHPScript(const QString& baseDirectory, QString scriptFileName = "obtainTCHP.casa");
private:
   void writeScript(QFile* file) override;
};


} // namespace thermal

} // namespace sac

} // namespace casaWizard
