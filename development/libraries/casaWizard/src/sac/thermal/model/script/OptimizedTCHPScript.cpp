#include "OptimizedTCHPScript.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

OptimizedTCHPScript::OptimizedTCHPScript(const QString& baseDirectory, QString scriptFileName) :
   OptimalValueExtractionScript(baseDirectory, scriptFileName)
{}

void OptimizedTCHPScript::writeScript(QFile* file)
{
   QTextStream out(file);
   out << "loadstate \"casa_state.bin\" \"bin\"\n";
   out << "exportDataTxt \"DoEParameters\" \"optimalTopCrustHeatProduction.txt\" \"BestMatchedCase\"\n";
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
